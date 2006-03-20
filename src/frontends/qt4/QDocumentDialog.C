/**
 * \file QDocumentDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QDocument.h"
#include "QDocumentDialog.h"

#include <QCloseEvent>

#include "floatplacement.h"
#include "lengthcombo.h"
#include "validators.h"
#include "panelstack.h"
#include "Qt2BC.h"
#include "checkedwidgets.h"
#include "qt_helpers.h"

#include "bufferparams.h"
#include "floatplacement.h"
#include "gettext.h"
#include "helper_funcs.h" // getSecond()
#include "language.h"
#include "lyxrc.h" // defaultUnit
#include "lyxtextclasslist.h"
#include "tex-strings.h" // tex_graphics
#include "Spacing.h"

#include "controllers/ControlDocument.h"
#include "controllers/frnt_lang.h"

#include "support/lstrings.h"

#include "controllers/ControlDocument.h"
#include "controllers/frnt_lang.h"


#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>
#include <QPixmap>
#include <QColor>
#include <QColorDialog>
#include <QValidator>

using lyx::support::token;
using lyx::support::bformat;
using lyx::support::getVectorFromString;

using std::string;


using std::distance;
using std::vector;
using std::string;


namespace lyx {
namespace frontend {


namespace {

char const * encodings[] = { "LaTeX default", "latin1", "latin2",
	"latin3", "latin4", "latin5", "latin9",
	"koi8-r", "koi8-u", "cp866", "cp1251",
	"iso88595", "pt154", 0
};

}

/*
QDocumentDialog::getTextClass()
{
	return latexModule->classCO->currentItem();
}
*/

QDocumentDialog::QDocumentDialog(QDocument * form)
	: form_(form), 
	lang_(getSecond(getLanguageData(false)))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	

    connect( savePB, SIGNAL( clicked() ), this, SLOT( saveDefaultClicked() ) );
    connect( defaultPB, SIGNAL( clicked() ), this, SLOT( useDefaultsClicked() ) );

	// Manage the restore, ok, apply, restore and cancel/close buttons
	form_->bcview().setOK(okPB);
	form_->bcview().setApply(applyPB);
	form_->bcview().setCancel(closePB);
	form_->bcview().setRestore(restorePB);


	textLayoutModule = new UiWidget<Ui::TextLayoutUi>;
	// text layout
	connect(textLayoutModule->fontsCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->fontsizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)), this, SLOT(setLSpacing(int)));
	connect(textLayoutModule->lspacingLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->indentRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLengthCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)), this, SLOT(setSkip(int)));
	connect(textLayoutModule->skipRB, SIGNAL(toggled(bool)), this, SLOT(enableSkip(bool)));
	connect(textLayoutModule->twoColumnCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	textLayoutModule->lspacingLE->setValidator(new QDoubleValidator(
		textLayoutModule->lspacingLE));
	textLayoutModule->skipLE->setValidator(unsignedLengthValidator(
		textLayoutModule->skipLE));

	for (int n = 0; tex_fonts[n][0]; ++n) {
		QString font = tex_fonts[n];
		textLayoutModule->fontsCO->insertItem(font);
	}

	textLayoutModule->fontsizeCO->insertItem(qt_("default"));
	textLayoutModule->fontsizeCO->insertItem(qt_("10"));
	textLayoutModule->fontsizeCO->insertItem(qt_("11"));
	textLayoutModule->fontsizeCO->insertItem(qt_("12"));

	textLayoutModule->skipCO->insertItem(qt_("SmallSkip"));
	textLayoutModule->skipCO->insertItem(qt_("MedSkip"));
	textLayoutModule->skipCO->insertItem(qt_("BigSkip"));
	textLayoutModule->skipCO->insertItem(qt_("Length"));
	// remove the %-items from the unit choice
	textLayoutModule->skipLengthCO->noPercents();
	textLayoutModule->lspacingCO->insertItem(
		qt_("Single"), Spacing::Single);
	textLayoutModule->lspacingCO->insertItem(
		qt_("OneHalf"), Spacing::Onehalf);
	textLayoutModule->lspacingCO->insertItem(
		qt_("Double"), Spacing::Double);
	textLayoutModule->lspacingCO->insertItem(
		qt_("Custom"), Spacing::Other);
	
	// initialize the length validator
	addCheckedLineEdit(form_->bcview(), textLayoutModule->skipLE);
	


	
	pageLayoutModule = new UiWidget<Ui::PageLayoutUi>;
	// page layout
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)), this, SLOT(setCustomPapersize(int)));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)), this, SLOT(setCustomPapersize(int)));
	connect(pageLayoutModule->portraitRB, SIGNAL(toggled(bool)), this, SLOT(portraitChanged()));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperheightLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperwidthLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperwidthUnitCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperheightUnitCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->portraitRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->landscapeRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->facingPagesCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->pagestyleCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	pageLayoutModule->pagestyleCO->insertItem(qt_("default"));
	pageLayoutModule->pagestyleCO->insertItem(qt_("empty"));
	pageLayoutModule->pagestyleCO->insertItem(qt_("plain"));
	pageLayoutModule->pagestyleCO->insertItem(qt_("headings"));
	pageLayoutModule->pagestyleCO->insertItem(qt_("fancy"));
	addCheckedLineEdit(form_->bcview(), pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightL);
	addCheckedLineEdit(form_->bcview(), pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthL);
	
	// paper
	QComboBox * cb = pageLayoutModule->papersizeCO;
	cb->insertItem(qt_("Default"));
	cb->insertItem(qt_("Custom"));
	cb->insertItem(qt_("US letter"));
	cb->insertItem(qt_("US legal"));
	cb->insertItem(qt_("US executive"));
	cb->insertItem(qt_("A3"));
	cb->insertItem(qt_("A4"));
	cb->insertItem(qt_("A5"));
	cb->insertItem(qt_("B3"));
	cb->insertItem(qt_("B4"));
	cb->insertItem(qt_("B5"));
	// remove the %-items from the unit choice
	pageLayoutModule->paperwidthUnitCO->noPercents();
	pageLayoutModule->paperheightUnitCO->noPercents();
	pageLayoutModule->paperheightLE->setValidator(unsignedLengthValidator(
		pageLayoutModule->paperheightLE));
	pageLayoutModule->paperwidthLE->setValidator(unsignedLengthValidator(
		pageLayoutModule->paperwidthLE));




	marginsModule = new UiWidget<Ui::MarginsUi>;
	// margins
	connect(marginsModule->marginCB, SIGNAL(toggled(bool)), this, SLOT(setCustomMargins(bool)));
	connect(marginsModule->marginCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(marginsModule->topLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->topUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->bottomLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->bottomUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->innerLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->innerUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->outerLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->outerUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->headheightLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->headheightUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->headsepLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->headsepUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->footskipLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->footskipUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));	
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
    connect( langModule->defaultencodingCB, SIGNAL( toggled(bool) ), langModule->encodingL, SLOT( setDisabled(bool) ) );
    connect( langModule->defaultencodingCB, SIGNAL( toggled(bool) ), langModule->encodingCO, SLOT( setDisabled(bool) ) );
	// language & quote
	connect(langModule->languageCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(langModule->defaultencodingCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(langModule->encodingCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(langModule->quoteStyleCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	// language & quotes
	vector<LanguagePair> const langs = getLanguageData(false);
	vector<LanguagePair>::const_iterator lit  = langs.begin();
	vector<LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		langModule->languageCO->insertItem(
			toqstr(lit->first));
	}

	int k = 0;
	while (encodings[k]) {
		langModule->encodingCO->insertItem(qt_(encodings[k++]));
	}

	langModule->quoteStyleCO->insertItem(qt_("``text''"));
	langModule->quoteStyleCO->insertItem(qt_("''text''"));
	langModule->quoteStyleCO->insertItem(qt_(",,text``"));
	langModule->quoteStyleCO->insertItem(qt_(",,text''"));
	langModule->quoteStyleCO->insertItem(qt_("<<text>>"));
	langModule->quoteStyleCO->insertItem(qt_(">>text<<"));



	numberingModule = new UiWidget<Ui::NumberingUi>;
	// numbering
	connect(numberingModule->depthSL, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(numberingModule->tocSL, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(numberingModule->depthSL, SIGNAL(valueChanged(int)), this, SLOT(updateNumbering()));
	connect(numberingModule->tocSL, SIGNAL(valueChanged(int)), this, SLOT(updateNumbering()));
	numberingModule->tocLV->setSorting(-1);



	biblioModule = new UiWidget<Ui::BiblioUi>;
    connect( biblioModule->citeNatbibRB, SIGNAL( toggled(bool) ), biblioModule->citationStyleL, SLOT( setEnabled(bool) ) );
    connect( biblioModule->citeNatbibRB, SIGNAL( toggled(bool) ), biblioModule->citeStyleCO, SLOT( setEnabled(bool) ) );
	// biblio
	connect(biblioModule->citeDefaultRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(biblioModule->citeNatbibRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(biblioModule->citeStyleCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(biblioModule->citeJurabibRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(biblioModule->bibtopicCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	// biblio
	biblioModule->citeStyleCO->insertItem(qt_("Author-year"));
	biblioModule->citeStyleCO->insertItem(qt_("Numerical"));
	biblioModule->citeStyleCO->setCurrentItem(0);

	
	
	mathsModule = new UiWidget<Ui::MathsUi>;
    connect( mathsModule->amsautoCB, SIGNAL( toggled(bool) ), mathsModule->amsCB, SLOT( setDisabled(bool) ) );
	// maths
	connect(mathsModule->amsCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(mathsModule->amsautoCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));

	
	latexModule = new UiWidget<Ui::LaTeXUi>;
	// latex class
	connect(latexModule->classCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(latexModule->optionsLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->psdriverCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(latexModule->classCO, SIGNAL(activated(int)), this, SLOT(classChanged()));
	// packages
	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = tex_graphics[n];
		latexModule->psdriverCO->insertItem(enc);
	}
	// latex
	for (LyXTextClassList::const_iterator cit = textclasslist.begin();
	     cit != textclasslist.end(); ++cit) {
		if (cit->isTeXClassAvailable()) {
			latexModule->classCO->insertItem(toqstr(cit->description()));
		} else {
			string item =
				bformat(_("Unavailable: %1$s"), cit->description());
			latexModule->classCO->insertItem(toqstr(item));
		}
	}
	

	
	branchesModule = new QBranches;
	connect(branchesModule, SIGNAL(changed()), this, SLOT(change_adaptor()));
	

	preambleModule = new UiWidget<Ui::PreambleUi>;
	// preamble
	connect(preambleModule->preambleMLE, SIGNAL(textChanged()), this, SLOT(change_adaptor()));


	bulletsModule = new BulletsModule;
	// bullets
	connect(bulletsModule, SIGNAL(changed()), this, SLOT(change_adaptor()));

	
	floatModule = new FloatPlacement(this);
	// float
	connect(floatModule, SIGNAL(changed()), this, SLOT(change_adaptor()));

	docPS->addPanel(latexModule, _("Document Class"));
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
}				


QDocumentDialog::~QDocumentDialog()
{
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
		setSkip(textLayoutModule->skipCO->currentItem());
}

void QDocumentDialog::portraitChanged()
{
	setMargins(pageLayoutModule->papersizeCO->currentItem());
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
	textLayoutModule->fontsizeCO->clear();
	textLayoutModule->fontsizeCO->insertItem("default");

	for (int n = 0; !token(items,'|',n).empty(); ++n)
		textLayoutModule->fontsizeCO->
			insertItem(toqstr(token(items,'|',n)));

	for (int n = 0; n<textLayoutModule->fontsizeCO->count(); ++n) {
		if (fromqstr(textLayoutModule->fontsizeCO->text(n)) == sel) {
			textLayoutModule->fontsizeCO->setCurrentItem(n);
			break;
		}
	}
}


void QDocumentDialog::updatePagestyle(string const & items, string const & sel)
{
	pageLayoutModule->pagestyleCO->clear();
	pageLayoutModule->pagestyleCO->insertItem("default");

	for (int n=0; !token(items,'|',n).empty(); ++n)
		pageLayoutModule->pagestyleCO->
			insertItem(toqstr(token(items,'|',n)));

	for (int n = 0; n<pageLayoutModule->pagestyleCO->count(); ++n) {
		if (fromqstr(pageLayoutModule->pagestyleCO->text(n))==sel) {
			pageLayoutModule->pagestyleCO->setCurrentItem(n);
			break;
		}
	}
}


void QDocumentDialog::classChanged()
{
	ControlDocument & cntrl = form_->controller();
	BufferParams & params = cntrl.params();

	lyx::textclass_type const tc = latexModule->classCO->currentItem();

	if (form_->controller().loadTextclass(tc)) {
		params.textclass = tc;
		if (lyxrc.auto_reset_options)
			params.useClassDefaults();
		form_->update_contents();
	} else {
		latexModule->classCO->setCurrentItem(params.textclass);
	}
}


void QDocumentDialog::updateNumbering()
{
	LyXTextClass const & tclass =
		form_->controller().params().getLyXTextClass();

	//numberingModule->tocLV->setUpdatesEnabled(false);

	// Update the example QListView
	int const depth = numberingModule->depthSL->value();
	int const toc = numberingModule->tocSL->value();
	QString const no = qt_("No");
	QString const yes = qt_("Yes");
	LyXTextClass::const_iterator end = tclass.end();
	LyXTextClass::const_iterator cit = tclass.begin();
	numberingModule->tocLV->clear();
	Q3ListViewItem * item = 0;
	for ( ; cit != end ; ++cit) {
		int const toclevel = (*cit)->toclevel;
		if (toclevel != LyXLayout::NOT_IN_TOC) {
			item = new Q3ListViewItem(numberingModule->tocLV,
						 item, qt_((*cit)->name()));
			item->setText(1, (toclevel <= depth) ? yes : no);
			item->setText(2, (toclevel <= toc) ? yes : no);
		}
	}

	//numberingModule->tocLV->setUpdatesEnabled(true);
	//numberingModule->tocLV->update();
}

void QDocumentDialog::apply(BufferParams & params)
{
	// preamble
	params.preamble =
		fromqstr(preambleModule->preambleMLE->text());

	// biblio
	params.cite_engine = biblio::ENGINE_BASIC;

	if (biblioModule->citeNatbibRB->isChecked()) {
		bool const use_numerical_citations =
			biblioModule->citeStyleCO->currentItem();
		if (use_numerical_citations)
			params.cite_engine = biblio::ENGINE_NATBIB_NUMERICAL;
		else
			params.cite_engine = biblio::ENGINE_NATBIB_AUTHORYEAR;

	} else if (biblioModule->citeJurabibRB->isChecked())
		params.cite_engine = biblio::ENGINE_JURABIB;

	params.use_bibtopic =
		biblioModule->bibtopicCB->isChecked();

	// language & quotes
	if (langModule->defaultencodingCB->isChecked()) {
		params.inputenc = "auto";
	} else {
		int i = langModule->encodingCO->currentItem();
		if (i == 0) {
			params.inputenc = "default";
		} else {
			params.inputenc = encodings[i];
		}
	}

	InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;
	switch (langModule->quoteStyleCO->currentItem()) {
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

	int const pos = langModule->languageCO->currentItem();
	params.language = languages.getLanguage(lang_[pos]);

	// numbering
	params.tocdepth = numberingModule->tocSL->value();
	params.secnumdepth = numberingModule->depthSL->value();

	// bullets
	params.user_defined_bullet(0) = bulletsModule->getBullet(0);
	params.user_defined_bullet(1) = bulletsModule->getBullet(1);
	params.user_defined_bullet(2) = bulletsModule->getBullet(2);
	params.user_defined_bullet(3) = bulletsModule->getBullet(3);

	// packages
	params.graphicsDriver =
		fromqstr(latexModule->psdriverCO->currentText());

	if (mathsModule->amsautoCB->isChecked()) {
		params.use_amsmath = BufferParams::AMS_AUTO;
	} else {
		if (mathsModule->amsCB->isChecked())
			params.use_amsmath = BufferParams::AMS_ON;
		else
			params.use_amsmath = BufferParams::AMS_OFF;
	}

	// layout
	params.textclass =
		latexModule->classCO->currentItem();

	params.fonts =
		fromqstr(textLayoutModule->fontsCO->currentText());

	params.fontsize =
		fromqstr(textLayoutModule->fontsizeCO->currentText());

	params.pagestyle =
		fromqstr(pageLayoutModule->pagestyleCO->currentText());

	switch (textLayoutModule->lspacingCO->currentItem()) {
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

	if (textLayoutModule->indentRB->isChecked())
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params.paragraph_separation = BufferParams::PARSEP_SKIP;

	switch (textLayoutModule->skipCO->currentItem()) {
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

	// paper
	params.papersize = PAPER_SIZE(
		pageLayoutModule->papersizeCO->currentItem());

	// custom, A3, B3 and B4 paper sizes need geometry
	int psize = pageLayoutModule->papersizeCO->currentItem();
	bool geom_papersize = (psize == 1 || psize == 5 || psize == 8 || psize == 9);

	params.paperwidth = widgetsToLength(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthUnitCO);

	params.paperheight = widgetsToLength(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightUnitCO);

	if (pageLayoutModule->facingPagesCB->isChecked())
		params.sides = LyXTextClass::TwoSides;
	else
		params.sides = LyXTextClass::OneSide;

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


void QDocumentDialog::update(BufferParams const & params)
{
	// set the default unit
	// FIXME: move to controller
	LyXLength::UNIT defaultUnit = LyXLength::CM;
	switch (lyxrc.default_papersize) {
		case PAPER_DEFAULT: break;

		case PAPER_USLETTER:
		case PAPER_USLEGAL:
		case PAPER_USEXECUTIVE:
			defaultUnit = LyXLength::IN;
			break;

		case PAPER_A3:
		case PAPER_A4:
		case PAPER_A5:
		case PAPER_B3:
		case PAPER_B4:
		case PAPER_B5:
			defaultUnit = LyXLength::CM;
			break;
		case PAPER_CUSTOM:
			break;
	}

	// preamble
	QString preamble = toqstr(params.preamble);
	preambleModule->preambleMLE->setText(preamble);

	// biblio
	biblioModule->citeDefaultRB->setChecked(
		params.cite_engine == biblio::ENGINE_BASIC);

	biblioModule->citeNatbibRB->setChecked(
		params.cite_engine == biblio::ENGINE_NATBIB_NUMERICAL ||
		params.cite_engine == biblio::ENGINE_NATBIB_AUTHORYEAR);

	biblioModule->citeStyleCO->setCurrentItem(
		params.cite_engine == biblio::ENGINE_NATBIB_NUMERICAL);

	biblioModule->citeJurabibRB->setChecked(
		params.cite_engine == biblio::ENGINE_JURABIB);

	biblioModule->bibtopicCB->setChecked(
		params.use_bibtopic);

	// language & quotes
	int const pos = int(findPos(lang_,
				    params.language->lang()));
	langModule->languageCO->setCurrentItem(pos);

	langModule->quoteStyleCO->setCurrentItem(
		params.quotes_language);

	langModule->defaultencodingCB->setChecked(true);

	if (params.inputenc != "auto") {
		langModule->defaultencodingCB->setChecked(false);
		if (params.inputenc == "default") {
			langModule->encodingCO->setCurrentItem(0);
		} else {
			int i = 0;
			while (encodings[i]) {
				if (encodings[i] == params.inputenc)
					langModule->encodingCO->setCurrentItem(i);
				++i;
			}
		}
	}

	// numbering
	int const min_toclevel = form_->controller().textClass().min_toclevel();
	int const max_toclevel = form_->controller().textClass().max_toclevel();
	if (min_toclevel != LyXLayout::NOT_IN_TOC)
		numberingModule->setEnabled(true);
	else {
		numberingModule->setEnabled(false);
		numberingModule->tocLV->clear();
	}
	numberingModule->depthSL->setMinValue(min_toclevel - 1);
	numberingModule->depthSL->setMaxValue(max_toclevel);
	numberingModule->depthSL->setValue(params.secnumdepth);
	numberingModule->tocSL->setMinValue(min_toclevel - 1);
	numberingModule->tocSL->setMaxValue(max_toclevel);
	numberingModule->tocSL->setValue(params.tocdepth);
	updateNumbering();

	// bullets
	bulletsModule->setBullet(0,params.user_defined_bullet(0));
	bulletsModule->setBullet(1,params.user_defined_bullet(1));
	bulletsModule->setBullet(2,params.user_defined_bullet(2));
	bulletsModule->setBullet(3,params.user_defined_bullet(3));

	// packages
	QString text = toqstr(params.graphicsDriver);
	int nitem = latexModule->psdriverCO->count();
	for (int n = 0; n < nitem ; ++n) {
		QString enc = tex_graphics[n];
		if (enc == text) {
			latexModule->psdriverCO->setCurrentItem(n);
		}
	}


	mathsModule->amsCB->setChecked(
		params.use_amsmath == BufferParams::AMS_ON);
	mathsModule->amsautoCB->setChecked(
		params.use_amsmath == BufferParams::AMS_AUTO);

	switch (params.spacing().getSpace()) {
		case Spacing::Other: nitem = 3; break;
		case Spacing::Double: nitem = 2; break;
		case Spacing::Onehalf: nitem = 1; break;
		case Spacing::Default: case Spacing::Single: nitem = 0; break;
	}

	// layout
	latexModule->classCO->setCurrentItem(params.textclass);

	updateFontsize(form_->controller().textClass().opt_fontsize(),
				params.fontsize);

	updatePagestyle(form_->controller().textClass().opt_pagestyle(),
				 params.pagestyle);

	for (int n = 0; tex_fonts[n][0]; ++n) {
		if (tex_fonts[n] == params.fonts) {
			textLayoutModule->fontsCO->setCurrentItem(n);
			break;
		}
	}

	textLayoutModule->lspacingCO->setCurrentItem(nitem);
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
	textLayoutModule->skipCO->setCurrentItem(skip);
	setSkip(skip);

	textLayoutModule->twoColumnCB->setChecked(
		params.columns == 2);

	if (!params.options.empty()) {
		latexModule->optionsLE->setText(
			toqstr(params.options));
	} else {
		latexModule->optionsLE->setText("");
	}

	floatModule->set(params.float_placement);

	// paper
	int const psize = params.papersize;
	pageLayoutModule->papersizeCO->setCurrentItem(psize);
	setCustomPapersize(psize);

	bool const landscape =
		params.orientation == ORIENTATION_LANDSCAPE;
	pageLayoutModule->landscapeRB->setChecked(landscape);
	pageLayoutModule->portraitRB->setChecked(!landscape);

	pageLayoutModule->facingPagesCB->setChecked(
		params.sides == LyXTextClass::TwoSides);


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




} // namespace frontend
} // namespace lyx
