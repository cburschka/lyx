/**
 * \file combox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 *
 * A combination of two objects (a button and a browser) is encapsulated to
 * get a combobox-like object.
 */

#ifndef FL_COMBOX_H
#define FL_COMBOX_H

#if defined(__cplusplus)
extern "C"
{
#endif

/** This will eventually be moved into the enum of in-built widgets
    in forms.h. */
enum {
    FL_COMBOX = 200
};

/** The various types of combox. */
typedef enum {
    FL_NORMAL_COMBOX,
    FL_DROPLIST_COMBOX
} FL_COMBOX_TYPE;

/** How the browser should be displayed relative to the buttons. */
typedef enum {
    FL_COMBOX_BELOW,
    FL_COMBOX_ABOVE
} FL_COMBOX_POSITION;

/** A function to create a combox widget.
 *  \param type is, as yet, unused. there is only one type of combox.
 *  \param x, \param y: the x,y coordinates of the upper left hand corner
 *  of the widget, relative to the parent form'd origin.
 *  \param w, \param h: the widget's dimensions.
 *  \param label: the widget's label as it appears on the GUI.
 */
FL_EXPORT FL_OBJECT *
fl_create_combox(FL_COMBOX_TYPE type,
		 FL_Coord x, FL_Coord y, FL_Coord w, FL_Coord h,
		 char const * label);

/** A function to create a combox widget and add it to the parent form.
 *  \see fl_create_combox() for an explanation of the argument list.
 */
FL_EXPORT FL_OBJECT *
fl_add_combox(FL_COMBOX_TYPE type,
	      FL_Coord x, FL_Coord y, FL_Coord w, FL_Coord h,
	      char const * label);

/** The combox browser has a default height of 100 pixels. Adjust to suit. */
FL_EXPORT void fl_set_combox_browser_height(FL_OBJECT * ob, int bh);

/** The browser will be displayed either below or above the button,
 *  dependent upon \param position.
 */
FL_EXPORT void fl_set_combox_position(FL_OBJECT * ob, 
				      FL_COMBOX_POSITION position);

/** Empty the browser and the combox, \param ob. */
FL_EXPORT void fl_clear_combox(FL_OBJECT * ob);

/** Add a line to the combox browser.*/
FL_EXPORT void fl_addto_combox(FL_OBJECT * ob, char const * text);

/** Set the combox to return line \param choice of the combox browser. */
FL_EXPORT void fl_set_combox(FL_OBJECT * ob, int choice);

/** \return the currently selected line of the combox browser. */
FL_EXPORT int fl_get_combox(FL_OBJECT * ob);

/** \return the contents of the combox.
 *  (Also the contents of currently selected line of the combox browser.)
 */
FL_EXPORT char const * fl_get_combox_text(FL_OBJECT * ob);

/** \return the contents of \param line of the combox browser. */
FL_EXPORT char const * fl_get_combox_line(FL_OBJECT * ob, int line);

/** \return the number of items in the combox browser. */
FL_EXPORT int fl_get_combox_maxitems(FL_OBJECT * ob);

/** Show the browser */
void fl_show_combox_browser(FL_OBJECT * ob);

/** Hide the browser */
void fl_hide_combox_browser(FL_OBJECT * ob);

#if defined(__cplusplus)
}
#endif

#endif
