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



#endif
