// -*- C++ -*-
/**
 * \file GDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
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
	std::map<int, VSpace::vspace_kind> vspacesizemap_;
	Gtk::ComboBoxText vspaceunitcombo_;
	Gtk::SpinButton * vspacelengthspin_;
	Gtk::Adjustment * vspacelengthadj_;
	void updateParagraphSeparationSensitivity();
	void classChanged();
	// *** End "Document" Page ***

	// *** Begin "Page" Page ***
	Gtk::SpinButton * pagewidthspin_;
	Gtk::SpinButton * pageheightspin_;
	Gtk::ComboBoxText pagewidthunitscombo_;
	Gtk::ComboBoxText pageheightunitscombo_;
	Gtk::ComboBoxText pagesizecombo_;
	Gtk::RadioButton * portraitradio_;
	Gtk::RadioButton * landscaperadio_;
	Gtk::ComboBoxText pagestylecombo_;
	Gtk::ToggleButton * doublesidedtoggle_;
	Gtk::ToggleButton * twocolumnstoggle_;
	void pageSizeChanged();
	// *** End "Page" Page ***

	// *** Begin "Margins" Page ***
	Gtk::CheckButton * defaultmargins_;
	Gtk::SpinButton * mtopspin_;
	Gtk::SpinButton * mbottomspin_;
	Gtk::SpinButton * minnerspin_;
	Gtk::SpinButton * mouterspin_;
	Gtk::SpinButton * mheadsepspin_;
	Gtk::SpinButton * mheadheightspin_;
	Gtk::SpinButton * mfootskipspin_;
	Gtk::ComboBoxText mtopunitcombo_;
	Gtk::ComboBoxText mbottomunitcombo_;
	Gtk::ComboBoxText minnerunitcombo_;
	Gtk::ComboBoxText mouterunitcombo_;
	Gtk::ComboBoxText mheadsepunitcombo_;
	Gtk::ComboBoxText mheadheightunitcombo_;
	Gtk::ComboBoxText mfootskipunitcombo_;
	void marginsChanged();
	// *** End "Margins" Page ***

	// *** Start "Language" Page ***
	Gtk::ComboBoxText languagecombo_;
	Gtk::ComboBoxText encodingcombo_;
	Gtk::RadioButton * qenglishradio_;
	Gtk::RadioButton * qswedishradio_;
	Gtk::RadioButton * qgermanradio_;
	Gtk::RadioButton * qpolishradio_;
	Gtk::RadioButton * qfrenchradio_;
	Gtk::RadioButton * qdanishradio_;
	std::vector<std::string> lang_;
	// *** End "Language" Page ***
};

} // namespace frontend
} // namespace lyx

#endif
