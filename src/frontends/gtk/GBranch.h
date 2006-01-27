// -*- C++ -*-
/**
 * \file GBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Reiter
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GBRANCH_H
#define GBRANCH_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlBranch;

/** This class provides a GTK+ implementation of the Branch Dialog.
 */
class GBranch : public GViewCB<ControlBranch, GViewGladeB> {
public:
	GBranch(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	/// enables the apply button if a synonym is selected from the list 
	void selection_changed();

	/** apply() won't act when this is true. 
	    true if no text is selected when the Branch dialog is opened 
	 */
	bool applylock_;

	Gtk::Button * cancelbutton_;
	Gtk::Button * okbutton_;
	Gtk::Label * brancheslabel_;
	Gtk::ComboBoxText branchescombo_;
};

} // namespace frontend
} // namespace lyx

#endif // GBRANCH_H
