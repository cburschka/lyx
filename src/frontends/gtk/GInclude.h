// -*- C++ -*-
/**
 * \file GInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GINCLUDE_H
#define GINCLUDE_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlInclude;

/** This class provides a GTK+ implementation of the Include Dialog.
 */
class GInclude : public GViewCB<ControlInclude, GViewGladeB> {
public:
	GInclude(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	Gtk::RadioButton * includeradio_;
	Gtk::RadioButton * inputradio_;
	Gtk::RadioButton * verbatimradio_;
	Gtk::Entry * fileentry_;
	Gtk::Button * openbutton_;
	Gtk::CheckButton * markspacescheck_;
	Gtk::CheckButton * previewcheck_;

	void onBrowseClicked();
	void onOpenClicked();
	void onTypeToggled();
};

} // namespace frontend
} // namespace lyx

#endif // GINCLUDE_H
