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
using support::contains;

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

	xml_->get_widget_derived("VerticalSpaceLength", vspacelengthentry_);
	vspacelengthentry_->set_relative(false);

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

	xml_->get_widget_derived("PageWidth", pagewidthlengthentry_);
	xml_->get_widget_derived("PageHeight", pageheightlengthentry_);
	pagewidthlengthentry_->set_relative(false);
	pageheightlengthentry_->set_relative(false);

	xml_->get_widget("Portrait", portraitradio_);
	xml_->get_widget("Landscape", landscaperadio_);

	xml_->get_widget("PageStyle", box);
	box->pack_start(pagestylecombo_, true, true, 0);
	box->show_all();
	xml_->get_widget("DoubleSided", doublesidedtoggle_);
	xml_->get_widget("TwoColumns", twocolumnstoggle_);
	// *** End "Page" Page ***

	// *** Begin "Margins" Page ***
	xml_->get_widget("DefaultMargins", defaultmargins_);
	defaultmargins_->signal_toggled().connect(
		sigc::mem_fun(*this, &GDocument::marginsChanged));

	xml_->get_widget_derived("MarginsTop", mtoplengthentry_);
	xml_->get_widget_derived("MarginsBottom", mbottomlengthentry_);
	xml_->get_widget_derived("MarginsInner", minnerlengthentry_);
	xml_->get_widget_derived("MarginsOuter", mouterlengthentry_);
	xml_->get_widget_derived("MarginsHeadSep", mheadseplengthentry_);
	xml_->get_widget_derived("MarginsHeadHeight", mheadheightlengthentry_);
	xml_->get_widget_derived("MarginsFootSkip", mfootskiplengthentry_);
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

	// *** Start "Numbering" Page ***
	Gtk::HScale * scale;
	xml_->get_widget("Numbering", scale);
	numberingadj_ = scale->get_adjustment();
	numberingadj_->signal_value_changed().connect(
		sigc::mem_fun(*this, &GDocument::numberingChanged));
	xml_->get_widget("NumberingLabel", numberinglabel_);

	xml_->get_widget("TOC", scale);
	TOCadj_ = scale->get_adjustment();
	TOCadj_->signal_value_changed().connect(
		sigc::mem_fun(*this, &GDocument::TOCChanged));
	xml_->get_widget("TOCLabel", TOClabel_);
	// *** End "Numbering" Page ***

	// *** Start "Bibliography" Page ***
	xml_->get_widget("BasicNumerical", basicnumericalradio_);
	xml_->get_widget("NatbibNumerical", natbibnumericalradio_);
	xml_->get_widget("NatbibAuthorYear", natbibauthoryearradio_);
	xml_->get_widget("Jurabib", jurabibradio_);
	xml_->get_widget("SectionedBibliography", sectionedbibliographytoggle_);
	// *** End "Bibliography" Page ***

	// *** Start "Math" Page ***
	xml_->get_widget("AMSAutomatically", AMSautomaticallyradio_);
	xml_->get_widget("AMSAlways", AMSalwaysradio_);
	xml_->get_widget("AMSNever", AMSneverradio_);
	// *** End "Math" Page ***

	// *** Start "Floats" Page ***
	xml_->get_widget("DocumentDefault", defaultradio_);
	xml_->get_widget("HereDefinitely", heredefinitelyradio_);
	xml_->get_widget("Alternative", alternativeradio_);
	alternativeradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GDocument::alternativeChanged));
	xml_->get_widget("TopOfPage", topcheck_);
	xml_->get_widget("BottomOfPage", bottomcheck_);
	xml_->get_widget("PageOfFloats", pageoffloatscheck_);
	xml_->get_widget("HereIfPossible", hereifpossiblecheck_);
	xml_->get_widget("IgnoreRules", ignorerulescheck_);
	// Initial sensitivity
	GDocument::alternativeChanged();

	// *** End "Floats" Page ***

	// *** Start "Bullets" Page ***
	// *** End "Bullets" Page ***

	// *** Start "Branches" Page ***
	xml_->get_widget("Branches", branchesview_);
	xml_->get_widget("AddBranch", addbranchbutton_);
	addbranchbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GDocument::addBranch));
	xml_->get_widget("RemoveBranch", removebranchbutton_);
	removebranchbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GDocument::removeBranch));
	branchCols_.add(branchColName_);
	branchCols_.add(branchColActivated_);
	branchCols_.add(branchColColor_);
	
	branchliststore_ = Gtk::ListStore::create(branchCols_);
	branchesview_->set_model(branchliststore_);
	branchesview_->append_column_editable(_("Name"), branchColName_);
	branchesview_->append_column_editable(_("Activated"), branchColActivated_);
	branchesview_->append_column_editable(_("Color"), branchColColor_);
	branchsel_ = branchesview_->get_selection();
	
	branchsel_->signal_changed().connect(
		sigc::mem_fun(*this, &GDocument::branchSelChanged));
	
	/*
	ErrorList::const_iterator cit = errors.begin();
	ErrorList::const_iterator end = errors.end();
	for (int rowindex = 0; cit != end; ++cit, ++rowindex) {
		Gtk::ListStore::Row row = *errliststore_->append();
		if (rowindex == 0)
			errlistsel_->select(*row);

		(*row)[listCol_] = cit->error;
		(*row)[listColIndex_] = rowindex;
	}
	*/
	Gtk::ListStore::Row row = *branchliststore_->append();
	(*row)[branchColName_] = "Munch";
	(*row)[branchColActivated_] = false;
	row = *branchliststore_->append();
	(*row)[branchColName_] = "Hunch";
	(*row)[branchColActivated_] = true;
	// *** End "Branches" Page ***

	// *** Begin "Preamble" Page ***
	Gtk::TextView *view;
	xml_->get_widget ("Preamble", view);
	preamblebuffer_ = view->get_buffer();
	// *** End "Preamble" Page ***
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
	vspacelengthentry_->set_length (vspacelen);

	// *** End "Document" Page ***

	// *** Begin "Page" Page ***
	int const psize = params.papersize;
	pagesizecombo_.set_active(psize);

	pagewidthlengthentry_->set_length(LyXLength(params.paperwidth));
	pageheightlengthentry_->set_length(LyXLength(params.paperheight));

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

	defaultmargins_->set_active(!params.use_geometry);

	mtoplengthentry_->set_length(params.topmargin);
	mbottomlengthentry_->set_length(params.bottommargin);
	minnerlengthentry_->set_length(params.leftmargin);
	mouterlengthentry_->set_length(params.rightmargin);
	mheadseplengthentry_->set_length(params.headsep);
	mheadheightlengthentry_->set_length(params.headheight);
	mfootskiplengthentry_->set_length(params.footskip);

	marginsChanged();
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

	// *** Start "Numbering" Page ***
	numberingadj_->set_value(params.secnumdepth + 2);
	TOCadj_->set_value(params.tocdepth + 1);
	// *** End "Numbering" Page ***

	// *** Start "Bibliography" Page ***
	switch (params.cite_engine) {
	case biblio::ENGINE_BASIC:
		basicnumericalradio_->set_active();
		break;
	case biblio::ENGINE_NATBIB_NUMERICAL:
		natbibnumericalradio_->set_active();
		break;
	case biblio::ENGINE_NATBIB_AUTHORYEAR:
		natbibauthoryearradio_->set_active();
		break;
	case biblio::ENGINE_JURABIB:
		jurabibradio_->set_active();
		break;
	}

	sectionedbibliographytoggle_->set_active(params.use_bibtopic);
	// *** End "Bibliography" Page ***

	// *** Start "Math" Page ***
	switch (params.use_amsmath) {
	case BufferParams::AMS_AUTO:
		AMSautomaticallyradio_->set_active();
		break;
	case BufferParams::AMS_ON:
		AMSalwaysradio_->set_active();
		break;
	case BufferParams::AMS_OFF:
		AMSneverradio_->set_active();
		break;
	}
	// *** End "Math" Page ***

	// *** Start "Floats" Page ***
	string const placement = params.float_placement;
	bool const here_definitely = contains(placement, 'H');
	bool const top    = contains(placement, 't');
	bool const bottom = contains(placement, 'b');
	bool const page   = contains(placement, 'p');
	bool const here   = contains(placement, 'h');
	bool const force  = contains(placement, '!');
	bool const alternatives = top || bottom || page || here;

	if (alternatives) {
		alternativeradio_->set_active(true);
	} else if (here_definitely) {
		heredefinitelyradio_->set_active(true);
	} else {
		defaultradio_->set_active(true);
	}
	ignorerulescheck_->set_active(force);
	topcheck_->set_active(top);
	bottomcheck_->set_active(bottom);
	pageoffloatscheck_->set_active(page);
	hereifpossiblecheck_->set_active(here);

	// *** End "Floats" Page ***

	// *** Start "Bullets" Page ***
	// *** End "Bullets" Page ***

	// *** Start "Branches" Page ***
	branchliststore_->clear();
	
	BranchList::const_iterator it = params.branchlist().begin();
	BranchList::const_iterator const end = params.branchlist().end();
	for (; it != end; ++it) {
		Gtk::ListStore::Row row = *branchliststore_->append();
		(*row)[branchColName_] = (*it).getBranch();
		std::cerr << "update: loading '" << (*it).getBranch() << "'\n";
		(*row)[branchColActivated_] = (*it).getSelected();
		(*row)[branchColColor_] = (*it).getColor();
	}
	// *** End "Branches" Page ***

	// *** Begin "Preamble" Page ***
	preamblebuffer_->set_text(params.preamble);
	// *** End "Preamble" Page ***

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
		LyXLength length = vspacelengthentry_->get_length();
		params.setDefSkip(VSpace(LyXGlueLength(length)));
	}

	// *** End "Document" Page ***

	// *** Begin "Page" Page ***
	params.papersize = PAPER_SIZE(
		pagesizecombo_.get_active_row_number());

	params.paperwidth = pagewidthlengthentry_->get_length_string();
	params.paperheight = pageheightlengthentry_->get_length_string();

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
	params.use_geometry = !defaultmargins_->get_active();

	params.topmargin = mtoplengthentry_->get_length_string();
	params.bottommargin = mbottomlengthentry_->get_length_string();
	params.leftmargin = minnerlengthentry_->get_length_string();
	params.rightmargin = mouterlengthentry_->get_length_string();
	params.headsep = mheadseplengthentry_->get_length_string();
	params.headheight = mheadheightlengthentry_->get_length_string();
	params.footskip = mfootskiplengthentry_->get_length_string();
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

	// *** Start "Numbering" Page ***
	params.secnumdepth = (int)(numberingadj_->get_value()) - 2;
	params.tocdepth = (int)(TOCadj_->get_value()) - 1;
	// *** End "Numbering" Page ***

	// *** Start "Bibliography" Page ***
	if (basicnumericalradio_->get_active())
		params.cite_engine = biblio::ENGINE_BASIC;
	else if (natbibnumericalradio_->get_active())
		params.cite_engine = biblio::ENGINE_NATBIB_NUMERICAL;
	else if (natbibauthoryearradio_->get_active())
		params.cite_engine = biblio::ENGINE_NATBIB_AUTHORYEAR;
	else if (jurabibradio_->get_active())
		params.cite_engine = biblio::ENGINE_JURABIB;

	params.use_bibtopic = sectionedbibliographytoggle_->get_active();
	// *** End "Bibliography" Page ***

	// *** Start "Math" Page ***
	if (AMSautomaticallyradio_->get_active())
		params.use_amsmath = BufferParams::AMS_AUTO;
	else if (AMSalwaysradio_->get_active())
		params.use_amsmath = BufferParams::AMS_ON;
	else if (AMSneverradio_->get_active())
		params.use_amsmath = BufferParams::AMS_OFF;
	// *** End "Math" Page ***

	// *** Start "Floats" Page ***
	string placement;
	if (alternativeradio_->get_active()) {
		if (ignorerulescheck_->get_active())
			placement += '!';
		if (topcheck_->get_active())
			placement += 't';
		if (bottomcheck_->get_active())
			placement += 'b';
		if (pageoffloatscheck_->get_active())
			placement += 'p';
		if (hereifpossiblecheck_->get_active())
			placement += 'h';
		if (placement == "!")
			placement.erase();
	} else if (heredefinitelyradio_->get_active())
		placement = "H";

	params.float_placement = placement;
	// *** End "Floats" Page ***

	// *** Start "Bullets" Page ***
	// *** End "Bullets" Page ***

	// *** Start "Branches" Page ***
	/*branchliststore_->clear();
	
	BranchList::const_iterator it = params.branchlist().begin();
	BranchList::const_iterator const end = params.branchlist().end();
	for (; it != end; ++it) {
		Gtk::ListStore::Row row = *branchliststore_->append();
		(*row)[branchColName_] = (*it).getBranch();
		(*row)[branchColActivated_] = (*it).getSelected();
		(*row)[branchColColor_] = (*it).getColor();
	}*/
	
	BranchList branchlist;
	
	Gtk::ListStore::iterator it = branchliststore_->children().begin();
	Gtk::ListStore::iterator const end = branchliststore_->children().end();
	for (; it != end; ++it) {
		Gtk::ListStore::Row row = *it;
		Glib::ustring const name = (*row)[branchColName_];
		if (branchlist.add(name)) {
			std::cerr << "apply: adding '" << name << "'\n";
			Branch * newbranch = branchlist.find(name);
			newbranch->setSelected((*row)[branchColActivated_]);
			Glib::ustring const color = (*row)[branchColColor_];
			newbranch->setColor(color);
		}
	}
	
	params.branchlist() = branchlist;
	
	// *** End "Branches" Page ***

	// *** Begin "Preamble" Page ***
	params.preamble = preamblebuffer_->get_text();
	// *** End "Preamble" Page ***
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

	vspacelengthentry_->set_sensitive(lengthsensitive);
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
			for (int n=0; !token(pagestyleitems,'|',n).empty(); ++n)
				pagestylecombo_.append_text(token(pagestyleitems,'|',n));

			comboBoxTextSet(pagestylecombo_, params.pagestyle);
		}
	} else {
		classcombo_.set_active(params.textclass);
	}
}


void GDocument::pageSizeChanged()
{
	bool const customsize = pagesizecombo_.get_active_row_number() == 1;
	pagewidthlengthentry_->set_sensitive(customsize);
	pageheightlengthentry_->set_sensitive(customsize);

	if (customsize)
		portraitradio_->set_active();
}


void GDocument::marginsChanged()
{
	bool const custom = !defaultmargins_->get_active();

	mtoplengthentry_->set_sensitive(custom);
	mbottomlengthentry_->set_sensitive(custom);
	minnerlengthentry_->set_sensitive(custom);
	mouterlengthentry_->set_sensitive(custom);
	mheadseplengthentry_->set_sensitive(custom);
	mheadheightlengthentry_->set_sensitive(custom);
	mfootskiplengthentry_->set_sensitive(custom);
}


void GDocument::numberingChanged()
{
	string const numberinglabels[] = {
		_("No headings numbered"),
		_("Only parts numbered"),
		_("Chapters and above numbered"),
		_("Sections and above numbered"),
		_("Subsections and above numbered"),
		_("Subsubsections and above numbered"),
		_("Paragraphs and above numbered"),
		_("All headings numbered")
	};

	int const value = (int)(numberingadj_->get_value());
	numberinglabel_->set_label("<i>" + numberinglabels[value] + "</i>");
}


void GDocument::TOCChanged()
{
	string const TOClabels[] = {
		_("Only Parts appear in TOC"),
		_("Chapters and above appear in TOC"),
		_("Sections and above appear in TOC"),
		_("Subsections and above appear in TOC"),
		_("Subsubsections and above appear in TOC"),
		_("Paragraphs and above appear in TOC"),
		_("TOC contains all headings")
	};

	int const value = (int)(TOCadj_->get_value());
	TOClabel_->set_label("<i>" + TOClabels[value] + "</i>");
}


void GDocument::alternativeChanged()
{
	bool const sens = alternativeradio_->get_active();
	topcheck_->set_sensitive(sens);
	bottomcheck_->set_sensitive(sens);
	pageoffloatscheck_->set_sensitive(sens);
	hereifpossiblecheck_->set_sensitive(sens);
	ignorerulescheck_->set_sensitive(sens);
}


void GDocument::addBranch()
{
	Gtk::ListStore::Row row = *branchliststore_->append();
	(*row)[branchColName_] = "New Branch";
	(*row)[branchColActivated_] = false;
	(*row)[branchColColor_] = "#000000";
}


void GDocument::removeBranch()
{
	Gtk::TreeModel::iterator const selected = branchsel_->get_selected();
	if (!branchliststore_->iter_is_valid(selected))
		return;

	branchliststore_->erase (selected);
}


void GDocument::branchSelChanged()
{
	if (branchsel_->get_selected() == branchliststore_->children().end())
		removebranchbutton_->set_sensitive(false);
	else
		removebranchbutton_->set_sensitive(true);
}

} // namespace frontend
} // namespace lyx
