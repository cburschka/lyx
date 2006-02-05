// -*- C++ -*-
/**
 * \file GPreferences.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GPREFERENCES_H
#define GPREFERENCES_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlPrefs;

/** This class provides a GTK+ implementation of the Preferences Dialog.
 */
class GPreferences : public GViewCB<ControlPrefs, GViewGladeB> {
public:
	GPreferences(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();

	// apply() won't act when this is true
	bool applylock_;

	// >>> Font tab
	Gtk::FontButton *romanfontbutton_;
	Gtk::FontButton *sansseriffontbutton_;
	Gtk::FontButton *typewriterfontbutton_;
	Gtk::Adjustment *dpiadj_;
	Gtk::Adjustment *zoomadj_;
	// <<< Font tab
};

} // namespace frontend
} // namespace lyx

#endif // GPREFERENCES_H
