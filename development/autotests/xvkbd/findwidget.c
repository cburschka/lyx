/*
 * xvkbd - Virtual Keyboard for X Window System
 *
 * Copyright (C) 2000 by Tom Sato <VEF00200@nifty.ne.jp>
 * http://homepage3.nifty.com/tsato/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * This code is derived from editres in X11R6 distribution.
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xmu/EditresP.h>

#include "resources.h"

#define EDITRES_PROTOCOL_VERSION 5  /* Editres Protocol version 1.1 */
#define EditresLocalSendWidgetTree 0  /* op code defined in Editres Protocol */

#define CLIENT_TIME_OUT 2000  /* 2sec */

static void RequestSendWidgetTree(Widget w);  /* forward */
static void LoseSelection(Widget w, Atom *sel);  /* forward */

static Atom atom_comm = None;
static Atom atom_command = None;
static Atom atom_resource_editor = None;
static Atom atom_editres_protocol = None;
static Atom atom_client_value = None;

static ResIdent client_ident = 1;

static Window client_window = None;
static ProtocolStream client_stream;
static int effective_protocol_version;

static Boolean waiting_response;

/*
 * List of widgets will be stored in widget_list[], and number
 * of widgets will be stored in num_widget_list.
 */
static int num_widget_list = 0;

static struct WidgetList {
  Window window;
  unsigned long id;
  unsigned long parent;
  char *name;
  char *class;
  char *path;
} *widget_list = NULL;


/*
 * This function will be called when the client requests the
 * value of the selection.
 */
static Boolean ConvertCommand(Widget w,
                      Atom *selection, Atom *target, Atom *type_ret,
                      XtPointer *value_ret, unsigned long *length_ret,
                      int *format_ret)
{
  if ((*selection != atom_comm) || (*target != atom_command))
    return(FALSE);

  *type_ret = atom_editres_protocol;
  *value_ret = (XtPointer)client_stream.real_top;
  *length_ret = client_stream.size + HEADER_SIZE;
  *format_ret = EDITRES_FORMAT;

  return(TRUE);
}

/*
 * Read response to the Editres LocalSendWidgetTree request
 * and make list of the widgets.
 * If appres.list is TRUE, list of all widgets will be printed
 * to the stdout.
 */
static void GetClientValue(Widget w, XtPointer data,
                           Atom *selection, Atom *type, XtPointer value,
                           unsigned long *length, int *format)
{
  ProtocolStream stream;
  unsigned char ident, version, error_code;
  unsigned short num_entries;
  WidgetInfo widgets;
  char *name, *class, *err_msg;
  unsigned long window;
  int i, j;

  if (*length == 0) return;

  stream.current = stream.top = (unsigned char *)value;
  stream.size = HEADER_SIZE;

  if (*length < HEADER_SIZE) {
    fprintf(stderr, "%s: incorrectly formatted message from client\n",
            PROGRAM_NAME);
    return;
  }

  _XEditResGet8(&stream, &ident);
  if (client_ident != ident) {
    fprintf(stderr, "%s: wincorrect ident from client\n", PROGRAM_NAME);
    if (!XtOwnSelection(w, *selection, CurrentTime,
                        ConvertCommand, LoseSelection, NULL))
    fprintf(stderr, "%s: XtOwnSelection() failed\n", PROGRAM_NAME);
    return;
  }

  _XEditResGet8(&stream, &error_code);
  _XEditResGet32(&stream, &stream.size);
  stream.top = stream.current;  /* reset stream to top of value */

  switch ((int) error_code) {
  case PartialSuccess:
    if (widget_list != NULL) {
      for (i = 0; i < num_widget_list; i++) {
        XtFree(widget_list[i].name);
        XtFree(widget_list[i].class);
        XtFree(widget_list[i].path);
      }
      XtFree((void *)widget_list);
      widget_list = NULL;
    }

    if (_XEditResGet16(&stream, &num_entries)) {
      widget_list = (struct WidgetList *)XtMalloc(sizeof(struct WidgetList)
                                                  * num_entries);
      num_widget_list = num_entries;
      for (i = 0; i < num_entries; i++) {
        if (!(_XEditResGetWidgetInfo(&stream, &widgets) &&
              _XEditResGetString8(&stream, &name) &&
              _XEditResGetString8(&stream, &class) &&
              _XEditResGet32(&stream, &window))) {
          num_widget_list = i;
          break;
        }
        widget_list[i].window = window;
        widget_list[i].id = widgets.ids[widgets.num_widgets - 1];
        if (widgets.num_widgets < 2) widget_list[i].parent = 0;
        else widget_list[i].parent = widgets.ids[widgets.num_widgets - 2];
        widget_list[i].name = XtNewString(name);
        widget_list[i].class = XtNewString(class);

        widget_list[i].path = NULL;
        if (widget_list[i].parent != 0) {
          for (j = 0; j < i; j++) {
            if (widget_list[j].id == widget_list[i].parent) {
              widget_list[i].path = XtMalloc(strlen(widget_list[j].path)
                                             + strlen(widget_list[i].name) + 2);
              sprintf(widget_list[i].path, "%s.%s",
                      widget_list[j].path, widget_list[i].name);
              break;
            }
          }
        }
        if (widget_list[i].path == NULL) {
          widget_list[i].path = XtMalloc(strlen(widget_list[i].name) + 2);
          sprintf(widget_list[i].path, "%s%s",
                  (widget_list[i].parent != 0) ? "*" : "",
                  widget_list[i].name);
        }
        if (appres.list_widgets) {
          fprintf(stdout, "0x%08lx 0x%08lx (%s) %s\n",
                  (long)widget_list[i].window, (long)widget_list[i].id,
                  widget_list[i].class, widget_list[i].path);
        }
      }
    }
    break;
  case ProtocolMismatch:
    if (!_XEditResGet8(&stream, &version)) {
      fprintf(stderr, "%s: unable to unpack protocol request\n", PROGRAM_NAME);
    } else if (version == effective_protocol_version) {
      fprintf(stderr, "%s: internal error - ProtocolMismatch\n", PROGRAM_NAME);
    } else {
      fprintf(stderr, "%s: protocol version mismatch (requested=%d, supported=%d)\n",
              PROGRAM_NAME, (int)effective_protocol_version, (int)version);
      effective_protocol_version = version;
      RequestSendWidgetTree(w);
    }
    break;
  case Failure:
    if (!_XEditResGetString8(&stream, &err_msg)) {
      fprintf(stderr, "%s: unable to unpack protocol request\n", PROGRAM_NAME);
    } else {
      fprintf(stderr, "%s: %s\n", PROGRAM_NAME, err_msg);
    }
    break;
  default:
    fprintf(stderr, "%s: unknown error code %d\n", PROGRAM_NAME, (int)error_code);
    break;
  }
  waiting_response = FALSE;
}

/*
 * This will be called when selection "EditresComm", and will
 * call GetClientValue() via XtGetSelectionValue().
 *
 * The selection is owned by this program when send the request
 * to the client, and will be lost when the client responded to
 * the request.
 */
static void LoseSelection(Widget w, Atom *sel)
{
  XtGetSelectionValue(w, *sel, atom_client_value, GetClientValue,
                      NULL, XtLastTimestampProcessed(XtDisplay(w)));
}

/*
 * Send the Editres LocalSendWidgetTree request to the client.
 */
static void RequestSendWidgetTree(Widget w)
{
  XClientMessageEvent client_event;

  client_ident = client_ident + 1;

  _XEditResResetStream(&client_stream);
  client_stream.current = client_stream.real_top;
  client_stream.alloc = client_stream.size + (2 * HEADER_SIZE);

  _XEditResPut8(&client_stream, client_ident);
  _XEditResPut8(&client_stream, (unsigned char)EditresLocalSendWidgetTree);
  _XEditResPut32(&client_stream, client_stream.size);

  if (!XtOwnSelection(w, atom_comm, CurrentTime,
                      ConvertCommand, LoseSelection, NULL))
    fprintf(stderr, "%s: XtOwnSelection() failed\n", PROGRAM_NAME);

  client_event.window = client_window;
  client_event.type = ClientMessage;
  client_event.message_type = atom_resource_editor;
  client_event.format = EDITRES_SEND_EVENT_FORMAT;
  client_event.data.l[0] = XtLastTimestampProcessed(XtDisplay(w));
  client_event.data.l[1] = atom_comm;
  client_event.data.l[2] = (long)client_ident;
  client_event.data.l[3] = effective_protocol_version;

  XSendEvent(XtDisplay(w), client_event.window, FALSE, (long)0,
             (XEvent *)&client_event);
}

/*
 * Clients which doesn't support Editres protocol will not respond
 * to the LocalSendWidgetTree request - we must detect it as timeout.
 */
static void ClientTimeOut(XtPointer client_data, XtIntervalId *id)
{
  fprintf(stderr, "%s: client 0x%lx didn't responded to Editres Protocol request\n",
          PROGRAM_NAME, (long)client_window);
  waiting_response = FALSE;
}

/*
 * Find a widget which name matches the specified pattern, and
 * return the window-id of the widget.  If there are two or more
 * matched widgets, one of them will be returned.
 *
 * If name of the widget to be matched is "foo.bar.zot", the pattern
 * (argument "name") can be "zot", "bar.zot" or "foo.bar.zot".
 * The pattern can leaded with "*" such as "*zot", but "*" can't be
 * used between the words (i.e., "foo*zot" is not allowed).
 */
Window FindWidget(Widget w, Window client, const char *name)
{
  XEvent event;
  XtAppContext app_con;
  XtIntervalId timer_id;
  const char *pattern;
  int pattern_len, inx, i;

  if (waiting_response) {
    fprintf(stderr, "%s: list widget requested recursively (ignored)\n",
            PROGRAM_NAME);
    return None;
  }

  if (appres.debug)
    fprintf(stderr, "%s: list widget tree for window 0x%lx\n",
            PROGRAM_NAME, (long)client);

  if (atom_comm == None) {
    Display *dpy = XtDisplay(w);
    atom_comm = XInternAtom(dpy, EDITRES_COMM_ATOM, False);
    atom_command = XInternAtom(dpy, EDITRES_COMMAND_ATOM, False);
    atom_resource_editor = XInternAtom(dpy, EDITRES_NAME, False);
    atom_editres_protocol = XInternAtom(dpy, EDITRES_PROTOCOL_ATOM, False);
    atom_client_value = XInternAtom(dpy, EDITRES_CLIENT_VALUE, False);
  }

  client_window = client;
  effective_protocol_version = EDITRES_PROTOCOL_VERSION;
  app_con = XtWidgetToApplicationContext(w);

  timer_id = XtAppAddTimeOut(app_con, CLIENT_TIME_OUT, ClientTimeOut, NULL);
  waiting_response = TRUE;
  RequestSendWidgetTree(w);
  while (waiting_response) {
    XtAppNextEvent(app_con, &event);
    XtDispatchEvent(&event);
  }
  XtRemoveTimeOut(timer_id);

  if (widget_list != NULL && strlen(name) != 0) {
    pattern = name;
    if (pattern[0] == '*') pattern = pattern + 1;
    if (strchr(pattern, '*') != NULL)
      fprintf(stderr, "%s: pattern should not include \"*\": %s\n",
              PROGRAM_NAME, name);
    pattern_len = strlen(pattern);
    for (i = 0; i < num_widget_list; i++) {
      if (appres.debug)
	fprintf(stderr, "FindWidget: %s\n", widget_list[i].path);
      inx = strlen(widget_list[i].path) - pattern_len;
      if (0 <= inx) {
        if (1 <= inx && widget_list[i].path[inx - 1] != '.'
            && widget_list[i].path[inx - 1] != '*') continue;
        if (strcmp(&widget_list[i].path[inx], pattern) == 0) break;
      }
    }
    if (i < num_widget_list) {
      if (appres.debug)
        fprintf(stderr, "%s: matched widget: %s (id=0x%lx, window=0x%lx)\n",
                PROGRAM_NAME, widget_list[i].path,
                (long)widget_list[i].id, (long)widget_list[i].window);
      return widget_list[i].window;
    }
  }
  if (appres.debug && widget_list == NULL)
    fprintf(stderr, "FindWidget: couldn't get widget list\n");
  if (strlen(name) != 0) {
    fprintf(stderr, "%s: no widget matched to pattern \"%s\"\n",
            PROGRAM_NAME, name);
  }

  return None;
}
