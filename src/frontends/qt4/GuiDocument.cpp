/**
 * \file GuiDocument.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Richard Heck (modules)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiDocument.h"

#include "BranchList.h"
#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Color.h"
#include "Encoding.h"
#include "FloatPlacement.h"
#include "frontend_helpers.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "GuiBranches.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "LaTeXHighlighter.h"
#include "Layout.h"
#include "LengthCombo.h"
#include "LyXRC.h" // defaultUnit
#include "ModuleList.h"
#include "OutputParams.h"
#include "PanelStack.h"
#include "PDFOptions.h"
#include "qt_helpers.h"
#include "Spacing.h"
#include "TextClassList.h"
#include "Validator.h"


// FIXME: those two headers are needed because of the
// WorkArea::redraw() call below.
#include "frontends/LyXView.h"
#include "frontends/WorkArea.h"

#include "insets/InsetListingsParams.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

#include <QCloseEvent>
#include <QScrollBar>
#include <QTextCursor>

#include <algorithm>
#include <sstream>

using std::distance;
using std::make_pair;
using std::pair;
using std::vector;
using std::string;
using std::ostringstream;
using std::sort;

///
template<class Pair>
std::vector<typename Pair::second_type> const
getSecond(std::vector<Pair> const & pr)
{
	 std::vector<typename Pair::second_type> tmp(pr.size());
	 std::transform(pr.begin(), pr.end(), tmp.begin(),
					 boost::bind(&Pair::second, _1));
	 return tmp;
}

char const * const tex_graphics[] =
{
	"default", "dvips", "dvitops", "emtex",
	"ln", "oztex", "textures", "none", ""
};


char const * const tex_graphics_gui[] =
{
	N_("Default"), "Dvips", "DVItoPS", "EmTeX",
	"LN", "OzTeX", "Textures", N_("None"), ""
};


char const * const tex_fonts_roman[] =
{
	"default", "cmr", "lmodern", "ae", "times", "palatino",
	"charter", "newcent", "bookman", "utopia", "beraserif",
	"ccfonts", "chancery", ""
};


char const * tex_fonts_roman_gui[] =
{
	N_("Default"), N_("Computer Modern Roman"), N_("Latin Modern Roman"),
	N_("AE (Almost European)"), N_("Times Roman"), N_("Palatino"),
	N_("Bitstream Charter"), N_("New Century Schoolbook"), N_("Bookman"),
	N_("Utopia"),  N_("Bera Serif"), N_("Concrete Roman"), N_("Zapf Chancery"),
	""
};


char const * const tex_fonts_sans[] =
{
	"default", "cmss", "lmss", "helvet", "avant", "berasans", "cmbr", ""
};


char const * tex_fonts_sans_gui[] =
{
	N_("Default"), N_("Computer Modern Sans"), N_("Latin Modern Sans"),
	N_("Helvetica"), N_("Avant Garde"), N_("Bera Sans"), N_("CM Bright"), ""
};


char const * const tex_fonts_monospaced[] =
{
	"default", "cmtt", "lmtt", "courier", "beramono", "luximono", "cmtl", ""
};


char const * tex_fonts_monospaced_gui[] =
{
	N_("Default"), N_("Computer Modern Typewriter"),
	N_("Latin Modern Typewriter"), N_("Courier"), N_("Bera Mono"),
	N_("LuxiMono"), N_("CM Typewriter Light"), ""
};


vector<pair<string, lyx::docstring> > pagestyles;


namespace lyx {
namespace frontend {

using support::token;
using support::bformat;
using support::findToken;
using support::getVectorFromString;

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



GuiDocument::GuiDocument(LyXView & lv)
	: GuiDialog(lv, "document"), Controller(this)
{
	setupUi(this);
	setController(this, false);
	setViewTitle(_("Document Settings"));

	lang_ = getSecond(getLanguageData(false));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));

	connect(savePB, SIGNAL(clicked()), this, SLOT(saveDefaultClicked()));
	connect(defaultPB, SIGNAL(clicked()), this, SLOT(useDefaultsClicked()));

	// Manage the restore, ok, apply, restore and cancel/close buttons
	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().setRestore(restorePB);

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
		this, SLOT(set_listings_msg()));
	connect(textLayoutModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(set_listings_msg()));
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
	bc().addCheckedLineEdit(textLayoutModule->skipLE);

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
		if (!isFontAvailable(tex_fonts_roman[n]))
			font += qt_(" (not installed)");
		fontModule->fontsRomanCO->addItem(font);
	}
	for (int n = 0; tex_fonts_sans[n][0]; ++n) {
		QString font = qt_(tex_fonts_sans_gui[n]);
		if (!isFontAvailable(tex_fonts_sans[n]))
			font += qt_(" (not installed)");
		fontModule->fontsSansCO->addItem(font);
	}
	for (int n = 0; tex_fonts_monospaced[n][0]; ++n) {
		QString font = qt_(tex_fonts_monospaced_gui[n]);
		if (!isFontAvailable(tex_fonts_monospaced[n]))
			font += qt_(" (not installed)");
		fontModule->fontsTypewriterCO->addItem(font);
	}

	fontModule->fontsizeCO->addItem(qt_("Default"));
	fontModule->fontsizeCO->addItem(qt_("10"));
	fontModule->fontsizeCO->addItem(qt_("11"));
	fontModule->fontsizeCO->addItem(qt_("12"));

	for (int n = 0; GuiDocument::fontfamilies_gui[n][0]; ++n)
		fontModule->fontsDefaultCO->addItem(
			qt_(GuiDocument::fontfamilies_gui[n]));


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
	bc().addCheckedLineEdit(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightL);
	bc().addCheckedLineEdit(pageLayoutModule->paperwidthLE,
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

	bc().addCheckedLineEdit(marginsModule->topLE,
		marginsModule->topL);
	bc().addCheckedLineEdit(marginsModule->bottomLE,
		marginsModule->bottomL);
	bc().addCheckedLineEdit(marginsModule->innerLE,
		marginsModule->innerL);
	bc().addCheckedLineEdit(marginsModule->outerLE,
		marginsModule->outerL);
	bc().addCheckedLineEdit(marginsModule->headsepLE,
		marginsModule->headsepL);
	bc().addCheckedLineEdit(marginsModule->headheightLE,
		marginsModule->headheightL);
	bc().addCheckedLineEdit(marginsModule->footskipLE,
		marginsModule->footskipL);


	langModule = new UiWidget<Ui::LanguageUi>;
	// language & quote
	connect(langModule->languageCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->defaultencodingRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(langModule->otherencodingRB, SIGNAL(clicked()),
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
		langModule->languageCO->addItem(toqstr(lit->first));
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
	
	selectionManager = 
		new GuiSelectionManager(latexModule->availableLV, latexModule->selectedLV, 
			latexModule->addPB, latexModule->deletePB, 
	 		latexModule->upPB, latexModule->downPB, 
			availableModel(), selectedModel());
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(updateModuleInfo()));
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(change_adaptor()));
	
	// postscript drivers
	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = qt_(tex_graphics_gui[n]);
		latexModule->psdriverCO->addItem(enc);
	}
	// latex classes
	//FIXME This seems too involved with the kernel. Some of this
	//should be moved to the kernel---which should perhaps just
	//give us a list of entries or something of the sort.
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
	branchesModule = new GuiBranches;
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

	// PDF support
	pdfSupportModule = new UiWidget<Ui::PDFSupportUi>;

	connect(pdfSupportModule->use_hyperrefGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->titleLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->authorLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->subjectLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->keywordsLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->bookmarksGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->bookmarksnumberedCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->bookmarksopenGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->bookmarksopenlevelSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->breaklinksCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->pdfborderCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->colorlinksCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->backrefCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->pagebackrefCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->fullscreenCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->optionsLE, SIGNAL(textChanged(const QString &)),
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
	docPS->addPanel(pdfSupportModule, _("PDF Properties"));
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


void GuiDocument::showPreamble()
{
	docPS->setCurrentPanel(_("LaTeX Preamble"));
}


void GuiDocument::saveDefaultClicked()
{
	saveDocDefault();
}


void GuiDocument::useDefaultsClicked()
{
	useClassDefaults();
}


void GuiDocument::change_adaptor()
{
	changed();
}


docstring GuiDocument::validate_listings_params()
{
	// use a cache here to avoid repeated validation
	// of the same parameters
	static string param_cache = string();
	static docstring msg_cache = docstring();
	
	if (textLayoutModule->bypassCB->isChecked())
		return docstring();

	string params = fromqstr(textLayoutModule->listingsED->toPlainText());
	if (params != param_cache) {
		param_cache = params;
		msg_cache = InsetListingsParams(params).validate();
	}
	return msg_cache;
}


void GuiDocument::set_listings_msg()
{
	static bool isOK = true;
	docstring msg = validate_listings_params();
	if (msg.empty()) {
		if (isOK)
			return;
		isOK = true;
		// listingsTB->setTextColor("black");
		textLayoutModule->listingsTB->setPlainText(
			qt_("Input listings parameters on the right. Enter ? for a list of parameters."));
	} else {
		isOK = false;
		// listingsTB->setTextColor("red");
		textLayoutModule->listingsTB->setPlainText(toqstr(msg));
	}
}


void GuiDocument::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiDocument::setLSpacing(int item)
{
	textLayoutModule->lspacingLE->setEnabled(item == 3);
}


void GuiDocument::setSkip(int item)
{
	bool const enable = (item == 3);
	textLayoutModule->skipLE->setEnabled(enable);
	textLayoutModule->skipLengthCO->setEnabled(enable);
}


void GuiDocument::enableSkip(bool skip)
{
	textLayoutModule->skipCO->setEnabled(skip);
	textLayoutModule->skipLE->setEnabled(skip);
	textLayoutModule->skipLengthCO->setEnabled(skip);
	if (skip)
		setSkip(textLayoutModule->skipCO->currentIndex());
}

void GuiDocument::portraitChanged()
{
	setMargins(pageLayoutModule->papersizeCO->currentIndex());
}

void GuiDocument::setMargins(bool custom)
{
	marginsModule->marginCB->setChecked(custom);
	setCustomMargins(custom);
}


void GuiDocument::setCustomPapersize(int papersize)
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


void GuiDocument::setCustomMargins(bool custom)
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


void GuiDocument::updateFontsize(string const & items, string const & sel)
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


void GuiDocument::romanChanged(int item)
{
	string const font = tex_fonts_roman[item];
	fontModule->fontScCB->setEnabled(providesSC(font));
	fontModule->fontOsfCB->setEnabled(providesOSF(font));
}


void GuiDocument::sansChanged(int item)
{
	string const font = tex_fonts_sans[item];
	bool scaleable = providesScale(font);
	fontModule->scaleSansSB->setEnabled(scaleable);
	fontModule->scaleSansLA->setEnabled(scaleable);
}


void GuiDocument::ttChanged(int item)
{
	string const font = tex_fonts_monospaced[item];
	bool scaleable = providesScale(font);
	fontModule->scaleTypewriterSB->setEnabled(scaleable);
	fontModule->scaleTypewriterLA->setEnabled(scaleable);
}


void GuiDocument::updatePagestyle(string const & items, string const & sel)
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

	int nn = 0;

	for (size_t i = 0; i < pagestyles.size(); ++i)
		if (pagestyles[i].first == sel)
			nn = pageLayoutModule->pagestyleCO->findText(
					toqstr(pagestyles[i].second));

	if (nn > 0)
		pageLayoutModule->pagestyleCO->setCurrentIndex(nn);
}


void GuiDocument::classChanged()
{
	textclass_type const tc = latexModule->classCO->currentIndex();
	bp_.setJustBaseClass(tc);
	if (lyxrc.auto_reset_options)
		bp_.useClassDefaults();
	updateContents();
}


void GuiDocument::updateModuleInfo()
{
	selectionManager->update();
	//Module description
	QListView const * const lv = selectionManager->selectedFocused() ?
	                             latexModule->selectedLV :
			latexModule->availableLV;
	if (lv->selectionModel()->selectedIndexes().isEmpty())
		latexModule->infoML->document()->clear();
	else {
		QModelIndex const idx = lv->selectionModel()->currentIndex();
		string const modName = fromqstr(idx.data().toString());
		string desc = getModuleDescription(modName);
		vector<string> pkgList = getPackageList(modName);
		string pkgdesc;
		//this mess formats the package list as "pkg1, pkg2, and pkg3"
		int const pkgListSize = pkgList.size();
		for (int i = 0; i < pkgListSize; ++i) {
			if (i == 1) {
				if (i == pkgListSize - 1) //last element
					pkgdesc += " and ";
				else
					pkgdesc += ", ";
			} else if (i > 1) {
				if (i == pkgListSize - 1) //last element
					pkgdesc += ", and ";
				else
					pkgdesc += ", ";
			}
			pkgdesc += pkgList[i];
		}
		if (!pkgdesc.empty())
			desc += " Requires " + pkgdesc + ".";
		latexModule->infoML->document()->setPlainText(toqstr(desc));
	}
}


void GuiDocument::updateNumbering()
{
	TextClass const & tclass = bp_.getTextClass();

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
			item->setText(0, toqstr(translateIfPossible((*cit)->name())));
			item->setText(1, (toclevel <= depth) ? yes : no);
			item->setText(2, (toclevel <= toc) ? yes : no);
		}
	}

	numberingModule->tocTW->setUpdatesEnabled(true);
	numberingModule->tocTW->update();
}


void GuiDocument::apply(BufferParams & params)
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
	if (langModule->defaultencodingRB->isChecked()) {
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
	
	// Modules
	params.clearLayoutModules();
	QStringList const selMods = selectedModel()->stringList();
	for (int i = 0; i != selMods.size(); ++i)
		params.addLayoutModule(lyx::fromqstr(selMods[i]));


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
	params.setJustBaseClass(latexModule->classCO->currentIndex());

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

	params.fontsDefaultFamily = GuiDocument::fontfamilies[
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

	// PDF support
	PDFOptions & pdf = params.pdfoptions();
	pdf.use_hyperref = pdfSupportModule->use_hyperrefGB->isChecked();
	pdf.title = fromqstr(pdfSupportModule->titleLE->text());
	pdf.author = fromqstr(pdfSupportModule->authorLE->text());
	pdf.subject = fromqstr(pdfSupportModule->subjectLE->text());
	pdf.keywords = fromqstr(pdfSupportModule->keywordsLE->text());

	pdf.bookmarks = pdfSupportModule->bookmarksGB->isChecked();
	pdf.bookmarksnumbered = pdfSupportModule->bookmarksnumberedCB->isChecked();
	pdf.bookmarksopen = pdfSupportModule->bookmarksopenGB->isChecked();
	pdf.bookmarksopenlevel = pdfSupportModule->bookmarksopenlevelSB->value();

	pdf.breaklinks = pdfSupportModule->breaklinksCB->isChecked();
	pdf.pdfborder = pdfSupportModule->pdfborderCB->isChecked();
	pdf.colorlinks = pdfSupportModule->colorlinksCB->isChecked();
	pdf.backref = pdfSupportModule->backrefCB->isChecked();
	pdf.pagebackref	= pdfSupportModule->pagebackrefCB->isChecked();
	if (pdfSupportModule->fullscreenCB->isChecked())
		pdf.pagemode = pdf.pagemode_fullscreen;
	else
		pdf.pagemode.clear();
	pdf.quoted_options = fromqstr(pdfSupportModule->optionsLE->text());
	if (pdf.use_hyperref || !pdf.empty())
		pdf.store_options = true;
}


/** Return the position of val in the vector if found.
    If not found, return 0.
 */
template<class A>
static size_t findPos(std::vector<A> const & vec, A const & val)
{
	typename std::vector<A>::const_iterator it =
		std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return distance(vec.begin(), it);
}


void GuiDocument::updateParams()
{
	updateParams(bp_);
}


void GuiDocument::updateParams(BufferParams const & params)
{
	// set the default unit
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
	preambleModule->update(params, id());

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

	bool default_enc = true;
	if (params.inputenc != "auto") {
		default_enc = false;
		if (params.inputenc == "default") {
			langModule->encodingCO->setCurrentIndex(0);
		} else {
			int const i = langModule->encodingCO->findText(
					toqstr(params.inputenc));
			if (i >= 0)
				langModule->encodingCO->setCurrentIndex(i);
			else
				// unknown encoding. Set to default.
				default_enc = true;
		}
	}
	langModule->defaultencodingRB->setChecked(default_enc);
	langModule->otherencodingRB->setChecked(!default_enc);

	// numbering
	int const min_toclevel = textClass().min_toclevel();
	int const max_toclevel = textClass().max_toclevel();
	if (textClass().hasTocLevels()) {
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
	updateModuleInfo();
	
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
	latexModule->classCO->setCurrentIndex(params.getBaseClass());
	
	updatePagestyle(textClass().opt_pagestyle(),
				 params.pagestyle);

	textLayoutModule->lspacingCO->setCurrentIndex(nitem);
	if (params.spacing().getSpace() == Spacing::Other) {
		textLayoutModule->lspacingLE->setText(
			toqstr(params.spacing().getValueAsString()));
	}
	setLSpacing(nitem);

	if (params.paragraph_separation == BufferParams::PARSEP_INDENT)
		textLayoutModule->indentRB->setChecked(true);
	else
		textLayoutModule->skipRB->setChecked(true);

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
		latexModule->optionsLE->setText(QString());
	}

	floatModule->set(params.float_placement);

	// Fonts
	updateFontsize(textClass().opt_fontsize(),
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
	n = findToken(GuiDocument::fontfamilies, params.fontsDefaultFamily);
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

	// PDF support
	PDFOptions const & pdf = params.pdfoptions();
	pdfSupportModule->use_hyperrefGB->setChecked(pdf.use_hyperref);
	pdfSupportModule->titleLE->setText(toqstr(pdf.title));
	pdfSupportModule->authorLE->setText(toqstr(pdf.author));
	pdfSupportModule->subjectLE->setText(toqstr(pdf.subject));
	pdfSupportModule->keywordsLE->setText(toqstr(pdf.keywords));

	pdfSupportModule->bookmarksGB->setChecked(pdf.bookmarks);
	pdfSupportModule->bookmarksnumberedCB->setChecked(pdf.bookmarksnumbered);
	pdfSupportModule->bookmarksopenGB->setChecked(pdf.bookmarksopen);

	pdfSupportModule->bookmarksopenlevelSB->setValue(pdf.bookmarksopenlevel);

	pdfSupportModule->breaklinksCB->setChecked(pdf.breaklinks);
	pdfSupportModule->pdfborderCB->setChecked(pdf.pdfborder);
	pdfSupportModule->colorlinksCB->setChecked(pdf.colorlinks);
	pdfSupportModule->backrefCB->setChecked(pdf.backref);
	pdfSupportModule->pagebackrefCB->setChecked(pdf.pagebackref);
	pdfSupportModule->fullscreenCB->setChecked
		(pdf.pagemode == pdf.pagemode_fullscreen);

	pdfSupportModule->optionsLE->setText(
		toqstr(pdf.quoted_options));
}


void GuiDocument::applyView()
{
	apply(params());
}


void GuiDocument::saveDocDefault()
{
	// we have to apply the params first
	applyView();
	saveAsDefault();
}


void GuiDocument::updateContents()
{
	//update list of available modules
	QStringList strlist;
	vector<string> const modNames = getModuleNames();
	vector<string>::const_iterator it = modNames.begin();
	for (; it != modNames.end(); ++it)
		strlist.push_back(toqstr(*it));
	available_model_.setStringList(strlist);
	//and selected ones, too
	QStringList strlist2;
	vector<string> const & selMods = getSelectedModules();
	it = selMods.begin();
	for (; it != selMods.end(); ++it)
		strlist2.push_back(toqstr(*it));
	selected_model_.setStringList(strlist2);

	updateParams(bp_);
}

void GuiDocument::useClassDefaults()
{
	bp_.setJustBaseClass(latexModule->classCO->currentIndex());
	bp_.useClassDefaults();
	updateContents();
}


bool GuiDocument::isValid()
{
	return validate_listings_params().empty();
}


char const * const GuiDocument::fontfamilies[5] = {
	"default", "rmdefault", "sfdefault", "ttdefault", ""
};


char const * GuiDocument::fontfamilies_gui[5] = {
	N_("Default"), N_("Roman"), N_("Sans Serif"), N_("Typewriter"), ""
};


bool GuiDocument::initialiseParams(string const &)
{
	bp_ = buffer().params();
	loadModuleNames();
	return true;
}


void GuiDocument::clearParams()
{
	bp_ = BufferParams();
}


BufferId GuiDocument::id() const
{
	return &buffer();
}


vector<string> GuiDocument::getModuleNames()
{
	return moduleNames_;
}


vector<string> const & GuiDocument::getSelectedModules()
{
	return params().getModules();
}


string GuiDocument::getModuleDescription(string const & modName) const
{
	LyXModule const * const mod = moduleList[modName];
	if (!mod)
		return string("Module unavailable!");
	return mod->description;
}


vector<string>
GuiDocument::getPackageList(string const & modName) const
{
	LyXModule const * const mod = moduleList[modName];
	if (!mod)
		return vector<string>(); //empty such thing
	return mod->packageList;
}


TextClass const & GuiDocument::textClass() const
{
	return textclasslist[bp_.getBaseClass()];
}


static void dispatch_bufferparams(Controller const & controller,
	BufferParams const & bp, kb_action lfun)
{
	ostringstream ss;
	ss << "\\begin_header\n";
	bp.writeFile(ss);
	ss << "\\end_header\n";
	controller.dispatch(FuncRequest(lfun, ss.str()));
}


void GuiDocument::dispatchParams()
{
	// This must come first so that a language change is correctly noticed
	setLanguage();

	// Apply the BufferParams. Note that this will set the base class
	// and then update the buffer's layout.
	//FIXME Could this be done last? Then, I think, we'd get the automatic
	//update mentioned in the next FIXME...
	dispatch_bufferparams(*this, params(), LFUN_BUFFER_PARAMS_APPLY);

	// Generate the colours requested by each new branch.
	BranchList & branchlist = params().branchlist();
	if (!branchlist.empty()) {
		BranchList::const_iterator it = branchlist.begin();
		BranchList::const_iterator const end = branchlist.end();
		for (; it != end; ++it) {
			docstring const & current_branch = it->getBranch();
			Branch const * branch = branchlist.find(current_branch);
			string const x11hexname =
					lyx::X11hexname(branch->getColor());
			// display the new color
			docstring const str = current_branch + ' ' + from_ascii(x11hexname);
			dispatch(FuncRequest(LFUN_SET_COLOR, str));
		}

		// Open insets of selected branches, close deselected ones
		dispatch(FuncRequest(LFUN_ALL_INSETS_TOGGLE,
			"assign branch"));
	}
	// FIXME: If we used an LFUN, we would not need those two lines:
	bufferview()->update();
	lyxview().currentWorkArea()->redraw();
}


void GuiDocument::setLanguage() const
{
	Language const * const newL = bp_.language;
	if (buffer().params().language == newL)
		return;

	string const & lang_name = newL->lang();
	dispatch(FuncRequest(LFUN_BUFFER_LANGUAGE, lang_name));
}


void GuiDocument::saveAsDefault() const
{
	dispatch_bufferparams(*this, params(), LFUN_BUFFER_SAVE_AS_DEFAULT);
}


bool GuiDocument::isFontAvailable(string const & font) const
{
	if (font == "default" || font == "cmr"
	    || font == "cmss" || font == "cmtt")
		// these are standard
		return true;
	if (font == "lmodern" || font == "lmss" || font == "lmtt")
		return LaTeXFeatures::isAvailable("lmodern");
	if (font == "times" || font == "palatino"
		 || font == "helvet" || font == "courier")
		return LaTeXFeatures::isAvailable("psnfss");
	if (font == "cmbr" || font == "cmtl")
		return LaTeXFeatures::isAvailable("cmbright");
	if (font == "utopia")
		return LaTeXFeatures::isAvailable("utopia")
			|| LaTeXFeatures::isAvailable("fourier");
	if (font == "beraserif" || font == "berasans"
		|| font == "beramono")
		return LaTeXFeatures::isAvailable("bera");
	return LaTeXFeatures::isAvailable(font);
}


bool GuiDocument::providesOSF(string const & font) const
{
	if (font == "cmr")
		return isFontAvailable("eco");
	if (font == "palatino")
		return isFontAvailable("mathpazo");
	return false;
}


bool GuiDocument::providesSC(string const & font) const
{
	if (font == "palatino")
		return isFontAvailable("mathpazo");
	if (font == "utopia")
		return isFontAvailable("fourier");
	return false;
}


bool GuiDocument::providesScale(string const & font) const
{
	return font == "helvet" || font == "luximono"
		|| font == "berasans"  || font == "beramono";
}


void GuiDocument::loadModuleNames ()
{
	moduleNames_.clear();
	LyXModuleList::const_iterator it = moduleList.begin();
	for (; it != moduleList.end(); ++it)
		moduleNames_.push_back(it->name);
	if (!moduleNames_.empty())
		sort(moduleNames_.begin(), moduleNames_.end());
}


Dialog * createGuiDocument(LyXView & lv) { return new GuiDocument(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiDocument_moc.cpp"
