// -*- C++ -*-
/**
 * \file GSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GSPELLCHECKER_H
#define GSPELLCHECKER_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlSpellchecker;

/** This class provides a GTK+ implementation of the FormSpellchecker Dialog.
 */
class GSpellchecker
	: public GViewCB<ControlSpellchecker, GViewGladeB> {
public:

	GSpellchecker(Dialog &);
private:
	// not needed.
	virtual void apply() {}
	// Build the dialog
	virtual void doBuild();

	virtual void update() {}

	virtual void show();

	// Fill in unknown word, suggestions, progress from backend
	virtual void partialUpdate(int);

	Glib::RefPtr<Gtk::ListStore> suggestionsstore_;
	Glib::RefPtr<Gtk::TreeSelection> suggestionssel_;

	Gtk::TreeModelColumn<Glib::ustring> listCol_;
	Gtk::TreeModel::ColumnRecord listCols_;

	Gtk::TreeView * suggestionsview_;
	Gtk::Entry * replacemententry_;
	Gtk::Entry * unknownentry_;
	Gtk::ProgressBar * progress_;
	Gtk::Button * ignorebutton_;

	// Replace button
	void onReplace(bool const all);
	// Suggestion list single click
	void onSuggestionSelection();
	// Suggestion list double click
	void onSuggestionActivate(Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * col);
	// Ignore button
	void onIgnore();
	// Ignore All button
	void onIgnoreAll();
	// Add button
	void onAdd();
};

} // namespace frontend
} // namespace lyx

#endif // GSPELLCHECKER_H
