// -*- C++ -*-
/**
 * \file GChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GCHANGES_H
#define GCHANGES_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlChanges;

/**
 * This class provides a GTK+ implementation of the Merge Changes Dialog.
 */
class GChanges
	: public GViewCB<ControlChanges, GViewGladeB> {
public:
	GChanges(Dialog &);

private:
	/// not needed.
	virtual void apply() {}
	/// Build the dialog
	virtual void doBuild();
	/// update the dialog
	virtual void update();

	void onAccept();
	void onReject();
	void onNext();

	void promptChange();
	void promptDismiss();

	Gtk::Label * messagelabel_;
	Gtk::Button * nextbutton_;
	Gtk::Button * acceptbutton_;
	Gtk::Button * rejectbutton_;
};

} // namespace frontend
} // namespace lyx

#endif // GCHANGES_H
