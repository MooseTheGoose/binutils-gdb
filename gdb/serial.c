/* Generic serial interface routines

   Copyright (C) 1992-2022 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include <ctype.h>
#include "serial.h"
#include "gdbcmd.h"
#include "cli/cli-utils.h"

/* Is serial being debugged?  */

static unsigned int global_serial_debug_p;

/* Serial I/O handlers.  */

static std::vector<const struct serial_ops *> serial_ops_list;

/* Pointer to list of scb's.  */

static struct serial *scb_base;

/* Non-NULL gives filename which contains a recording of the remote session,
   suitable for playback by gdbserver.  */

static std::string serial_logfile;
static struct ui_file *serial_logfp = NULL;

static const struct serial_ops *serial_interface_lookup (const char *);
static void serial_logchar (struct ui_file *stream,
			    int ch_type, int ch, int timeout);
static const char logbase_hex[] = "hex";
static const char logbase_octal[] = "octal";
static const char logbase_ascii[] = "ascii";
static const char *const logbase_enums[] =
{logbase_hex, logbase_octal, logbase_ascii, NULL};
static const char *serial_logbase = logbase_ascii;


static int serial_current_type = 0;

/* Log char CH of type CHTYPE, with TIMEOUT.  */

/* Define bogus char to represent a BREAK.  Should be careful to choose a value
   that can't be confused with a normal char, or an error code.  */
#define SERIAL_BREAK 1235

static void
serial_logchar (struct ui_file *stream, int ch_type, int ch, int timeout)
{
  if (ch_type != serial_current_type)
    {
      fprintf_unfiltered (stream, "\n%c ", ch_type);
      serial_current_type = ch_type;
    }

  if (serial_logbase != logbase_ascii)
    fputc_unfiltered (' ', stream);

  switch (ch)
    {
    case SERIAL_TIMEOUT:
      fprintf_unfiltered (stream, "<Timeout: %d seconds>", timeout);
      return;
    case SERIAL_ERROR:
      fprintf_unfiltered (stream, "<Error: %s>", safe_strerror (errno));
      return;
    case SERIAL_EOF:
      fputs_unfiltered ("<Eof>", stream);
      return;
    case SERIAL_BREAK:
      fputs_unfiltered ("<Break>", stream);
      return;
    default:
      if (serial_logbase == logbase_hex)
	fprintf_unfiltered (stream, "%02x", ch & 0xff);
      else if (serial_logbase == logbase_octal)
	fprintf_unfiltered (stream, "%03o", ch & 0xff);
      else
	switch (ch)
	  {
	  case '\\':
	    fputs_unfiltered ("\\\\", stream);
	    break;
	  case '\b':
	    fputs_unfiltered ("\\b", stream);
	    break;
	  case '\f':
	    fputs_unfiltered ("\\f", stream);
	    break;
	  case '\n':
	    fputs_unfiltered ("\\n", stream);
	    break;
	  case '\r':
	    fputs_unfiltered ("\\r", stream);
	    break;
	  case '\t':
	    fputs_unfiltered ("\\t", stream);
	    break;
	  case '\v':
	    fputs_unfiltered ("\\v", stream);
	    break;
	  default:
	    fprintf_unfiltered (stream,
				isprint (ch) ? "%c" : "\\x%02x", ch & 0xFF);
	    break;
	  }
    }
}

void
serial_log_command (struct target_ops *self, const char *cmd)
{
  if (!serial_logfp)
    return;

  serial_current_type = 'c';

  fputs_unfiltered ("\nc ", serial_logfp);
  fputs_unfiltered (cmd, serial_logfp);

  /* Make sure that the log file is as up-to-date as possible,
     in case we are getting ready to dump core or something.  */
  gdb_flush (serial_logfp);
}


static const struct serial_ops *
serial_interface_lookup (const char *name)
{
  for (const serial_ops *ops : serial_ops_list)
    if (strcmp (name, ops->name) == 0)
      return ops;

  return NULL;
}

void
serial_add_interface (const struct serial_ops *optable)
{
  serial_ops_list.push_back (optable);
}

/* Return the open serial device for FD, if found, or NULL if FD is
   not already opened.  */

struct serial *
serial_for_fd (int fd)
{
  struct serial *scb;

  for (scb = scb_base; scb; scb = scb->next)
    if (scb->fd == fd)
      return scb;

  return NULL;
}

/* Create a new serial for OPS.  */

static struct serial *
new_serial (const struct serial_ops *ops)
{
  struct serial *scb;

  scb = XCNEW (struct serial);

  scb->ops = ops;

  scb->bufp = scb->buf;
  scb->error_fd = -1;
  scb->refcnt = 1;

  return scb;
}

static struct serial *serial_open_ops_1 (const struct serial_ops *ops,
					 const char *open_name);

/* Open up a device or a network socket, depending upon the syntax of NAME.  */

struct serial *
serial_open (const char *name)
{
  const struct serial_ops *ops;
  const char *open_name = name;

  if (startswith (name, "|"))
    {
      ops = serial_interface_lookup ("pipe");
      /* Discard ``|'' and any space before the command itself.  */
      ++open_name;
      open_name = skip_spaces (open_name);
    }
  /* Check for a colon, suggesting an IP address/port pair.
     Do this *after* checking for all the interesting prefixes.  We
     don't want to constrain the syntax of what can follow them.  */
  else if (strchr (name, ':'))
    ops = serial_interface_lookup ("tcp");
  else
    {
#ifndef USE_WIN32API
      /* Check to see if name is a socket.  If it is, then treat it
	 as such.  Otherwise assume that it's a character device.  */
      struct stat sb;
      if (stat (name, &sb) == 0 && (sb.st_mode & S_IFMT) == S_IFSOCK)
	ops = serial_interface_lookup ("local");
      else
#endif
	ops = serial_interface_lookup ("hardwire");
    }

  if (!ops)
    return NULL;

  return serial_open_ops_1 (ops, open_name);
}

/* Open up a serial for OPS, passing OPEN_NAME to the open method.  */

static struct serial *
serial_open_ops_1 (const struct serial_ops *ops, const char *open_name)
{
  struct serial *scb;

  scb = new_serial (ops);

  /* `...->open (...)' would get expanded by the open(2) syscall macro.  */
  if ((*scb->ops->open) (scb, open_name))
    {
      xfree (scb);
      return NULL;
    }

  scb->name = open_name != NULL ? xstrdup (open_name) : NULL;
  scb->next = scb_base;
  scb_base = scb;

  if (!serial_logfile.empty ())
    {
      stdio_file_up file (new stdio_file ());

      if (!file->open (serial_logfile.c_str (), "w"))
	perror_with_name (serial_logfile.c_str ());

      serial_logfp = file.release ();
    }

  return scb;
}

/* See serial.h.  */

struct serial *
serial_open_ops (const struct serial_ops *ops)
{
  return serial_open_ops_1 (ops, NULL);
}

/* Open a new serial stream using a file handle, using serial
   interface ops OPS.  */

static struct serial *
serial_fdopen_ops (const int fd, const struct serial_ops *ops)
{
  struct serial *scb;

  if (!ops)
    {
      ops = serial_interface_lookup ("terminal");
      if (!ops)
	ops = serial_interface_lookup ("hardwire");
    }

  if (!ops)
    return NULL;

  scb = new_serial (ops);

  scb->name = NULL;
  scb->next = scb_base;
  scb_base = scb;

  if ((ops->fdopen) != NULL)
    (*ops->fdopen) (scb, fd);
  else
    scb->fd = fd;

  return scb;
}

struct serial *
serial_fdopen (const int fd)
{
  return serial_fdopen_ops (fd, NULL);
}

static void
do_serial_close (struct serial *scb, int really_close)
{
  struct serial *tmp_scb;

  if (serial_logfp)
    {
      fputs_unfiltered ("\nEnd of log\n", serial_logfp);
      serial_current_type = 0;

      /* XXX - What if serial_logfp == gdb_stdout or gdb_stderr?  */
      delete serial_logfp;
      serial_logfp = NULL;
    }

  /* ensure that the FD has been taken out of async mode.  */
  if (scb->async_handler != NULL)
    serial_async (scb, NULL, NULL);

  if (really_close)
    scb->ops->close (scb);

  xfree (scb->name);

  /* For serial_is_open.  */
  scb->bufp = NULL;

  if (scb_base == scb)
    scb_base = scb_base->next;
  else
    for (tmp_scb = scb_base; tmp_scb; tmp_scb = tmp_scb->next)
      {
	if (tmp_scb->next != scb)
	  continue;

	tmp_scb->next = tmp_scb->next->next;
	break;
      }

  serial_unref (scb);
}

void
serial_close (struct serial *scb)
{
  do_serial_close (scb, 1);
}

void
serial_un_fdopen (struct serial *scb)
{
  do_serial_close (scb, 0);
}

int
serial_is_open (struct serial *scb)
{
  return scb->bufp != NULL;
}

void
serial_ref (struct serial *scb)
{
  scb->refcnt++;
}

void
serial_unref (struct serial *scb)
{
  --scb->refcnt;
  if (scb->refcnt == 0)
    xfree (scb);
}

int
serial_readchar (struct serial *scb, int timeout)
{
  int ch;

  /* FIXME: cagney/1999-10-11: Don't enable this check until the ASYNC
     code is finished.  */
  if (0 && serial_is_async_p (scb) && timeout < 0)
    internal_error (__FILE__, __LINE__,
		    _("serial_readchar: blocking read in async mode"));

  ch = scb->ops->readchar (scb, timeout);
  if (serial_logfp != NULL)
    {
      serial_logchar (serial_logfp, 'r', ch, timeout);

      /* Make sure that the log file is as up-to-date as possible,
	 in case we are getting ready to dump core or something.  */
      gdb_flush (serial_logfp);
    }
  if (serial_debug_p (scb))
    {
      fprintf_unfiltered (gdb_stdlog, "[");
      serial_logchar (gdb_stdlog, 'r', ch, timeout);
      fprintf_unfiltered (gdb_stdlog, "]");
      gdb_flush (gdb_stdlog);
    }

  return (ch);
}

int
serial_write (struct serial *scb, const void *buf, size_t count)
{
  if (serial_logfp != NULL)
    {
      const char *str = (const char *) buf;
      size_t c;

      for (c = 0; c < count; c++)
	serial_logchar (serial_logfp, 'w', str[c] & 0xff, 0);

      /* Make sure that the log file is as up-to-date as possible,
	 in case we are getting ready to dump core or something.  */
      gdb_flush (serial_logfp);
    }
  if (serial_debug_p (scb))
    {
      const char *str = (const char *) buf;
      size_t c;

      for (c = 0; c < count; c++)
	{
	  fprintf_unfiltered (gdb_stdlog, "[");
	  serial_logchar (gdb_stdlog, 'w', str[c] & 0xff, 0);
	  fprintf_unfiltered (gdb_stdlog, "]");
	}
      gdb_flush (gdb_stdlog);
    }

  return (scb->ops->write (scb, buf, count));
}

void
serial_printf (struct serial *desc, const char *format, ...)
{
  va_list args;
  va_start (args, format);

  std::string buf = string_vprintf (format, args);
  serial_write (desc, buf.c_str (), buf.length ());

  va_end (args);
}

int
serial_drain_output (struct serial *scb)
{
  return scb->ops->drain_output (scb);
}

int
serial_flush_output (struct serial *scb)
{
  return scb->ops->flush_output (scb);
}

int
serial_flush_input (struct serial *scb)
{
  return scb->ops->flush_input (scb);
}

int
serial_send_break (struct serial *scb)
{
  if (serial_logfp != NULL)
    serial_logchar (serial_logfp, 'w', SERIAL_BREAK, 0);

  return (scb->ops->send_break (scb));
}

void
serial_raw (struct serial *scb)
{
  scb->ops->go_raw (scb);
}

serial_ttystate
serial_get_tty_state (struct serial *scb)
{
  return scb->ops->get_tty_state (scb);
}

serial_ttystate
serial_copy_tty_state (struct serial *scb, serial_ttystate ttystate)
{
  return scb->ops->copy_tty_state (scb, ttystate);
}

int
serial_set_tty_state (struct serial *scb, serial_ttystate ttystate)
{
  return scb->ops->set_tty_state (scb, ttystate);
}

void
serial_print_tty_state (struct serial *scb,
			serial_ttystate ttystate,
			struct ui_file *stream)
{
  scb->ops->print_tty_state (scb, ttystate, stream);
}

int
serial_setbaudrate (struct serial *scb, int rate)
{
  return scb->ops->setbaudrate (scb, rate);
}

int
serial_setstopbits (struct serial *scb, int num)
{
  return scb->ops->setstopbits (scb, num);
}

/* See serial.h.  */

int
serial_setparity (struct serial *scb, int parity)
{
  return scb->ops->setparity (scb, parity);
}

int
serial_can_async_p (struct serial *scb)
{
  return (scb->ops->async != NULL);
}

int
serial_is_async_p (struct serial *scb)
{
  return (scb->ops->async != NULL) && (scb->async_handler != NULL);
}

void
serial_async (struct serial *scb,
	      serial_event_ftype *handler,
	      void *context)
{
  int changed = ((scb->async_handler == NULL) != (handler == NULL));

  scb->async_handler = handler;
  scb->async_context = context;
  /* Only change mode if there is a need.  */
  if (changed)
    scb->ops->async (scb, handler != NULL);
}

void
serial_debug (struct serial *scb, int debug_p)
{
  scb->debug_p = debug_p;
}

int
serial_debug_p (struct serial *scb)
{
  return scb->debug_p || global_serial_debug_p;
}

#ifdef USE_WIN32API
void
serial_wait_handle (struct serial *scb, HANDLE *read, HANDLE *except)
{
  if (scb->ops->wait_handle)
    scb->ops->wait_handle (scb, read, except);
  else
    {
      *read = (HANDLE) _get_osfhandle (scb->fd);
      *except = NULL;
    }
}

void
serial_done_wait_handle (struct serial *scb)
{
  if (scb->ops->done_wait_handle)
    scb->ops->done_wait_handle (scb);
}
#endif

int
serial_pipe (struct serial *scbs[2])
{
  const struct serial_ops *ops;
  int fildes[2];

  ops = serial_interface_lookup ("pipe");
  if (!ops)
    {
      errno = ENOSYS;
      return -1;
    }

  if (gdb_pipe (fildes) == -1)
    return -1;

  scbs[0] = serial_fdopen_ops (fildes[0], ops);
  scbs[1] = serial_fdopen_ops (fildes[1], ops);
  return 0;
}

/* Serial set/show framework.  */

static struct cmd_list_element *serial_set_cmdlist;
static struct cmd_list_element *serial_show_cmdlist;

/* See serial.h.  */

int baud_rate = -1;

static void
serial_baud_show_cmd (struct ui_file *file, int from_tty,
		      struct cmd_list_element *c, const char *value)
{
  fprintf_filtered (file, _("Baud rate for remote serial I/O is %s.\n"),
		    value);
}

/* See serial.h.  */

int serial_parity = GDBPARITY_NONE;

static const char parity_none[] = "none";
static const char parity_odd[] = "odd";
static const char parity_even[] = "even";
static const char *const parity_enums[] =
  {parity_none, parity_odd, parity_even,  NULL};
static const char *parity = parity_none;

/* Set serial_parity value.  */

static void
set_parity (const char *ignore_args, int from_tty, struct cmd_list_element *c)
{
  if (parity == parity_odd)
    serial_parity = GDBPARITY_ODD;
  else if (parity == parity_even)
    serial_parity = GDBPARITY_EVEN;
  else
    serial_parity = GDBPARITY_NONE;
}

void _initialize_serial ();
void
_initialize_serial ()
{
#if 0
  add_com ("connect", class_obscure, connect_command, _("\
Connect the terminal directly up to the command monitor.\n\
Use <CR>~. or <CR>~^D to break out."));
#endif /* 0 */

  add_setshow_prefix_cmd ("serial", class_maintenance,
			  _("Set default serial/parallel port configuration."),
			  _("Show default serial/parallel port configuration."),
			  &serial_set_cmdlist, &serial_show_cmdlist,
			  &setlist, &showlist);

  /* If target is open when baud changes, it doesn't take effect until
     the next open (I think, not sure).  */
  add_setshow_zinteger_cmd ("baud", no_class, &baud_rate, _("\
Set baud rate for remote serial I/O."), _("\
Show baud rate for remote serial I/O."), _("\
This value is used to set the speed of the serial port when debugging\n\
using remote targets."),
			    NULL,
			    serial_baud_show_cmd,
			    &serial_set_cmdlist, &serial_show_cmdlist);

  add_setshow_enum_cmd ("parity", no_class, parity_enums,
			&parity, _("\
Set parity for remote serial I/O."), _("\
Show parity for remote serial I/O."), NULL,
			set_parity,
			NULL, /* FIXME: i18n: */
			&serial_set_cmdlist, &serial_show_cmdlist);

  add_setshow_filename_cmd ("remotelogfile", no_class, &serial_logfile, _("\
Set filename for remote session recording."), _("\
Show filename for remote session recording."), _("\
This file is used to record the remote session for future playback\n\
by gdbserver."),
			    NULL,
			    NULL, /* FIXME: i18n: */
			    &setlist, &showlist);

  add_setshow_enum_cmd ("remotelogbase", no_class, logbase_enums,
			&serial_logbase, _("\
Set numerical base for remote session logging."), _("\
Show numerical base for remote session logging."), NULL,
			NULL,
			NULL, /* FIXME: i18n: */
			&setlist, &showlist);

  add_setshow_zuinteger_cmd ("serial", class_maintenance,
			     &global_serial_debug_p, _("\
Set serial debugging."), _("\
Show serial debugging."), _("\
When non-zero, serial port debugging is enabled."),
			     NULL,
			     NULL, /* FIXME: i18n: */
			     &setdebuglist, &showdebuglist);
}
