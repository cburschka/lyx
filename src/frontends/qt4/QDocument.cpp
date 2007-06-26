/**
 * \file QDocument.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QDocument.h"

#include "CheckedLineEdit.h"
#include "FloatPlacement.h"
#include "LengthCombo.h"
#include "PanelStack.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "Validator.h"

// For the Branches module
#include "QBranches.h"

#include "QViewSource.h" // For latexHighlighter use in the preamble.

#include "controllers/ControlDocument.h"

#include "BufferParams.h"
#include "Encoding.h"
#include "gettext.h"
#include "frontend_helpers.h" // getSecond()
#include "Language.h"
#include "LyXRC.h" // defaultUnit
#include "TextClassList.h"
#include "Spacing.h"

#include "insets/InsetListingsParams.h"
#include "controllers/ControlDocument.h"

#include "support/lstrings.h"

#include <QCloseEvent>
#include <QScrollBar>
#include <QTextCursor>

#include <map>

using lyx::support::token;
using lyx::support::bformat;
using lyx::support::findToken;
using lyx::support::getVectorFromString;

using std::distance;
using std::make_pair;
using std::pair;
using std::vector;
using std::string;


char const * const tex_graphics[] = {"default", "dvips", "dvitops", "emtex",
		      "ln", "oztex", "textures", "none", ""
};


char const * const tex_graphics_gui[] = {N_("Default"), "Dvips", "DVItoPS", "EmTeX",
		      "LN", "OzTeX", "Textures", N_("None"), ""
};


char const * const tex_fonts_roman[] = {"default", "cmr", "lmodern", "ae", "times", "palatino",
			    "charter", "newcent", "bookman", "utopia", "beraserif", "ccfonts", "chancery", ""
};


char const * tex_fonts_roman_gui[] = { N_("Default"), N_("Computer Modern Roman"), N_("Latin Modern Roman"),
			    N_("AE (Almost European)"), N_("Times Roman"), N_("Palatino"), N_("Bitstream Charter"),
			    N_("New Century Schoolbook"), N_("Bookman"), N_("Utopia"),  N_("Bera Serif"),
			    N_("Concrete Roman"), N_("Zapf Chancery"), ""
};


char const * const tex_fonts_sans[] = {"default", "cmss", "lmss", "helvet", "avant", "berasans", "cmbr", ""
};


char const * tex_fonts_sans_gui[] = { N_("Default"), N_("Computer Modern Sans"), N_("Latin Modern Sans"),
			    N_("Helvetica"), N_("Avant Garde"), N_("Bera Sans"), N_("CM Bright"), ""
};


char const * const tex_fonts_monospaced[] = {"default", "cmtt", "lmtt", "courier", "beramono",
			    "luximono", "cmtl", ""
};


char const * tex_fonts_monospaced_gui[] = { N_("Default"), N_("Computer Modern Typewriter"),
			    N_("Latin Modern Typewriter"), N_("Courier"), N_("Bera Mono"), N_("LuxiMono"),
			    N_("CM Typewriter Light"), ""
};


vector<pair<string, lyx::docstring> > pagestyles;


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// PreambleModule
//
/////////////////////////////////////////////////////////////////////

PreambleModule::PreambleModule(): current_id_(0)
{
	// This is not a memory leak. The object will be destroyed
	// with this.
	(void) new LaTeXHighlighter(preambleTE->document());
	setFocusProxy(preambleTE);
	connect(preambleTE, SIGNAL(textChanged()), this, SIGNAL(changed()));
}


void PreambleModule::update(BufferParams const & params, BufferId id)
{
	QString preamble = toqstr(params.preamble);
	// Nothing to do if the params and preamble are unchanged.
	if (id == current_id_
		&& preamble == preambleTE->document()->toPlainText())
		return;

	QTextCursor cur = preambleTE->textCursor();
	// Save the coords before switching to the new one.
	preamble_coords_[current_id_] =
		make_pair(cur.position(), preambleTE->verticalScrollBar()->value());

	// Save the params address for further use.
	current_id_ = id;
	preambleTE->document()->setPlainText(preamble);
	Coords::const_iterator it = preamble_coords_.find(current_id_);
	if (it == preamble_coords_.end())
		// First time we open this one.
		preamble_coords_[current_id_] = make_pair(0,0);
	else {
		// Restore saved coords.
		QTextCursor cur = preambleTE->textCursor();
		cur.setPosition(it->second.first);
		preambleTE->setTextCursor(cur);
		preambleTE->verticalScrollBar()->setValue(it->second.second);
	}
}


void PreambleModule::apply(BufferParams & params)
{
	params.preamble = fromqstr(preambleTE->document()->toPlainText());
}


void PreambleModule::closeEvent(QCloseEvent * e)
{
	// Save the coords before closing.
	QTextCursor cur = preambleTE->textCursor();
	preamble_coords_[current_id_] =
		make_pair(cur.position(), preambleTE->verticalScrollBar()->value());
	e->accept();
}


/////////////////////////////////////////////////////////////////////
//
// DocumentDialog
//
/////////////////////////////////////////////////////////////////////

QDocumentDialog::QDocumentDialog(QDocument * form)
	: form_(form),
	lang_(getSecond(getLanguageData(false)))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), form, SLOT(slotRestore()));

	connect(savePB, SIGNAL(clicked()), this, SLOT(saveDefaultClicked()));
	connect(defaultPB, SIGNAL(clicked()), this, SLOT(useDefaultsClicked()));

	// Manage the restore, ok, apply, restore and cancel/close buttons
	form_->bcview().setOK(okPB);
	form_->bcview().setApply(applyPB);
	form_->bcview().setCancel(closePB);
	form_->bcview().setRestore(restorePB);


	textLayoutModule = new UiWidget<Ui::TextLayoutUi>;
	// text layout
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)),
		this, SLOT(setLSpacing(int)));
	connect(textLayoutModule->lspacingLE, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->indentRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLengthCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)),
		this, SLOT(setSkip(int)));
	connect(textLayoutModule->skipRB, SIGNAL(toggled(bool)),
		this, SLOT(enableSkip(bool)));
	connect(textLayoutModule->twoColumnCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->bypassCB, SIGNAL(clicked()), 
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->bypassCB, SIGNAL(clicked()), 
		this, SLOT(validate_listings_params()));
	connect(textLayoutModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(validate_listings_params()));
	textLayoutModule->listingsTB->setPlainText(
		qt_("Input listings parameters on the right. Enter ? for a list of parameters."));
	textLayoutModule->lspacingLE->setValidator(new QDoubleValidator(
		textLayoutModule->lspacingLE));
	textLayoutModule->skipLE->setValidator(unsignedLengthValidator(
		textLayoutModule->skipLE));

	textLayoutModule->skipCO->addItem(qt_("SmallSkip"));
	textLayoutModule->skipCO->addItem(qt_("MedSkip"));
	textLayoutModule->skipCO->addItem(qt_("BigSkip"));
	textLayoutModule->skipCO->addItem(qt_("Length"));
	// remove the %-items from the unit choice
	textLayoutModule->skipLengthCO->noPercents();
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Single, qt_("Single"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Onehalf, qt_("OneHalf"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Double, qt_("Double"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Other, qt_("Custom"));

	// initialize the length validator
	addCheckedLineEdit(form_->bcview(), textLayoutModule->skipLE);

	fontModule = new UiWidget<Ui::FontUi>;
	// fonts
	connect(fontModule->fontsRomanCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsRomanCO, SIGNAL(activated(int)),
		this, SLOT(romanChanged(int)));
	connect(fontModule->fontsSansCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsSansCO, SIGNAL(activated(int)),
		this, SLOT(sansChanged(int)));
	connect(fontModule->fontsTypewriterCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsTypewriterCO, SIGNAL(activated(int)),
		this, SLOT(ttChanged(int)));
	connect(fontModule->fontsDefaultCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsizeCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->scaleSansSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->scaleTypewriterSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontScCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontOsfCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	for (int n = 0; tex_fonts_roman[n][0]; ++n) {
		QString font = qt_(tex_fonts_roman_gui[n]);
		if (!form_->controller().isFontAvailable(tex_fonts_roman[n]))
			font += qt_(" (not installed)");
		fontModule->fontsRomanCO->addItem(font);
	}
	for (int n = 0; tex_fonts_sans[n][0]; ++n) {
		QString font = qt_(tex_fonts_sans_gui[n]);
		if (!form_->controller().isFontAvailable(tex_fonts_sans[n]))
			font += qt_(" (not installed)");
		fontModule->fontsSansCO->addItem(font);
	}
	for (int n = 0; tex_fonts_monospaced[n][0]; ++n) {
		QString font = qt_(tex_fonts_monospaced_gui[n]);
		if (!form_->controller().isFontAvailable(tex_fonts_monospaced[n]))
			font += qt_(" (not installed)");
		fontModule->fontsTypewriterCO->addItem(font);
	}

	fontModule->fontsizeCO->addItem(qt_("Default"));
	fontModule->fontsizeCO->addItem(qt_("10"));
	fontModule->fontsizeCO->addItem(qt_("11"));
	fontModule->fontsizeCO->addItem(qt_("12"));

	for (int n = 0; ControlDocument::fontfamilies_gui[n][0]; ++n)
		fontModule->fontsDefaultCO->addItem(
			qt_(ControlDocument::fontfamilies_gui[n]));


	pageLayoutModule = new UiWidget<Ui::PageLayoutUi>;
	// page layout
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)),
		this, SLOT(setCustomPapersize(int)));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)),
		this, SLOT(setCustomPapersize(int)));
	connect(pageLayoutModule->portraitRB, SIGNAL(clicked()),
		this, SLOT(portraitChanged()));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperheightLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperwidthLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperwidthUnitCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperheightUnitCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->portraitRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->landscapeRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->facingPagesCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->pagestyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));

	pageLayoutModule->pagestyleCO->addItem(qt_("Default"));
	pageLayoutModule->pagestyleCO->addItem(qt_("empty"));
	pageLayoutModule->pagestyleCO->addItem(qt_("plain"));
	pageLayoutModule->pagestyleCO->addItem(qt_("headings"));
	pageLayoutModule->pagestyleCO->addItem(qt_("fancy"));
	addCheckedLineEdit(form_->bcview(), pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightL);
	addCheckedLineEdit(form_->bcview(), pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthL);

	// paper
	QComboBox * cb = pageLayoutModule->papersizeCO;
	cb->addItem(qt_("Default"));
	cb->addItem(qt_("Custom"));
	cb->addItem(qt_("US letter"));
	cb->addItem(qt_("US legal"));
	cb->addItem(qt_("US executive"));
	cb->addItem(qt_("A3"));
	cb->addItem(qt_("A4"));
	cb->addItem(qt_("A5"));
	cb->addItem(qt_("B3"));
	cb->addItem(qt_("B4"));
	cb->addItem(qt_("B5"));
	// remove the %-items from the unit choice
	pageLayoutModule->paperwidthUnitCO->noPercents();
	pageLayoutModule->paperheightUnitCO->noPercents();
	pageLayoutModule->paperheightLE->setValidator(unsignedLengthValidator(
		pageLayoutModule->paperheightLE));
	pageLayoutModule->paperwidthLE->setValidator(unsignedLengthValidator(
		pageLayoutModule->paperwidthLE));




	marginsModule = new UiWidget<Ui::MarginsUi>;
	// margins
	connect(marginsModule->marginCB, SIGNAL(toggled(bool)),
		this, SLOT(setCustomMargins(bool)));
	connect(marginsModule->marginCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(marginsModule->topLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->topUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->bottomLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->bottomUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->innerLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->innerUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->outerLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->outerUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headheightLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headheightUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headsepLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headsepUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->footskipLE, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->footskipUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	marginsModule->topLE->setValidator(unsignedLengthValidator(
		marginsModule->topLE));
	marginsModule->bottomLE->setValidator(unsignedLengthValidator(
		marginsModule->bottomLE));
	marginsModule->innerLE->setValidator(unsignedLengthValidator(
		marginsModule->innerLE));
	marginsModule->outerLE->setValidator(unsignedLengthValidator(
		marginsModule->outerLE));
	marginsModule->headsepLE->setValidator(unsignedLengthValidator(
		marginsModule->headsepLE));
	marginsModule->headheightLE->setValidator(unsignedLengthValidator(
		marginsModule->headheightLE));
	marginsModule->footskipLE->setValidator(unsignedLengthValidator(
		marginsModule->footskipLE));

	addCheckedLineEdit(form_->bcview(), marginsModule->topLE,
		marginsModule->topL);
	addCheckedLineEdit(form_->bcview(), marginsModule->bottomLE,
		marginsModule->bottomL);
	addCheckedLineEdit(form_->bcview(), marginsModule->innerLE,
		marginsModule->innerL);
	addCheckedLineEdit(form_->bcview(), marginsModule->outerLE,
		marginsModule->outerL);
	addCheckedLineEdit(form_->bcview(), marginsModule->headsepLE,
		marginsModule->headsepL);
	addCheckedLineEdit(form_->bcview(), marginsModule->headheightLE,
		marginsModule->headheightL);
	addCheckedLineEdit(form_->bcview(), marginsModule->footskipLE,
		marginsModule->footskipL);


	langModule = new UiWidget<Ui::LanguageUi>;
	connect(langModule->defaultencodingCB, SIGNAL(toggled(bool)),
		langModule->encodingL, SLOT(setDisabled(bool)));
	connect(langModule->defaultencodingCB, SIGNAL(toggled(bool)),
		langModule->encodingCO, SLOT(setDisabled(bool)));
	// language & quote
	connect(langModule->languageCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->defaultencodingCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(langModule->encodingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->quoteStyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	// language & quotes
	vector<LanguagePair> const langs = getLanguageData(false);
	vector<LanguagePair>::const_iterator lit  = langs.begin();
	vector<LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		langModule->languageCO->addItem(
			toqstr(lit->first));
	}

	// Always put the default encoding in the first position.
	// It is special because the displayed text is translated.
	langModule->encodingCO->addItem(qt_("LaTeX default"));
	Encodings::const_iterator it = encodings.begin();
	Encodings::const_iterator const end = encodings.end();
	for (; it != end; ++it)
		langModule->encodingCO->addItem(toqstr(it->latexName()));

	langModule->quoteStyleCO->addItem(qt_("``text''"));
	langModule->quoteStyleCO->addItem(qt_("''text''"));
	langModule->quoteStyleCO->addItem(qt_(",,text``"));
	langModule->quoteStyleCO->addItem(qt_(",,text''"));
	langModule->quoteStyleCO->addItem(qt_("<<text>>"));
	langModule->quoteStyleCO->addItem(qt_(">>text<<"));



	numberingModule = new UiWidget<Ui::NumberingUi>;
	// numbering
	connect(numberingModule->depthSL, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(numberingModule->tocSL, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(numberingModule->depthSL, SIGNAL(valueChanged(int)),
		this, SLOT(updateNumbering()));
	connect(numberingModule->tocSL, SIGNAL(valueChanged(int)),
		this, SLOT(updateNumbering()));
	numberingModule->tocTW->setColumnCount(3);
	numberingModule->tocTW->headerItem()->setText(0, qt_("Example"));
	numberingModule->tocTW->headerItem()->setText(1, qt_("Numbered"));
	numberingModule->tocTW->headerItem()->setText(2, qt_("Appears in TOC"));


	biblioModule = new UiWidget<Ui::BiblioUi>;
	connect(biblioModule->citeNatbibRB, SIGNAL(toggled(bool)),
		biblioModule->citationStyleL, SLOT(setEnabled(bool)));
	connect(biblioModule->citeNatbibRB, SIGNAL(toggled(bool)),
		biblioModule->citeStyleCO, SLOT(setEnabled(bool)));
	// biblio
	connect(biblioModule->citeDefaultRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(biblioModule->citeNatbibRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(biblioModule->citeStyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(biblioModule->citeJurabibRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(biblioModule->bibtopicCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	// biblio
	biblioModule->citeStyleCO->addItem(qt_("Author-year"));
	biblioModule->citeStyleCO->addItem(qt_("Numerical"));
	biblioModule->citeStyleCO->setCurrentIndex(0);



	mathsModule = new UiWidget<Ui::MathsUi>;
	connect(mathsModule->amsautoCB, SIGNAL(toggled(bool)),
		mathsModule->amsCB, SLOT(setDisabled(bool)));
	connect(mathsModule->esintautoCB, SIGNAL(toggled(bool)),
		mathsModule->esintCB, SLOT(setDisabled(bool)));
	// maths
	connect(mathsModule->amsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(mathsModule->amsautoCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(mathsModule->esintCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(mathsModule->esintautoCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	latexModule = new UiWidget<Ui::LaTeXUi>;
	// latex class
	connect(latexModule->classCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(latexModule->optionsLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(latexModule->psdriverCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(latexModule->classCO, SIGNAL(activated(int)),
		this, SLOT(classChanged()));
	// packages
	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = qt_(tex_graphics_gui[n]);
		latexModule->psdriverCO->addItem(enc);
	}
	// latex
	for (TextClassList::const_iterator cit = textclasslist.begin();
	     cit != textclasslist.end(); ++cit) {
		if (cit->isTeXClassAvailable()) {
			latexModule->classCO->addItem(toqstr(cit->description()));
		} else {
			docstring item =
				bformat(_("Unavailable: %1$s"), from_utf8(cit->description()));
			latexModule->classCO->addItem(toqstr(item));
		}
	}

	// branches
	branchesModule = new QBranches;
	connect(branchesModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	// preamble
	preambleModule = new PreambleModule;
	connect(preambleModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	// bullets
	bulletsModule = new BulletsModule;
	connect(bulletsModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));


	// float
	floatModule = new FloatPlacement;
	connect(floatModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	docPS->addPanel(latexModule, _("Document Class"));
	docPS->addPanel(fontModule, _("Fonts"));
	docPS->addPanel(textLayoutModule, _("Text Layout"));
	docPS->addPanel(pageLayoutModule, _("Page Layout"));
	docPS->addPanel(marginsModule, _("Page Margins"));
	docPS->addPanel(langModule, _("Language"));
	docPS->addPanel(numberingModule, _("Numbering & TOC"));
	docPS->addPanel(biblioModule, _("Bibliography"));
	docPS->addPanel(mathsModule, _("Math Options"));
	docPS->addPanel(floatModule, _("Float Placement"));
	docPS->addPanel(bulletsModule, _("Bullets"));
	docPS->addPanel(branchesModule, _("Branches"));
	docPS->addPanel(preambleModule, _("LaTeX Preamble"));
	docPS->setCurrentPanel(_("Document Class"));
// FIXME: hack to work around resizing bug in Qt >= 4.2
// bug verified with Qt 4.2.{0-3} (JSpitzm)
#if QT_VERSION >= 0x040200
	docPS->updateGeometry();
#endif
}


void QDocumentDialog::showPreamble()
{
	docPS->setCurrentPanel(_("LaTeX Preamble"));
}


void QDocumentDialog::saveDefaultClicked()
{
	form_->saveDocDefault();
}


void QDocumentDialog::useDefaultsClicked()
{
	form_->useClassDefaults();
}


void QDocumentDialog::change_adaptor()
{
	form_->changed();
}


void QDocumentDialog::validate_listings_params()
{
	static bool isOK = true;
	InsetListingsParams par(fromqstr(textLayoutModule->listingsED->toPlainText()));
	docstring msg;
	if (!textLayoutModule->bypassCB->isChecked())
		msg = par.validate();
	if (msg.empty()) {
		if (isOK)
			return;
		isOK = true;
		// listingsTB->setTextColor("black");
		textLayoutModule->listingsTB->setPlainText(
			qt_("Input listings parameters on the right. Enter ? for a list of parameters."));
		okPB->setEnabled(true);
		applyPB->setEnabled(true);
	} else {
		isOK = false;
		// listingsTB->setTextColor("red");
		textLayoutModule->listingsTB->setPlainText(toqstr(msg));
		okPB->setEnabled(false);
		applyPB->setEnabled(false);
	}
}


void QDocumentDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QDocumentDialog::setLSpacing(int item)
{
	textLayoutModule->lspacingLE->setEnabled(item == 3);
}


void QDocumentDialog::setSkip(int item)
{
	bool const enable = (item == 3);
	textLayoutModule->skipLE->setEnabled(enable);
	textLayoutModule->skipLengthCO->setEnabled(enable);
}


void QDocumentDialog::enableSkip(bool skip)
{
	textLayoutModule->skipCO->setEnabled(skip);
	textLayoutModule->skipLE->setEnabled(skip);
	textLayoutModule->skipLengthCO->setEnabled(skip);
	if (skip)
		setSkip(textLayoutModule->skipCO->currentIndex());
}

void QDocumentDialog::portraitChanged()
{
	setMargins(pageLayoutModule->papersizeCO->currentIndex());
}

void QDocumentDialog::setMargins(bool custom)
{
	marginsModule->marginCB->setChecked(custom);
	setCustomMargins(custom);
}


void QDocumentDialog::setCustomPapersize(int papersize)
{
	bool const custom = (papersize == 1);

	pageLayoutModule->paperwidthL->setEnabled(custom);
	pageLayoutModule->paperwidthLE->setEnabled(custom);
	pageLayoutModule->paperwidthUnitCO->setEnabled(custom);
	pageLayoutModule->paperheightL->setEnabled(custom);
	pageLayoutModule->paperheightLE->setEnabled(custom);
	pageLayoutModule->paperheightLE->setFocus();
	pageLayoutModule->paperheightUnitCO->setEnabled(custom);
}


void QDocumentDialog::setCustomMargins(bool custom)
{
	marginsModule->topL->setEnabled(!custom);
	marginsModule->topLE->setEnabled(!custom);
	marginsModule->topUnit->setEnabled(!custom);

	marginsModule->bottomL->setEnabled(!custom);
	marginsModule->bottomLE->setEnabled(!custom);
	marginsModule->bottomUnit->setEnabled(!custom);

	marginsModule->innerL->setEnabled(!custom);
	marginsModule->innerLE->setEnabled(!custom);
	marginsModule->innerUnit->setEnabled(!custom);

	marginsModule->outerL->setEnabled(!custom);
	marginsModule->outerLE->setEnabled(!custom);
	marginsModule->outerUnit->setEnabled(!custom);

	marginsModule->headheightL->setEnabled(!custom);
	marginsModule->headheightLE->setEnabled(!custom);
	marginsModule->headheightUnit->setEnabled(!custom);

	marginsModule->headsepL->setEnabled(!custom);
	marginsModule->headsepLE->setEnabled(!custom);
	marginsModule->headsepUnit->setEnabled(!custom);

	marginsModule->footskipL->setEnabled(!custom);
	marginsModule->footskipLE->setEnabled(!custom);
	marginsModule->footskipUnit->setEnabled(!custom);
}


void QDocumentDialog::updateFontsize(string const & items, string const & sel)
{
	fontModule->fontsizeCO->clear();
	fontModule->fontsizeCO->addItem(qt_("Default"));

	for (int n = 0; !token(items,'|',n).empty(); ++n)
		fontModule->fontsizeCO->
			addItem(toqstr(token(items,'|',n)));

	for (int n = 0; n < fontModule->fontsizeCO->count(); ++n) {
		if (fromqstr(fontModule->fontsizeCO->itemText(n)) == sel) {
			fontModule->fontsizeCO->setCurrentIndex(n);
			break;
		}
	}
}


void QDocumentDialog::romanChanged(int item)
{
	string const font = tex_fonts_roman[item];

	fontModule->fontScCB->setEnabled(
		form_->controller().providesSC(font));
	fontModule->fontOsfCB->setEnabled(
		form_->controller().providesOSF(font));
}


void QDocumentDialog::sansChanged(int item)
{
	string const font = tex_fonts_sans[item];
	bool scaleable = form_->controller().providesScale(font);
	fontModule->scaleSansSB->setEnabled(scaleable);
	fontModule->scaleSansLA->setEnabled(scaleable);
}


void QDocumentDialog::ttChanged(int item)
{
	string const font = tex_fonts_monospaced[item];
	bool scaleable = form_->controller().providesScale(font);
	fontModule->scaleTypewriterSB->setEnabled(scaleable);
	fontModule->scaleTypewriterLA->setEnabled(scaleable);
}


void QDocumentDialog::updatePagestyle(string const & items, string const & sel)
{
	pagestyles.clear();
	pageLayoutModule->pagestyleCO->clear();
	pageLayoutModule->pagestyleCO->addItem(qt_("Default"));

	for (int n = 0; !token(items,'|',n).empty(); ++n) {
		string style = token(items, '|', n);
		docstring style_gui = _(style);
		pagestyles.push_back(pair<string, docstring>(style, style_gui));
		pageLayoutModule->pagestyleCO->addItem(toqstr(style_gui));
	}

	if (sel == "default") {
		pageLayoutModule->pagestyleCO->setCurrentIndex(0);
		return;
	}

	int n = 0;

	for (size_t i = 0; i < pagestyles.size(); ++i)
		if (pagestyles[i].first == sel)
			n = pageLayoutModule->pagestyleCO->findText(
					toqstr(pagestyles[i].second));

	if (n > 0)
		pageLayoutModule->pagestyleCO->setCurrentIndex(n);
}


void QDocumentDialog::classChanged()
{
	ControlDocument & cntrl = form_->controller();
	BufferParams & params = cntrl.params();

	textclass_type const tc = latexModule->classCO->currentIndex();

	if (form_->controller().loadTextclass(tc)) {
		params.textclass = tc;
		if (lyxrc.auto_reset_options)
			params.useClassDefaults();
		form_->update_contents();
	} else {
		latexModule->classCO->setCurrentIndex(params.textclass);
	}
}


void QDocumentDialog::updateNumbering()
{
	TextClass const & tclass =
		form_->controller().params().getTextClass();

	numberingModule->tocTW->setUpdatesEnabled(false);
	numberingModule->tocTW->clear();

	int const depth = numberingModule->depthSL->value();
	int const toc = numberingModule->tocSL->value();
	QString const no = qt_("No");
	QString const yes = qt_("Yes");
	TextClass::const_iterator end = tclass.end();
	TextClass::const_iterator cit = tclass.begin();
	QTreeWidgetItem * item = 0;
	for ( ; cit != end ; ++cit) {
		int const toclevel = (*cit)->toclevel;
		if (toclevel != Layout::NOT_IN_TOC
		    && (*cit)->labeltype == LABEL_COUNTER) {
			item = new QTreeWidgetItem(numberingModule->tocTW);
			item->setText(0, qt_((*cit)->name()));
			item->setText(1, (toclevel <= depth) ? yes : no);
			item->setText(2, (toclevel <= toc) ? yes : no);
		}
	}

	numberingModule->tocTW->setUpdatesEnabled(true);
	numberingModule->tocTW->update();
}

void QDocumentDialog::apply(BufferParams & params)
{
	// preamble
	preambleModule->apply(params);

	// biblio
	params.setCiteEngine(biblio::ENGINE_BASIC);

	if (biblioModule->citeNatbibRB->isChecked()) {
		bool const use_numerical_citations =
			biblioModule->citeStyleCO->currentIndex();
		if (use_numerical_citations)
			params.setCiteEngine(biblio::ENGINE_NATBIB_NUMERICAL);
		else
			params.setCiteEngine(biblio::ENGINE_NATBIB_AUTHORYEAR);

	} else if (biblioModule->citeJurabibRB->isChecked())
		params.setCiteEngine(biblio::ENGINE_JURABIB);

	params.use_bibtopic =
		biblioModule->bibtopicCB->isChecked();

	// language & quotes
	if (langModule->defaultencodingCB->isChecked()) {
		params.inputenc = "auto";
	} else {
		int i = langModule->encodingCO->currentIndex();
		if (i == 0)
			params.inputenc = "default";
		else
			params.inputenc =
				fromqstr(langModule->encodingCO->currentText());
	}

	InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;
	switch (langModule->quoteStyleCO->currentIndex()) {
	case 0:
		lga = InsetQuotes::EnglishQ;
		break;
	case 1:
		lga = InsetQuotes::SwedishQ;
		break;
	case 2:
		lga = InsetQuotes::GermanQ;
		break;
	case 3:
		lga = InsetQuotes::PolishQ;
		break;
	case 4:
		lga = InsetQuotes::FrenchQ;
		break;
	case 5:
		lga = InsetQuotes::DanishQ;
		break;
	}
	params.quotes_language = lga;

	int const pos = langModule->languageCO->currentIndex();
	params.language = lyx::languages.getLanguage(lang_[pos]);

	// numbering
	if (params.getTextClass().hasTocLevels()) {
		params.tocdepth = numberingModule->tocSL->value();
		params.secnumdepth = numberingModule->depthSL->value();
	}

	// bullets
	params.user_defined_bullet(0) = bulletsModule->getBullet(0);
	params.user_defined_bullet(1) = bulletsModule->getBullet(1);
	params.user_defined_bullet(2) = bulletsModule->getBullet(2);
	params.user_defined_bullet(3) = bulletsModule->getBullet(3);

	// packages
	params.graphicsDriver =
		tex_graphics[latexModule->psdriverCO->currentIndex()];

	if (mathsModule->amsautoCB->isChecked()) {
		params.use_amsmath = BufferParams::package_auto;
	} else {
		if (mathsModule->amsCB->isChecked())
			params.use_amsmath = BufferParams::package_on;
		else
			params.use_amsmath = BufferParams::package_off;
	}

	if (mathsModule->esintautoCB->isChecked())
		params.use_esint = BufferParams::package_auto;
	else {
		if (mathsModule->esintCB->isChecked())
			params.use_esint = BufferParams::package_on;
		else
			params.use_esint = BufferParams::package_off;
	}

	// text layout
	params.textclass =
		latexModule->classCO->currentIndex();

	if (pageLayoutModule->pagestyleCO->currentIndex() == 0)
		params.pagestyle = "default";
	else {
		docstring style_gui =
			qstring_to_ucs4(pageLayoutModule->pagestyleCO->currentText());
		for (size_t i = 0; i < pagestyles.size(); ++i)
			if (pagestyles[i].second == style_gui)
				params.pagestyle = pagestyles[i].first;
	}

	switch (textLayoutModule->lspacingCO->currentIndex()) {
	case 0:
		params.spacing().set(Spacing::Single);
		break;
	case 1:
		params.spacing().set(Spacing::Onehalf);
		break;
	case 2:
		params.spacing().set(Spacing::Double);
		break;
	case 3:
		params.spacing().set(Spacing::Other,
			fromqstr(textLayoutModule->lspacingLE->text()));
		break;
	}

	if (textLayoutModule->twoColumnCB->isChecked())
		params.columns = 2;
	else
		params.columns = 1;

	// text should have passed validation
	params.listings_params =
		InsetListingsParams(fromqstr(textLayoutModule->listingsED->toPlainText())).params();

	if (textLayoutModule->indentRB->isChecked())
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params.paragraph_separation = BufferParams::PARSEP_SKIP;

	switch (textLayoutModule->skipCO->currentIndex()) {
	case 0:
		params.setDefSkip(VSpace(VSpace::SMALLSKIP));
		break;
	case 1:
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	case 2:
		params.setDefSkip(VSpace(VSpace::BIGSKIP));
		break;
	case 3:
	{
		VSpace vs = VSpace(
			widgetsToLength(textLayoutModule->skipLE,
				textLayoutModule->skipLengthCO)
			);
		params.setDefSkip(vs);
		break;
	}
	default:
		// DocumentDefskipCB assures that this never happens
		// so Assert then !!!  - jbl
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}

	params.options =
		fromqstr(latexModule->optionsLE->text());

	params.float_placement = floatModule->get();

	// fonts
	params.fontsRoman =
		tex_fonts_roman[fontModule->fontsRomanCO->currentIndex()];

	params.fontsSans =
		tex_fonts_sans[fontModule->fontsSansCO->currentIndex()];

	params.fontsTypewriter =
		tex_fonts_monospaced[fontModule->fontsTypewriterCO->currentIndex()];

	params.fontsSansScale = fontModule->scaleSansSB->value();

	params.fontsTypewriterScale = fontModule->scaleTypewriterSB->value();

	params.fontsSC = fontModule->fontScCB->isChecked();

	params.fontsOSF = fontModule->fontOsfCB->isChecked();

	params.fontsDefaultFamily = ControlDocument::fontfamilies[
		fontModule->fontsDefaultCO->currentIndex()];

	if (fontModule->fontsizeCO->currentIndex() == 0)
		params.fontsize = "default";
	else
		params.fontsize =
			fromqstr(fontModule->fontsizeCO->currentText());

	// paper
	params.papersize = PAPER_SIZE(
		pageLayoutModule->papersizeCO->currentIndex());

	// custom, A3, B3 and B4 paper sizes need geometry
	int psize = pageLayoutModule->papersizeCO->currentIndex();
	bool geom_papersize = (psize == 1 || psize == 5 || psize == 8 || psize == 9);

	params.paperwidth = widgetsToLength(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthUnitCO);

	params.paperheight = widgetsToLength(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightUnitCO);

	if (pageLayoutModule->facingPagesCB->isChecked())
		params.sides = TextClass::TwoSides;
	else
		params.sides = TextClass::OneSide;

	if (pageLayoutModule->landscapeRB->isChecked())
		params.orientation = ORIENTATION_LANDSCAPE;
	else
		params.orientation = ORIENTATION_PORTRAIT;

	// margins
	params.use_geometry =
		(!marginsModule->marginCB->isChecked()
		|| geom_papersize);

	Ui::MarginsUi const * m(marginsModule);

	params.leftmargin = widgetsToLength(m->innerLE, m->innerUnit);

	params.topmargin = widgetsToLength(m->topLE, m->topUnit);

	params.rightmargin = widgetsToLength(m->outerLE, m->outerUnit);

	params.bottommargin = widgetsToLength(m->bottomLE, m->bottomUnit);

	params.headheight = widgetsToLength(m->headheightLE, m->headheightUnit);

	params.headsep = widgetsToLength(m->headsepLE, m->headsepUnit);

	params.footskip = widgetsToLength(m->footskipLE, m->footskipUnit);

	branchesModule->apply(params);
}

namespace {

/** Return the position of val in the vector if found.
    If not found, return 0.
 */
template<class A>
typename std::vector<A>::size_type
findPos(std::vector<A> const & vec, A const & val)
{
	typename std::vector<A>::const_iterator it =
		std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return distance(vec.begin(), it);
}

} // namespace anom


void QDocumentDialog::updateParams(BufferParams const & params)
{
	// set the default unit
	// FIXME: move to controller
	Length::UNIT defaultUnit = Length::CM;
	switch (lyxrc.default_papersize) {
		case PAPER_DEFAULT: break;

		case PAPER_USLETTER:
		case PAPER_USLEGAL:
		case PAPER_USEXECUTIVE:
			defaultUnit = Length::IN;
			break;

		case PAPER_A3:
		case PAPER_A4:
		case PAPER_A5:
		case PAPER_B3:
		case PAPER_B4:
		case PAPER_B5:
			defaultUnit = Length::CM;
			break;
		case PAPER_CUSTOM:
			break;
	}

	// preamble
	preambleModule->update(params, form_->controller().id());

	// biblio
	biblioModule->citeDefaultRB->setChecked(
		params.getEngine() == biblio::ENGINE_BASIC);

	biblioModule->citeNatbibRB->setChecked(
		params.getEngine() == biblio::ENGINE_NATBIB_NUMERICAL ||
		params.getEngine() == biblio::ENGINE_NATBIB_AUTHORYEAR);

	biblioModule->citeStyleCO->setCurrentIndex(
		params.getEngine() == biblio::ENGINE_NATBIB_NUMERICAL);

	biblioModule->citeJurabibRB->setChecked(
		params.getEngine() == biblio::ENGINE_JURABIB);

	biblioModule->bibtopicCB->setChecked(
		params.use_bibtopic);

	// language & quotes
	int const pos = int(findPos(lang_,
				    params.language->lang()));
	langModule->languageCO->setCurrentIndex(pos);

	langModule->quoteStyleCO->setCurrentIndex(
		params.quotes_language);

	langModule->defaultencodingCB->setChecked(true);

	if (params.inputenc != "auto") {
		langModule->defaultencodingCB->setChecked(false);
		if (params.inputenc == "default") {
			langModule->encodingCO->setCurrentIndex(0);
		} else {
			int const i = langModule->encodingCO->findText(
					toqstr(params.inputenc));
			if (i >= 0)
				langModule->encodingCO->setCurrentIndex(i);
			else
				// unknown encoding. Set to default.
				langModule->defaultencodingCB->setChecked(true);
		}
	}

	// numbering
	int const min_toclevel = form_->controller().textClass().min_toclevel();
	int const max_toclevel = form_->controller().textClass().max_toclevel();
	if (form_->controller().textClass().hasTocLevels()) {
		numberingModule->setEnabled(true);
		numberingModule->depthSL->setMinimum(min_toclevel - 1);
		numberingModule->depthSL->setMaximum(max_toclevel);
		numberingModule->depthSL->setValue(params.secnumdepth);
		numberingModule->tocSL->setMaximum(min_toclevel - 1);
		numberingModule->tocSL->setMaximum(max_toclevel);
		numberingModule->tocSL->setValue(params.tocdepth);
		updateNumbering();
	} else {
		numberingModule->setEnabled(false);
		numberingModule->tocTW->clear();
	}

	// bullets
	bulletsModule->setBullet(0, params.user_defined_bullet(0));
	bulletsModule->setBullet(1, params.user_defined_bullet(1));
	bulletsModule->setBullet(2, params.user_defined_bullet(2));
	bulletsModule->setBullet(3, params.user_defined_bullet(3));
	bulletsModule->init();

	// packages
	int nitem = findToken(tex_graphics, params.graphicsDriver);
	if (nitem >= 0)
		latexModule->psdriverCO->setCurrentIndex(nitem);

	mathsModule->amsCB->setChecked(
		params.use_amsmath == BufferParams::package_on);
	mathsModule->amsautoCB->setChecked(
		params.use_amsmath == BufferParams::package_auto);

	mathsModule->esintCB->setChecked(
		params.use_esint == BufferParams::package_on);
	mathsModule->esintautoCB->setChecked(
		params.use_esint == BufferParams::package_auto);

	switch (params.spacing().getSpace()) {
		case Spacing::Other: nitem = 3; break;
		case Spacing::Double: nitem = 2; break;
		case Spacing::Onehalf: nitem = 1; break;
		case Spacing::Default: case Spacing::Single: nitem = 0; break;
	}

	// text layout
	latexModule->classCO->setCurrentIndex(params.textclass);

	updatePagestyle(form_->controller().textClass().opt_pagestyle(),
				 params.pagestyle);

	textLayoutModule->lspacingCO->setCurrentIndex(nitem);
	if (params.spacing().getSpace() == Spacing::Other) {
		textLayoutModule->lspacingLE->setText(
			toqstr(params.spacing().getValueAsString()));
	}
	setLSpacing(nitem);

	if (params.paragraph_separation
	    == BufferParams::PARSEP_INDENT) {
		textLayoutModule->indentRB->setChecked(true);
	} else {
		textLayoutModule->skipRB->setChecked(true);
	}

	int skip = 0;
	switch (params.getDefSkip().kind()) {
	case VSpace::SMALLSKIP:
		skip = 0;
		break;
	case VSpace::MEDSKIP:
		skip = 1;
		break;
	case VSpace::BIGSKIP:
		skip = 2;
		break;
	case VSpace::LENGTH:
	{
		skip = 3;
		string const length = params.getDefSkip().asLyXCommand();
		lengthToWidgets(textLayoutModule->skipLE,
			textLayoutModule->skipLengthCO,
			length, defaultUnit);
		break;
	}
	default:
		skip = 0;
		break;
	}
	textLayoutModule->skipCO->setCurrentIndex(skip);
	setSkip(skip);

	textLayoutModule->twoColumnCB->setChecked(
		params.columns == 2);

	// break listings_params to multiple lines
	string lstparams =
		InsetListingsParams(params.listings_params).separatedParams();
	textLayoutModule->listingsED->setPlainText(toqstr(lstparams));

	if (!params.options.empty()) {
		latexModule->optionsLE->setText(
			toqstr(params.options));
	} else {
		latexModule->optionsLE->setText("");
	}

	floatModule->set(params.float_placement);

	//fonts
	updateFontsize(form_->controller().textClass().opt_fontsize(),
			params.fontsize);

	int n = findToken(tex_fonts_roman, params.fontsRoman);
	if (n >= 0) {
		fontModule->fontsRomanCO->setCurrentIndex(n);
		romanChanged(n);
	}

	n = findToken(tex_fonts_sans, params.fontsSans);
	if (n >= 0)	{
		fontModule->fontsSansCO->setCurrentIndex(n);
		sansChanged(n);
	}

	n = findToken(tex_fonts_monospaced, params.fontsTypewriter);
	if (n >= 0) {
		fontModule->fontsTypewriterCO->setCurrentIndex(n);
		ttChanged(n);
	}

	fontModule->fontScCB->setChecked(params.fontsSC);
	fontModule->fontOsfCB->setChecked(params.fontsOSF);
	fontModule->scaleSansSB->setValue(params.fontsSansScale);
	fontModule->scaleTypewriterSB->setValue(params.fontsTypewriterScale);
	n = findToken(ControlDocument::fontfamilies, params.fontsDefaultFamily);
	if (n >= 0)
		fontModule->fontsDefaultCO->setCurrentIndex(n);

	// paper
	int const psize = params.papersize;
	pageLayoutModule->papersizeCO->setCurrentIndex(psize);
	setCustomPapersize(psize);

	bool const landscape =
		params.orientation == ORIENTATION_LANDSCAPE;
	pageLayoutModule->landscapeRB->setChecked(landscape);
	pageLayoutModule->portraitRB->setChecked(!landscape);

	pageLayoutModule->facingPagesCB->setChecked(
		params.sides == TextClass::TwoSides);


	lengthToWidgets(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthUnitCO, params.paperwidth, defaultUnit);

	lengthToWidgets(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightUnitCO, params.paperheight, defaultUnit);

	// margins
	Ui::MarginsUi * m = marginsModule;

	setMargins(!params.use_geometry);

	lengthToWidgets(m->topLE, m->topUnit,
		params.topmargin, defaultUnit);

	lengthToWidgets(m->bottomLE, m->bottomUnit,
		params.bottommargin, defaultUnit);

	lengthToWidgets(m->innerLE, m->innerUnit,
		params.leftmargin, defaultUnit);

	lengthToWidgets(m->outerLE, m->outerUnit,
		params.rightmargin, defaultUnit);

	lengthToWidgets(m->headheightLE, m->headheightUnit,
		params.headheight, defaultUnit);

	lengthToWidgets(m->headsepLE, m->headsepUnit,
		params.headsep, defaultUnit);

	lengthToWidgets(m->footskipLE, m->footskipUnit,
		params.footskip, defaultUnit);

	branchesModule->update(params);
}


/////////////////////////////////////////////////////////////////////
//
// Document
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlDocument, QView<QDocumentDialog> >
	DocumentBase;


QDocument::QDocument(Dialog & parent)
	: DocumentBase(parent, _("Document Settings"))
{}


void QDocument::build_dialog()
{
	dialog_.reset(new QDocumentDialog(this));
}


void QDocument::showPreamble()
{
	dialog_->showPreamble();
}


void QDocument::apply()
{
	if (!dialog_.get())
		return;

	dialog_->apply(controller().params());
}


void QDocument::update_contents()
{
	if (!dialog_.get())
		return;

	dialog_->updateParams(controller().params());
}

void QDocument::saveDocDefault()
{
	// we have to apply the params first
	apply();
	controller().saveAsDefault();
}


void QDocument::useClassDefaults()
{
	BufferParams & params = controller().params();

	///\todo verify the use of below with lyx-devel:
	params.textclass = dialog_->latexModule->classCO->currentIndex();

	params.useClassDefaults();
	update_contents();
}

} // namespace frontend
} // namespace lyx

#include "QDocument_moc.cpp"
