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

	// >>> Font tab
	Gtk::FontButton *romanfontbutton_;
	Gtk::FontButton *sansseriffontbutton_;
	Gtk::FontButton *typewriterfontbutton_;
	Gtk::Adjustment *dpiadj_;
	Gtk::Adjustment *zoomadj_;
	// <<< Font tab
	
	// >>> Graphics tab
	Gtk::RadioButton *graphicscolorradio_;
	Gtk::RadioButton *graphicsgrayscaleradio_;
	Gtk::RadioButton *graphicsmonoradio_;
	Gtk::RadioButton *graphicsnoneradio_;
	
	Gtk::RadioButton *instprevonradio_;
	Gtk::RadioButton *instprevoffradio_;
	Gtk::RadioButton *instprevnomathradio_;
	// <<< Graphics tab
	
	// >>> Keyboard tab
	void keyboard_sensitivity ();
	Gtk::CheckButton *keyboardmapcheck_;
	Gtk::FileChooserButton keyboardmap1fcbutton_;
	Gtk::FileChooserButton keyboardmap2fcbutton_;
	// <<< Keyboard tab
};

} // namespace frontend
} // namespace lyx

#endif // GPREFERENCES_H
