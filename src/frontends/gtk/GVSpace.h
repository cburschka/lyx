// -*- C++ -*-
/**
 * \file GVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GVSPACE_H
#define GVSPACE_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlVSpace;

/** This class provides a GTK+ implementation of the VSpace Dialog.
 */
class GVSpace : public GViewCB<ControlVSpace, GViewGladeB> {
public:
	GVSpace(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	void onSpacingComboChanged();

	Gtk::ComboBox * spacingcombo_;
	Gtk::SpinButton * valuespin_;
	Gtk::ComboBoxText valueunitscombo_;
	Gtk::CheckButton * protectcheck_;
};

} // namespace frontend
} // namespace lyx

#endif // GVSPACE_H
