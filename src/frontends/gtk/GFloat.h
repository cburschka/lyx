// -*- C++ -*-
/**
 * \file GFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GFLOAT_H
#define GFLOAT_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlFloat;

/** This class provides a GTK+ implementation of the ERT Dialog.
 */
class GFloat : public GViewCB<ControlFloat, GViewGladeB> {
public:
	GFloat(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	void updateSensitivity();

	// apply() won't act when this is true
	bool applylock_;

	Gtk::RadioButton * defaultradio_;
	Gtk::RadioButton * heredefinitelyradio_;
	Gtk::RadioButton * alternativeradio_;
	Gtk::CheckButton * topcheck_;
	Gtk::CheckButton * bottomcheck_;
	Gtk::CheckButton * pageoffloatscheck_;
	Gtk::CheckButton * hereifpossiblecheck_;
	Gtk::CheckButton * ignorerulescheck_;
	Gtk::CheckButton * spancolumnscheck_;
	Gtk::CheckButton * rotatesidewayscheck_;
};

} // namespace frontend
} // namespace lyx

#endif // GFLOAT_H
