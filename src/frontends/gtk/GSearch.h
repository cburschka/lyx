// -*- C++ -*-
/**
 * \file GSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GSEARCH_H
#define GSEARCH_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlSearch;

/** This class provides a GTK+ implementation of the FormSearch Dialog.
 */
class GSearch : public GViewCB<ControlSearch, GViewGladeB> {
public:
	GSearch(Dialog & parent);
private:
	virtual void apply() {}
	virtual void doBuild();
	virtual void update();

	void onFindNext();
	void onReplace();
	void onReplaceAll();
	void onFindEntryChanged();

	Gtk::Button * findnextbutton;
	Gtk::Button * replacebutton;
	Gtk::Button * replaceallbutton;
	Gtk::Entry * findentry;
	Gtk::Entry * replaceentry;
	Gtk::CheckButton * casecheck;
	Gtk::CheckButton * matchwordcheck;
	Gtk::CheckButton * backwardscheck;
};

} // namespace frontend
} // namespace lyx

#endif // GSEARCH_H
