// -*- C++ -*-
/**
 * \file GThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Reiter
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTHESAURUS_H
#define GTHESAURUS_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlThesaurus;

/** This class provides a GTK+ implementation of the Thesaurus Dialog.
 */
class GThesaurus : public GViewCB<ControlThesaurus, GViewGladeB> {
public:
	GThesaurus(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	/// updates the synonym list (e.g. when the keyword is changed)
	void update_lists();
	/// enables the apply button if a synonym is selected from the list 
	void selection_changed();
	/// changes the keyword entry content to the synonym double-clicked on
	void meaningsview_activated(const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*);

	/** apply() won't act when this is true. 
	    true if no text is selected when the thesaurus dialog is opened 
	 */
	bool applylock_;

	Gtk::Button * cancelbutton_;
	Gtk::Button * okbutton_;
	Gtk::Button * applybutton_;
	Gtk::Entry * keywordentry_;
	Gtk::TreeView * meaningsview_;
	Glib::RefPtr<Gtk::TreeStore> synTreeStore_;

};

} // namespace frontend
} // namespace lyx

#endif // GTHESAURUS_H
