// -*- C++ -*-
/* This file is part of
 * =================================================
 *
 *          LyX, The Document Processor
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= */

#ifndef GNOME_HELPERS_H
#define GNOME_HELPERS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "debug.h"
#include <glade/glade-xml.h>
#include <glib.h>

// Glade Helper Function.


/** This function will get a widget from the glade XML representation and
 * will wrap it into the gtk--/gnome-- representation.
 */
template<class T>
T* getWidgetPtr(GladeXML* xml, char const * name)
{
	T* result = static_cast<T*>(Gtk::wrap_auto((GtkObject*)glade_xml_get_widget(xml, name)));
	if (result == NULL)
	{
		lyxerr << "** ERROR **: unable to load widget: " << name << endl;
		g_assert(result != NULL);
	}
	return result;
}

class Gdk_Font;

/** Takes a Gdk::Font object reference and returns the name associated
 * with the font it holds.
 */
string get_font_name(Gdk_Font const & font);


/** Takes a GdkFont pointer and returns the name associated with the font
 * it holds. It returns a newly allocated gchar* string.
 *
 * This function was lifted from e-font.c from the gabber package.
 */
gchar * get_font_name(GdkFont const * font);

#endif
