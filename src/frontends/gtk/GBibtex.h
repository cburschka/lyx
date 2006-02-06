// -*- C++ -*-
/**
 * \file GBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GBIBTEX_H
#define GBIBTEX_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlBibtex;

/** This class provides a GTK+ implementation of the Bibtex Dialog.
 */
class GBibtex : public GViewCB<ControlBibtex, GViewGladeB> {
public:
	GBibtex(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	void add();
	void remove();
	void browse();
	void validate();

	Gtk::TreeView *databasesview_;
	Glib::RefPtr<Gtk::TreeView::Selection> databasessel_;
	Glib::RefPtr<Gtk::ListStore> databasesstore_;
	Gtk::TreeModelColumn<Glib::ustring> stringcol_;
	Gtk::Button *removebutton_;
	Gtk::ComboBoxEntryText stylecombo_;
	Gtk::ComboBox *contentcombo_;
	Gtk::CheckButton *toccheck_;
};

} // namespace frontend
} // namespace lyx

#endif // GBIBTEX_H
