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
#include "controllers/frnt_lang.h"

#include "support/lstrings.h"
#include "lyxrc.h"

#include "lyxtextclasslist.h"
#include "bufferparams.h"
#include "tex-strings.h"
#include "language.h"

using std::string;
using std::vector;

namespace lyx {

using support::bformat;
using support::token;

namespace frontend {

namespace {

char const * const encodings[] = { "Language Default", "LaTeX default",
	"latin1", "latin2", "latin3", "latin4", "latin5", "latin9",
	"koi8-r", "koi8-u", "cp866", "cp1251", "iso88595", "pt154", 0
};

}


GDocument::GDocument(Dialog & parent)
	: GViewCB<ControlDocument, GViewGladeB>(parent, _("Document Settings"), false),
	lang_(getSecond(getLanguageData(false)))
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

	classcombo_.signal_changed().connect(
		sigc::mem_fun(*this, &GDocument::classChanged));

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

	// *** Begin "Page" Page ***
	xml_->get_widget("PageSize", box);
	box->pack_start(pagesizecombo_, true, true, 0);
	box->show_all();
	pagesizecombo_.append_text(_("Default"));
	pagesizecombo_.append_text(_("Custom"));
	pagesizecombo_.append_text(_("US letter"));
	pagesizecombo_.append_text(_("US legal"));
	pagesizecombo_.append_text(_("US executive"));
	pagesizecombo_.append_text(_("A3"));
	pagesizecombo_.append_text(_("A4"));
	pagesizecombo_.append_text(_("A5"));
	pagesizecombo_.append_text(_("B3"));
	pagesizecombo_.append_text(_("B4"));
	pagesizecombo_.append_text(_("B5"));
	pagesizecombo_.signal_changed().connect(
		sigc::mem_fun(*this, &GDocument::pageSizeChanged));

	xml_->get_widget("PageWidth", pagewidthspin_);
	xml_->get_widget("PageHeight", pageheightspin_);
	xml_->get_widget("PageWidthUnits", box);
	box->pack_start(pagewidthunitscombo_, true, true, 0);
	box->show_all();
	populateUnitCombo(pagewidthunitscombo_, false);
	xml_->get_widget("PageHeightUnits", box);
	box->pack_start(pageheightunitscombo_, true, true, 0);
	box->show_all();
	populateUnitCombo(pageheightunitscombo_, false);

	xml_->get_widget("Portrait", portraitradio_);
	xml_->get_widget("Landscape", landscaperadio_);

	xml_->get_widget("PageStyle", box);
	box->pack_start(pagestylecombo_, true, true, 0);
	box->show_all();
	xml_->get_widget("DoubleSided", doublesidedtoggle_);
	xml_->get_widget("TwoColumns", twocolumnstoggle_);
	// *** End "Page" Page ***

	// *** Begin "Margins" Page ***
	xml_->get_widget("Margins", box);
	box->pack_start(marginscombo_, true, true, 0);
	box->show_all();
	marginscombo_.signal_changed().connect(
		sigc::mem_fun(*this, &GDocument::marginsChanged));
	pagesizecombo_.signal_changed().connect(
		sigc::mem_fun(*this, &GDocument::populateMargins));

	xml_->get_widget("MarginTop", mtopspin_);
	xml_->get_widget("MarginBottom", mbottomspin_);
	xml_->get_widget("MarginInner", minnerspin_);
	xml_->get_widget("MarginOuter", mouterspin_);
	xml_->get_widget("MarginHeadSep", mheadsepspin_);
	xml_->get_widget("MarginHeadHeight", mheadheightspin_);
	xml_->get_widget("MarginFootSkip", mfootskipspin_);

	xml_->get_widget("MarginTopUnits", box);
	box->pack_start(mtopunitcombo_, true, true, 0);
	populateUnitCombo(mtopunitcombo_, false);
	xml_->get_widget("MarginBottomUnits", box);
	box->pack_start(mbottomunitcombo_, true, true, 0);
	populateUnitCombo(mbottomunitcombo_, false);
	xml_->get_widget("MarginInnerUnits", box);
	box->pack_start(minnerunitcombo_, true, true, 0);
	populateUnitCombo(minnerunitcombo_, false);
	xml_->get_widget("MarginOuterUnits", box);
	box->pack_start(mouterunitcombo_, true, true, 0);
	populateUnitCombo(mouterunitcombo_, false);
	xml_->get_widget("MarginHeadSepUnits", box);
	box->pack_start(mheadsepunitcombo_, true, true, 0);
	populateUnitCombo(mheadsepunitcombo_, false);
	xml_->get_widget("MarginHeadHeightUnits", box);
	box->pack_start(mheadheightunitcombo_, true, true, 0);
	populateUnitCombo(mheadheightunitcombo_, false);
	xml_->get_widget("MarginFootSkipUnits", box);
	box->pack_start(mfootskipunitcombo_, true, true, 0);
	populateUnitCombo(mfootskipunitcombo_, false);

	Gtk::Table * table;
	xml_->get_widget("MarginsTable", table);
	table->show_all();
	// *** End "Margins" Page ***

	// *** Start "Language" Page ***
	xml_->get_widget("Language", box);
	box->pack_start(languagecombo_, true, true, 0);
	box->show_all();

	vector<LanguagePair> const langs = getLanguageData(false);
	vector<LanguagePair>::const_iterator lit  = langs.begin();
	vector<LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		languagecombo_.append_text(lit->first);
	}

	xml_->get_widget("TextEncoding", box);
	box->pack_start(encodingcombo_, true, true, 0);
	box->show_all();
	
	int i = 0;
	while (encodings[i])
		encodingcombo_.append_text(encodings[i++]);

	xml_->get_widget("EnglishQuote", qenglishradio_);
	xml_->get_widget("SwedishQuote", qswedishradio_);
	xml_->get_widget("GermanQuote", qgermanradio_);
	xml_->get_widget("PolishQuote", qpolishradio_);
	xml_->get_widget("FrenchQuote", qfrenchradio_);
	xml_->get_widget("DanishQuote", qdanishradio_);
	// *** End "Language" Page ***
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
	comboBoxTextSet(fontsizecombo_, params.fontsize);

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

	// *** Begin "Page" Page ***
	int const psize = params.papersize2;
	pagesizecombo_.set_active(psize);

	setWidgetsFromLength(
		*(pagewidthspin_->get_adjustment()),
		pagewidthunitscombo_, LyXLength(params.paperwidth));
	setWidgetsFromLength(
		*(pageheightspin_->get_adjustment()),
		pageheightunitscombo_, LyXLength(params.paperheight));

	if (params.orientation == ORIENTATION_PORTRAIT)
		portraitradio_->set_active();
	else
		landscaperadio_->set_active();

	if (params.columns == 2)
		twocolumnstoggle_->set_active(true);
	else
		twocolumnstoggle_->set_active(false);

	doublesidedtoggle_->set_active(params.sides == LyXTextClass::TwoSides);

	comboBoxTextSet(pagestylecombo_, params.pagestyle);
	// *** End "Page" Page ***

	// *** Begin "Margins" Page ***
	int item = params.paperpackage;
	if (params.use_geometry) {
		item = 1;
	} else if (item > 0) {
		item = item + 1;
	}
	marginscombo_.set_active(item);

	setWidgetsFromLength(
		*(mtopspin_->get_adjustment()),
		mtopunitcombo_,
		LyXLength(params.topmargin));
	setWidgetsFromLength(
		*(mbottomspin_->get_adjustment()),
		mbottomunitcombo_,
		LyXLength(params.bottommargin));
	setWidgetsFromLength(
		*(minnerspin_->get_adjustment()),
		minnerunitcombo_,
		LyXLength(params.leftmargin));
	setWidgetsFromLength(
		*(mouterspin_->get_adjustment()),
		mouterunitcombo_,
		LyXLength(params.rightmargin));
	setWidgetsFromLength(
		*(mheadsepspin_->get_adjustment()),
		mheadsepunitcombo_,
		LyXLength(params.headsep));
	setWidgetsFromLength(
		*(mheadheightspin_->get_adjustment()),
		mheadheightunitcombo_,
		LyXLength(params.headheight));
	setWidgetsFromLength(
		*(mfootskipspin_->get_adjustment()),
		mfootskipunitcombo_,
		LyXLength(params.footskip));

	// *** End "Margins" Page ***

	// *** Start "Language" Page ***
	int const langpos = findPos(lang_,
		params.language->lang());
	languagecombo_.set_active(langpos);

	// Default to "auto"
	encodingcombo_.set_active(0);
	if (params.inputenc == "default") {
		encodingcombo_.set_active(1);
	} else {
		int i = 0;
		while (encodings[i]) {
			if (encodings[i] == params.inputenc)
				encodingcombo_.set_active(i);
			++i;
		}
	}

	switch (params.quotes_language) {
	case InsetQuotes::EnglishQ:
		qenglishradio_->set_active();
		break;
	case InsetQuotes::SwedishQ:
		qswedishradio_->set_active();
		break;
	case InsetQuotes::GermanQ:
		qgermanradio_->set_active();
		break;
	case InsetQuotes::PolishQ:
		qpolishradio_->set_active();
		break;
	case InsetQuotes::FrenchQ:
		qfrenchradio_->set_active();
		break;
	case InsetQuotes::DanishQ:
		qdanishradio_->set_active();
		break;
	}

	// *** End "Language" Page ***

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
	params.fontsize = fontsizecombo_.get_active_text();

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

	// *** Begin "Page" Page ***
	params.papersize2 = VMARGIN_PAPER_TYPE(
		pagesizecombo_.get_active_row_number());

	params.paperwidth = getLengthFromWidgets(
		*(pagewidthspin_->get_adjustment()), pagewidthunitscombo_);
	params.paperheight = getLengthFromWidgets(
		*(pageheightspin_->get_adjustment()), pageheightunitscombo_);

	if (portraitradio_->get_active())
		params.orientation = ORIENTATION_PORTRAIT;
	else
		params.orientation = ORIENTATION_LANDSCAPE;

	if (twocolumnstoggle_->get_active())
		params.columns = 2;
	else
		params.columns = 1;

	if (doublesidedtoggle_->get_active())
		params.sides = LyXTextClass::TwoSides;
	else
		params.sides = LyXTextClass::OneSide;

	params.pagestyle = pagestylecombo_.get_active_text();

	// *** End "Page" Page ***

	// *** Begin "Margins" Page ***
	int psize = pagesizecombo_.get_active_row_number();
	bool geom_papersize = (psize == 1 || psize == 5 || psize == 8 || psize == 9);
	params.use_geometry =
		(marginscombo_.get_active_row_number() == 1 || geom_papersize);

	int margin = marginscombo_.get_active_row_number();
	if (margin > 0) {
		margin = margin - 1;
	}
	params.paperpackage = PAPER_PACKAGES(margin);

	params.setPaperStuff();

	params.topmargin = getLengthFromWidgets(
		*(mtopspin_->get_adjustment()),
		mtopunitcombo_);
	params.bottommargin = getLengthFromWidgets(
		*(mbottomspin_->get_adjustment()),
		mbottomunitcombo_);
	params.leftmargin = getLengthFromWidgets(
		*(minnerspin_->get_adjustment()),
		minnerunitcombo_);
	params.rightmargin = getLengthFromWidgets(
		*(mouterspin_->get_adjustment()),
		mouterunitcombo_);
	params.headsep = getLengthFromWidgets(
		*(mheadsepspin_->get_adjustment()),
		mheadsepunitcombo_);
	params.headheight = getLengthFromWidgets(
		*(mheadheightspin_->get_adjustment()),
		mheadheightunitcombo_);
	params.footskip = getLengthFromWidgets(
		*(mfootskipspin_->get_adjustment()),
		mfootskipunitcombo_);
	// *** End "Margins" Page ***

	// *** Start "Language" Page ***
	int const encodingsel = encodingcombo_.get_active_row_number();
	if (encodingsel == 0)
		params.inputenc = "auto";
	else if (encodingsel == 1)
		params.inputenc = "default";
	else
		params.inputenc = encodings[encodingsel];

	int const langsel = languagecombo_.get_active_row_number();
	params.language = languages.getLanguage(lang_[langsel]);

	if (qenglishradio_->get_active())
		params.quotes_language = InsetQuotes::EnglishQ;
	else if (qswedishradio_->get_active())
		params.quotes_language = InsetQuotes::SwedishQ;
	else if (qgermanradio_->get_active())
		params.quotes_language = InsetQuotes::GermanQ;
	else if (qpolishradio_->get_active())
		params.quotes_language = InsetQuotes::PolishQ;
	else if (qfrenchradio_->get_active())
		params.quotes_language = InsetQuotes::FrenchQ;
	else if (qdanishradio_->get_active())
		params.quotes_language = InsetQuotes::DanishQ;
	// *** End "Language" Page ***

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


void GDocument::classChanged()
{
	ControlDocument & ctrl = controller();
	BufferParams & params = ctrl.params();

	lyx::textclass_type const tc = classcombo_.get_active_row_number();

	if (ctrl.loadTextclass(tc)) {
		params.textclass = tc;

		if (lyxrc.auto_reset_options) {
			params.useClassDefaults();
			update();
		} else {
			fontsizecombo_.clear();
			fontsizecombo_.append_text("default");

			string const fontsizeitems = ctrl.textClass().opt_fontsize();
			for (int n = 0; !token(fontsizeitems, '|', n).empty(); ++n)
				fontsizecombo_.append_text(token(fontsizeitems, '|', n));

			comboBoxTextSet(fontsizecombo_, params.fontsize);

			pagestylecombo_.clear();
			pagestylecombo_.append_text("default");

			string const pagestyleitems = ctrl.textClass().opt_pagestyle();
			std::cerr << "pagestyleitems=" << pagestyleitems << "\n";
			for (int n=0; !token(pagestyleitems,'|',n).empty(); ++n)
				pagestylecombo_.append_text(token(pagestyleitems,'|',n));

			comboBoxTextSet(pagestylecombo_, params.pagestyle);
			std::cerr << "params.pagestyle=" << params.pagestyle << "\n";
		}
	} else {
		classcombo_.set_active(params.textclass);
	}
}


void GDocument::pageSizeChanged()
{
	bool const customsize = pagesizecombo_.get_active_row_number() == 1;
	pagewidthspin_->set_sensitive(customsize);
	pageheightspin_->set_sensitive(customsize);
	pagewidthunitscombo_.set_sensitive(customsize);
	pageheightunitscombo_.set_sensitive(customsize);

	if (customsize)
		portraitradio_->set_active();
}


void GDocument::populateMargins()
{
	int olditem = marginscombo_.get_active_row_number();

	marginscombo_.clear();
	// Magic order
	marginscombo_.append_text(_("Default"));
	marginscombo_.append_text(_("Custom"));

	int papersize = pagesizecombo_.get_active_row_number();
	if (papersize < 0)
		papersize = 0;

	bool const a4size = (papersize == 6 || papersize == 0
			&& lyxrc.default_papersize == PAPER_A4PAPER);
	if (a4size && portraitradio_->get_active()) {
		marginscombo_.append_text(_("Small margins"));
		marginscombo_.append_text(_("Very small margins"));
		marginscombo_.append_text(_("Very wide margins"));
	} else if (olditem > 1) {
		olditem = 0;
	}
	marginscombo_.set_active(olditem);
}


void GDocument::marginsChanged()
{
	bool const custom =
		marginscombo_.get_active_row_number() == 1;

	mtopspin_->set_sensitive(custom);
	mbottomspin_->set_sensitive(custom);
	minnerspin_->set_sensitive(custom);
	mouterspin_->set_sensitive(custom);
	mheadsepspin_->set_sensitive(custom);
	mheadheightspin_->set_sensitive(custom);
	mfootskipspin_->set_sensitive(custom);
	mtopunitcombo_.set_sensitive(custom);
	mbottomunitcombo_.set_sensitive(custom);
	minnerunitcombo_.set_sensitive(custom);
	mouterunitcombo_.set_sensitive(custom);
	mheadsepunitcombo_.set_sensitive(custom);
	mheadheightunitcombo_.set_sensitive(custom);
	mfootskipunitcombo_.set_sensitive(custom);
}

} // namespace frontend
} // namespace lyx
