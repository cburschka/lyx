
// -*- C++ -*-
/**
 * \file GtkLengthEntry.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GLENGTHENTRY_H
#define GLENGTHENTRY_H


#include "ghelpers.h"
#include "lengthcommon.h"

#include <gtkmm.h>
#include <libglademm.h>


class FuncRequest;

namespace lyx {
namespace frontend {

class GtkLengthEntry : public Gtk::HBox {
public:
	GtkLengthEntry::GtkLengthEntry(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	void set_length(LyXLength const & length);
	void set_length(std::string const & length);
	LyXLength const get_length();
	std::string const get_length_string();
	void set_relative(bool rel);

	Gtk::SpinButton *get_spin();
	Gtk::ComboBoxText *get_combo();

protected:
	Gtk::SpinButton spin_;
	Gtk::ComboBoxText combo_;
	bool relative_;
};

} // namespace frontend
} // namespace lyx

#endif // NOT GTKLENGTHENTRY_H
