/**
 * \file freebrowser.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * A freebrowser is a browser widget in its own form.
 * It is used for example to instantiate the drop-down list beneath a
 * combox.
 */

#ifndef FL_FREEBROWSER_H
#define FL_FREEBROWSER_H

#include "lyx_forms.h"

#if defined(__cplusplus)
extern "C"
{
#endif

typedef enum {
    FL_FREEBROWSER_BELOW,
    FL_FREEBROWSER_ABOVE
} FL_FREEBROWSER_POSITION;

typedef struct fl_freebrowser_ {
    /** Use this and you'll be told when something happens.
     *	\param == 0: the browser has merely been hidden.
     *         == 1: a change has been made.
     */
    void (* callback) (struct fl_freebrowser_ *, int action);

    /** Set this to 1 if you want to capture all KeyPress events
     *  generating printable chars.
     */
    int want_printable;
    /** Is set only if want_printable is true. */
    char last_printable;

    /** The "owner" of the freebrowser. Not used by the struct. */
    void * parent;
    /** The browser itself. */
    FL_OBJECT * browser;

    /** Details of the implementation. */
    FL_FORM * form;
    Window save_window;
} FL_FREEBROWSER;


/** A function to create a freebrowser widget, "owned" by \c parent. */
FL_EXPORT FL_FREEBROWSER *
fl_create_freebrowser(void * parent);

/** Free all memory allocated to \param ptr and to its components. */
FL_EXPORT void
fl_free_freebrowser(FL_FREEBROWSER * ptr);

/** \c abs_x, \c abs_y are the coordinates of the top left corner
 *  of the browser relative to the top left hand corner of the _screen_.
 *  \c w, \c h are the browser dimensions.
 */
FL_EXPORT void
fl_show_freebrowser(FL_FREEBROWSER *,
		    FL_Coord abs_x, FL_Coord abs_y, FL_Coord w, FL_Coord h);

FL_EXPORT void
fl_hide_freebrowser(FL_FREEBROWSER *);

#if defined(__cplusplus)
}
#endif

#endif /* not FL_FREEBROWSER_H */
