// -*- C++ -*-
/**
 * \file GRef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 * \author Andreas Klostermann
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GREF_H
#define GREF_H

#include "GViewBase.h"
#include <string>
namespace lyx {
namespace frontend {

class ControlRef;

/** This class provides a GTK+ implementation of the Note Dialog.
 */
class GRef : public GViewCB<ControlRef, GViewGladeB> {
public:
	GRef(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();
	virtual void update_labels();
	// Signal callbacks
	void selection_changed ();
	void buffer_changed();
	void update_validity();
	void jumpto();
	void back();
	void refview_activated(const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*);
	// apply() won't act when this is true
	bool applylock_;
	std::string lastbuffer_;
	Gtk::Entry * labelentry_;
	Gtk::Entry * nameentry_;
	Gtk::TreeView * refview_;
	Glib::RefPtr<Gtk::ListStore> refListStore_;
	Glib::RefPtr<Gtk::ListStore> bufferstore_;
	Gtk::ComboBox * formatcombo_;
	Gtk::ComboBox * buffercombo_;
	Gtk::Button * jumptobutton_;
	Gtk::Button * backbutton_;
	Gtk::Button * cancelbutton_;
	Gtk::Button * okbutton_;
	Gtk::Button * applybutton_;
	Gtk::Button * refreshbutton_;

};

} // namespace frontend
} // namespace lyx

#endif // GREF_H
