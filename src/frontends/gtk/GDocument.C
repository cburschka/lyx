/**
 * \file GDocument.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GDocument.h"
#include "ghelpers.h"

#include "ControlDocument.h"
#include "controllers/helper_funcs.h"

#include "support/lstrings.h"

#include "lyxtextclasslist.h"
#include "bufferparams.h"
#include "tex-strings.h"

using std::string;

namespace lyx {

using support::bformat;

namespace frontend {

GDocument::GDocument(Dialog & parent)
	: GViewCB<ControlDocument, GViewGladeB>(parent, _("Document Settings"), false)
{}

void GDocument::doBuild()
{
	string const gladeName = findGladeFile("document");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	// Manage the action area buttons
	Gtk::Button * button;
	xml_->get_widget("Cancel", button);
	setCancel(button);
	xml_->get_widget("OK", button);
	setOK(button);
	xml_->get_widget("Apply", button);
	setApply(button);
	xml_->get_widget("Revert", button);
	setRestore(button);

	xml_->get_widget("UseClassDefaults", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GDocument::resetToDefaults));
	xml_->get_widget("SaveAsDocumentDefaults", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GDocument::saveAsDefaults));


	// *** Start "Document" Page ***
	Gtk::Box * box = NULL;
	xml_->get_widget("DocumentClass", box);
	// Prevent combo making dialog super-wide due to long class names
	classcombo_.set_size_request(1, -1);
	box->pack_start(classcombo_, true, true, 0);
	box->show_all();

	// Populate Document Class combo
	for (LyXTextClassList::const_iterator cit = textclasslist.begin();
	     cit != textclasslist.end(); ++cit) {
		if (cit->isTeXClassAvailable()) {
			classcombo_.append_text(cit->description());
		} else {
			string item =
				bformat(_("Unavailable: %1$s"), cit->description());
			classcombo_.append_text(item);
		}
	}

	xml_->get_widget("ExtraOptions", extraoptionsentry_);

	xml_->get_widget("PostscriptDriver", box);
	box->pack_start(psdrivercombo_, true, true, 0);
	box->show_all();

	// Populate Postscript driver combo
	for (int i = 0; tex_graphics[i][0]; ++i) {
		psdrivercombo_.append_text(tex_graphics[i]);
	}

	xml_->get_widget("Font", box);
	box->pack_start(fontcombo_, true, true, 0);
	box->show_all();

	// Populate font combo
	for (int i = 0; tex_fonts[i][0]; ++i) {
		fontcombo_.append_text(tex_fonts[i]);
	}

	xml_->get_widget("FontSize", box);
	box->pack_start(fontsizecombo_, true, true, 0);
	box->show_all();
	fontsizecombo_.append_text(_("Default"));
	fontsizecombo_.append_text(_("10"));
	fontsizecombo_.append_text(_("11"));
	fontsizecombo_.append_text(_("12"));
	// These are the corresponding strings to be passed to the backend
	fontsizemap_[0] = "default";
	fontsizemap_[1] = "10";
	fontsizemap_[2] = "11";
	fontsizemap_[3] = "12";

	Gtk::SpinButton * spin;
	xml_->get_widget("LineSpacing", spin);
	linespacingadj_ = spin->get_adjustment();

	xml_->get_widget("Indentation", indentradio_);
	xml_->get_widget("VerticalSpace", vspaceradio_);
	vspaceradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GDocument::updateParagraphSeparationSensitivity));

	xml_->get_widget("VerticalSpaceSize", box);
	box->pack_start(vspacesizecombo_, true, true, 0);
	box->show_all();
	// The order of these items is magic
	vspacesizecombo_.append_text(_("Small Skip"));
	vspacesizecombo_.append_text(_("Medium Skip"));
	vspacesizecombo_.append_text(_("Big Skip"));
	vspacesizecombo_.append_text(_("Custom"));
	vspacesizemap_[0] = VSpace::SMALLSKIP;
	vspacesizemap_[1] = VSpace::MEDSKIP;
	vspacesizemap_[2] = VSpace::BIGSKIP;
	vspacesizemap_[3] = VSpace::LENGTH;

	vspacesizecombo_.signal_changed().connect(
		sigc::mem_fun(*this,
		&GDocument::updateParagraphSeparationSensitivity));

	xml_->get_widget("VerticalSpaceLength", vspacelengthspin_);
	vspacelengthadj_ = vspacelengthspin_->get_adjustment();

	xml_->get_widget("VerticalSpaceUnit", box);
	box->pack_start(vspaceunitcombo_, true, true, 0);
	box->show_all();

	populateUnitCombo(vspaceunitcombo_, false);

	updateParagraphSeparationSensitivity();

	// *** End "Document" Page ***
}


void GDocument::update()
{
	BufferParams & params = controller().params();

	// *** Start "Document" Page ***

	// Document Class
	classcombo_.set_active(params.textclass);

	// Extra Options
	extraoptionsentry_->set_text(params.options);

	// Postscript driver
	comboBoxTextSet(psdrivercombo_, params.graphicsDriver);

	// Font & Size
	comboBoxTextSet(fontcombo_, params.fonts);
	for (int i = 0; i <= 3; ++i) {
		if (fontsizemap_[i] == params.fontsize)
			fontsizecombo_.set_active(i);
	}

	// Line Spacing
	linespacingadj_->set_value(params.spacing().getValue());

	// Paragraph Separation
	if (params.paragraph_separation == BufferParams::PARSEP_INDENT) {
		indentradio_->set_active(true);
	} else {
		vspaceradio_->set_active(true);
	}

	// Paragraph Separation Vertical Space Size
	VSpace::vspace_kind const skipkind = params.getDefSkip().kind();
	for (int i = 0; i <= 3; ++i) {
		if (vspacesizemap_[i] == skipkind)
			vspacesizecombo_.set_active(i);
	}

	LyXLength vspacelen = params.getDefSkip().length().len();
	setWidgetsFromLength(*vspacelengthadj_, vspaceunitcombo_, vspacelen);

	// *** End "Document" Page ***

	// Be a cheesy bastard, for the moment
	bc().valid();
}


void GDocument::apply()
{
	BufferParams & params = controller().params();

	// *** Start "Document" Page ***

	// Document Class
	params.textclass = classcombo_.get_active_row_number();

	// Extra Options
	params.options = extraoptionsentry_->get_text();

	// Postscript Driver
	params.graphicsDriver = psdrivercombo_.get_active_text();

	// Font & Size
	params.fonts = fontcombo_.get_active_text();
	params.fontsize =
		fontsizemap_[fontsizecombo_.get_active_row_number()];

	// Line Spacing
	params.spacing().set(Spacing::Other, linespacingadj_->get_value());

	// Paragraph Separation
	if (indentradio_->get_active()) {
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	} else {
		params.paragraph_separation = BufferParams::PARSEP_SKIP;
	}

	// Paragraph Separation Vertical Space Size
	VSpace::vspace_kind const selection =
		vspacesizemap_[vspacesizecombo_.get_active_row_number()];
	params.setDefSkip(VSpace(selection));
	if (selection == VSpace::LENGTH) {
		string const length =
		getLengthFromWidgets(*vspacelengthadj_,
				     vspaceunitcombo_);

		params.setDefSkip(VSpace(LyXGlueLength(length)));
	}

	// *** End "Document" Page ***
}


void GDocument::saveAsDefaults()
{
	apply();
	controller().saveAsDefault();
}


void GDocument::resetToDefaults()
{
	BufferParams & params = controller().params();
	params.textclass = classcombo_.get_active_row_number();
	params.useClassDefaults();
	update();
}


void GDocument::updateParagraphSeparationSensitivity()
{
	bool const vspacesensitive = vspaceradio_->get_active();

	vspacesizecombo_.set_sensitive(vspacesensitive);

	bool const lengthsensitive = vspacesensitive &&
		(vspacesizecombo_.get_active_row_number() == 3);

	vspacelengthspin_->set_sensitive(lengthsensitive);
	vspaceunitcombo_.set_sensitive(lengthsensitive);
}


} // namespace frontend
} // namespace lyx
