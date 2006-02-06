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

#include "BranchList.h"

#include "GtkLengthEntry.h"

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
	GtkLengthEntry *vspacelengthentry_;
	void updateParagraphSeparationSensitivity();
	void classChanged();
	// *** End "Document" Page ***

	// *** Begin "Page" Page ***
	GtkLengthEntry * pagewidthlengthentry_;
	GtkLengthEntry * pageheightlengthentry_;
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
	GtkLengthEntry * mtoplengthentry_;
	GtkLengthEntry * mbottomlengthentry_;
	GtkLengthEntry * minnerlengthentry_;
	GtkLengthEntry * mouterlengthentry_;
	GtkLengthEntry * mheadseplengthentry_;
	GtkLengthEntry * mheadheightlengthentry_;
	GtkLengthEntry * mfootskiplengthentry_;
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

	// *** Start "Numbering" Page ***
	Gtk::Adjustment * numberingadj_;
	Gtk::Adjustment * TOCadj_;
	Gtk::Label * numberinglabel_;
	Gtk::Label * TOClabel_;
	void numberingChanged();
	void TOCChanged();
	// *** End "Numbering" Page ***

	// *** Start "Bibliography" Page ***
	Gtk::RadioButton * basicnumericalradio_;
	Gtk::RadioButton * natbibnumericalradio_;
	Gtk::RadioButton * natbibauthoryearradio_;
	Gtk::RadioButton * jurabibradio_;
	Gtk::ToggleButton * sectionedbibliographytoggle_;
	// *** End "Bibliography" Page ***

	// *** Start "Math" Page ***
	Gtk::RadioButton * AMSautomaticallyradio_;
	Gtk::RadioButton * AMSalwaysradio_;
	Gtk::RadioButton * AMSneverradio_;
	// *** End "Math" Page ***

	// *** Start "Floats" Page ***
	Gtk::RadioButton * defaultradio_;
	Gtk::RadioButton * heredefinitelyradio_;
	Gtk::RadioButton * alternativeradio_;
	Gtk::CheckButton * topcheck_;
	Gtk::CheckButton * bottomcheck_;
	Gtk::CheckButton * pageoffloatscheck_;
	Gtk::CheckButton * hereifpossiblecheck_;
	Gtk::CheckButton * ignorerulescheck_;
	void alternativeChanged();
	// *** End "Floats" Page ***

	// *** Start "Bullets" Page ***
	// *** End "Bullets" Page ***

	// *** Start "Branches" Page ***
	//BranchList branchlist_;
	Gtk::TreeView * branchesview_;
	Gtk::Button * addbranchbutton_;
	Gtk::Button * removebranchbutton_;
	Gtk::TreeModelColumn<Glib::ustring> branchColName_;
	Gtk::TreeModelColumn<bool> branchColActivated_;
	Gtk::TreeModelColumn<Glib::ustring> branchColColor_;
	Gtk::TreeModel::ColumnRecord branchCols_;

	Glib::RefPtr<Gtk::ListStore> branchliststore_;
	Glib::RefPtr<Gtk::TreeSelection> branchsel_;
	void addBranch();
	void removeBranch();
	void branchSelChanged();
	// *** End "Branches" Page ***

	// *** Begin "Preamble" Page ***
	Glib::RefPtr<Gtk::TextBuffer> preamblebuffer_;
	// *** End "Preamble" Page ***
};

} // namespace frontend
} // namespace lyx

#endif
