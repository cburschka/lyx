/**
 * \file gnome_helpers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "gnome_helpers.h"
#include <gtkmm/texttag.h>

void addDefaultTags(Glib::RefPtr<Gtk::TextBuffer> & buf) {
	Glib::RefPtr<Gtk::TextTag> italicTag = Gtk::TextTag::create("italic");
	italicTag->property_style() = Pango::STYLE_ITALIC;

	Glib::RefPtr<Gtk::TextTag> boldTag =  Gtk::TextTag::create("bold");
	boldTag->property_weight() = Pango::WEIGHT_BOLD;

	buf->get_tag_table()->add(italicTag);
	buf->get_tag_table()->add(boldTag);

}
	
