// -*- C++ -*-
/**
 * \file gnome_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GNOME_HELPERS_H
#define GNOME_HELPERS_H


#include "debug.h"

#undef Status
#undef Value
#undef DestroyNotify

#include <gtkmm/textbuffer.h>


/**
 * This helper function adds default tags to a TextBuffer
 *
 * Hopefully gtk will come with its own one day, but until then...
 */
void addDefaultTags(Glib::RefPtr<Gtk::TextBuffer> &);

#endif
