// -*- C++ -*-
/**
 * \file GDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \auther John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GDOCUMENT_H
#define GDOCUMENT_H

#include "GViewBase.h"
#include "vspace.h"

#include <gtkmm.h>

#include <map>

namespace lyx {
namespace frontend {

class ControlDocument;

/** This class provides a gtk implementation of the document dialog.
 */
class GDocument
	: public GViewCB<ControlDocument, GViewGladeB> {
public:
	GDocument(Dialog &);
private:
	/// Build the dialog
	virtual void doBuild();
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update();

	void saveAsDefaults();
	void resetToDefaults();

	// *** Start "Document" Page ***
	Gtk::ComboBoxText classcombo_;
	Gtk::Entry * extraoptionsentry_;
	Gtk::ComboBoxText psdrivercombo_;
	Gtk::ComboBoxText fontcombo_;
	Gtk::ComboBoxText fontsizecombo_;
	Gtk::Adjustment * linespacingadj_;
	Gtk::RadioButton * indentradio_;
	Gtk::RadioButton * vspaceradio_;
	Gtk::ComboBoxText vspacesizecombo_;
	std::map<int, std::string> fontsizemap_;
	std::map<int, VSpace::vspace_kind> vspacesizemap_;
	Gtk::ComboBoxText vspaceunitcombo_;
	Gtk::SpinButton * vspacelengthspin_;
	Gtk::Adjustment * vspacelengthadj_;
	void updateParagraphSeparationSensitivity();
	// *** End "Document" Page ***

};

} // namespace frontend
} // namespace lyx

#endif
