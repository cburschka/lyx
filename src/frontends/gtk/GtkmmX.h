// -*- C++ -*-
/**
 * \file GtkmmX.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTKMMX_H
#define GTKMMX_H

#include <X11/Xlib.h>
#include <gtkmm.h>
#include <gdk/gdkx.h>


inline Display * getDisplay()
{
	return gdk_x11_get_default_xdisplay();
}


inline int getScreen()
{
	return gdk_x11_get_default_screen();
}


inline Window getRootWindow()
{
	static Window rootWin = 
		GDK_WINDOW_XID(Gdk::Display::get_default()->
			       get_default_screen()->
			       get_root_window()->gobj());
	return rootWin;
}


inline int getDepth()
{
	static int depth;
	if (!depth) {
		int width, height, x, y;
		Gdk::Display::get_default()->get_default_screen()->
			get_root_window()->
			get_geometry(x, y, width, height, depth);
	}
	return depth;
}


inline Colormap getColormap()
{
	static Colormap colormap = GDK_COLORMAP_XCOLORMAP(
		Gdk::Display::get_default()->get_default_screen()->
		get_default_colormap()->gobj());
	return colormap;
}


#endif
