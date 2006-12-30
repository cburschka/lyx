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

#include "FloatPlacement.h"
#include "lengthcombo.h"
#include "validators.h"
#include "panelstack.h"
#include "Qt2BC.h"
#include "checkedwidgets.h"
#include "qt_helpers.h"

#include "bufferparams.h"
#include "encoding.h"
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


using lyx::support::token;
using lyx::support::bformat;
using lyx::support::findToken;
using lyx::support::getVectorFromString;

using std::distance;
using std::vector;
using std::string;


namespace lyx {
namespace frontend {


QDocumentDialog::QDocumentDialog(QDocument * form)
	: form_(form),
	lang_(getSecond(getLanguageData(false)))
{
	setupUi(this);

	Q_CONNECT_1(QPushButton, okPB, clicked, bool,
				QDocument, form, slotOK, void);
	Q_CONNECT_1(QPushButton, applyPB, clicked, bool,
				QDocument, form, slotApply, void);
	Q_CONNECT_1(QPushButton, closePB, clicked, bool,
				QDocument, form, slotClose, void);
	Q_CONNECT_1(QPushButton, restorePB, clicked, bool,
				QDocument, form, slotRestore, void);


	Q_CONNECT_1(QPushButton, savePB, clicked, bool,
				QDocumentDialog, this, saveDefaultClicked, void);
	Q_CONNECT_1(QPushButton, defaultPB, clicked, bool,
				QDocumentDialog, this, useDefaultsClicked, void);

	// Manage the restore, ok, apply, restore and cancel/close buttons
	form_->bcview().setOK(okPB);
	form_->bcview().setApply(applyPB);
	form_->bcview().setCancel(closePB);
	form_->bcview().setRestore(restorePB);


	textLayoutModule = new UiWidget<Ui::TextLayoutUi>;
	// text layout
	Q_CONNECT_1(QComboBox, textLayoutModule->lspacingCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	Q_CONNECT_1(QComboBox, textLayoutModule->lspacingCO, activated, int, 
				QDocumentDialog, this, setLSpacing, int);
	Q_CONNECT_1(QLineEdit, textLayoutModule->lspacingLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	Q_CONNECT_1(QRadioButton, textLayoutModule->skipRB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	Q_CONNECT_1(QRadioButton, textLayoutModule->indentRB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	Q_CONNECT_1(QComboBox, textLayoutModule->skipCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	Q_CONNECT_1(QLineEdit, textLayoutModule->skipLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	Q_CONNECT_1(QComboBox, textLayoutModule->skipLengthCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	Q_CONNECT_1(QComboBox, textLayoutModule->skipCO, activated, int, 
				QDocumentDialog, this, setSkip, int);
	Q_CONNECT_1(QRadioButton, textLayoutModule->skipRB, toggled, bool, 
				QDocumentDialog, this, enableSkip, bool);
	Q_CONNECT_1(QCheckBox, textLayoutModule->twoColumnCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);

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

	Q_CONNECT_1(QComboBox, fontModule->fontsRomanCO, activated, int,
				QDocumentDialog, this, change_adaptor, void);

	Q_CONNECT_1(QComboBox, fontModule->fontsRomanCO, activated, int, 
				QDocumentDialog, this, romanChanged, int);

	Q_CONNECT_1(QComboBox, fontModule->fontsSansCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void)
	
	Q_CONNECT_1(QComboBox, fontModule->fontsSansCO, activated, int, 
				QDocumentDialog, this, sansChanged, int);
	
	Q_CONNECT_1(QComboBox, fontModule->fontsTypewriterCO, activated,int, 
				QDocumentDialog, this, change_adaptor, void);

	Q_CONNECT_1(QComboBox, fontModule->fontsTypewriterCO, activated, int, 
				QDocumentDialog, this, ttChanged, int);

	Q_CONNECT_1(QComboBox, fontModule->fontsDefaultCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);

	Q_CONNECT_1(QComboBox, fontModule->fontsizeCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);

	Q_CONNECT_1(QSpinBox, fontModule->scaleSansSB, valueChanged, int, 
				QDocumentDialog, this, change_adaptor, void);

	Q_CONNECT_1(QSpinBox, fontModule->scaleTypewriterSB, valueChanged, int, 
				QDocumentDialog, this, change_adaptor, void);

	Q_CONNECT_1(QCheckBox, fontModule->fontScCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);

	Q_CONNECT_1(QCheckBox, fontModule->fontOsfCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);

	for (int n = 0; tex_fonts_roman[n][0]; ++n) {
		QString font = toqstr(tex_fonts_roman_gui[n]);
		if (!form_->controller().isFontAvailable(tex_fonts_roman[n]))
			font += qt_(" (not installed)");
		fontModule->fontsRomanCO->addItem(font);
	}
	for (int n = 0; tex_fonts_sans[n][0]; ++n) {
		QString font = toqstr(tex_fonts_sans_gui[n]);
		if (!form_->controller().isFontAvailable(tex_fonts_sans[n]))
			font += qt_(" (not installed)");
		fontModule->fontsSansCO->addItem(font);
	}
	for (int n = 0; tex_fonts_monospaced[n][0]; ++n) {
		QString font = toqstr(tex_fonts_monospaced_gui[n]);
		if (!form_->controller().isFontAvailable(tex_fonts_monospaced[n]))
			font += qt_(" (not installed)");
		fontModule->fontsTypewriterCO->addItem(font);
	}

	fontModule->fontsizeCO->addItem(qt_("default"));
	fontModule->fontsizeCO->addItem(qt_("10"));
	fontModule->fontsizeCO->addItem(qt_("11"));
	fontModule->fontsizeCO->addItem(qt_("12"));

	for (int n = 0; ControlDocument::fontfamilies_gui[n][0]; ++n)
		fontModule->fontsDefaultCO->addItem(
			qt_(ControlDocument::fontfamilies_gui[n]));




	pageLayoutModule = new UiWidget<Ui::PageLayoutUi>;
	// page layout
	Q_CONNECT_1(QComboBox, pageLayoutModule->papersizeCO, activated, int, 
				QDocumentDialog, this, setCustomPapersize, int);
	
	Q_CONNECT_1(QComboBox, pageLayoutModule->papersizeCO, activated, int, 
				QDocumentDialog, this, setCustomPapersize, int);
	
	Q_CONNECT_1(QRadioButton, pageLayoutModule->portraitRB, toggled, bool, 
				QDocumentDialog, this, portraitChanged, void);
	
	Q_CONNECT_1(QComboBox, pageLayoutModule->papersizeCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, pageLayoutModule->paperheightLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, pageLayoutModule->paperwidthLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QComboBox, pageLayoutModule->paperwidthUnitCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(LengthCombo,  pageLayoutModule->paperheightUnitCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QRadioButton, pageLayoutModule->portraitRB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QRadioButton, pageLayoutModule->landscapeRB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QCheckBox, pageLayoutModule->facingPagesCB, toggled, bool,
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QComboBox, pageLayoutModule->pagestyleCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);

	pageLayoutModule->pagestyleCO->addItem(qt_("default"));
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
	
	Q_CONNECT_1(QCheckBox, marginsModule->marginCB, toggled, bool, 
				QDocumentDialog, this, setCustomMargins, bool);
	
	Q_CONNECT_1(QCheckBox, marginsModule->marginCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, marginsModule->topLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(LengthCombo, marginsModule->topUnit, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, marginsModule->bottomLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(LengthCombo, marginsModule->bottomUnit, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, marginsModule->innerLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(LengthCombo, marginsModule->innerUnit, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, marginsModule->outerLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(LengthCombo, marginsModule->outerUnit, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, marginsModule->headheightLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(LengthCombo, marginsModule->headheightUnit, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, marginsModule->headsepLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(LengthCombo, marginsModule->headsepUnit, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, marginsModule->footskipLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(LengthCombo, marginsModule->footskipUnit, activated, int, 
				QDocumentDialog, this, change_adaptor, void);

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
	Q_CONNECT_1(QCheckBox, langModule->defaultencodingCB, toggled, bool, 
				QLabel, langModule->encodingL, setDisabled, bool);
	
	Q_CONNECT_1(QCheckBox, langModule->defaultencodingCB, toggled, bool,
				QComboBox, langModule->encodingCO, setDisabled, bool);

	// language & quote
	Q_CONNECT_1(QComboBox, langModule->languageCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QCheckBox, langModule->defaultencodingCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QComboBox, langModule->encodingCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QComboBox, langModule->quoteStyleCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
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
	Q_CONNECT_1(QSlider, numberingModule->depthSL, valueChanged, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QSlider, numberingModule->tocSL, valueChanged, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QSlider, numberingModule->depthSL, valueChanged, int, 
				QDocumentDialog, this, updateNumbering, void);
	
	Q_CONNECT_1(QSlider, numberingModule->tocSL, valueChanged, int, 
				QDocumentDialog, this, updateNumbering, void);

	numberingModule->tocTW->setColumnCount(3);
	numberingModule->tocTW->headerItem()->setText(0, qt_("Example"));
	numberingModule->tocTW->headerItem()->setText(1, qt_("Numbered"));
	numberingModule->tocTW->headerItem()->setText(2, qt_("Appears in TOC"));


	biblioModule = new UiWidget<Ui::BiblioUi>;
	Q_CONNECT_1(QRadioButton, biblioModule->citeNatbibRB,  toggled, bool, 
				QLabel, biblioModule->citationStyleL, setEnabled, bool);
	
	Q_CONNECT_1(QRadioButton, biblioModule->citeNatbibRB,  toggled, bool, 
				QComboBox, biblioModule->citeStyleCO, setEnabled, bool);
	// biblio
	Q_CONNECT_1(QRadioButton, biblioModule->citeDefaultRB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QRadioButton, biblioModule->citeNatbibRB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QComboBox, biblioModule->citeStyleCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QRadioButton, biblioModule->citeJurabibRB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QCheckBox, biblioModule->bibtopicCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	// biblio
	biblioModule->citeStyleCO->addItem(qt_("Author-year"));
	biblioModule->citeStyleCO->addItem(qt_("Numerical"));
	biblioModule->citeStyleCO->setCurrentIndex(0);



	mathsModule = new UiWidget<Ui::MathsUi>;
	Q_CONNECT_1(QCheckBox, mathsModule->amsautoCB, toggled, bool, 
				QCheckBox, mathsModule->amsCB, setDisabled, bool);
	
	Q_CONNECT_1(QCheckBox, mathsModule->esintautoCB, toggled, bool, 
				QCheckBox, mathsModule->esintCB, setDisabled, bool);
	// maths
	Q_CONNECT_1(QCheckBox, mathsModule->amsCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QCheckBox, mathsModule->amsautoCB, toggled, bool,
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QCheckBox, mathsModule->esintCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QCheckBox, mathsModule->esintautoCB, toggled, bool, 
				QDocumentDialog, this, change_adaptor, void);

	latexModule = new UiWidget<Ui::LaTeXUi>;
	// latex class
	Q_CONNECT_1(QComboBox, latexModule->classCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QLineEdit, latexModule->optionsLE, textChanged, const QString&, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QComboBox, latexModule->psdriverCO, activated, int, 
				QDocumentDialog, this, change_adaptor, void);
	
	Q_CONNECT_1(QComboBox, latexModule->classCO, activated, int, 
				QDocumentDialog, this, classChanged, void);
	// packages
	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = tex_graphics[n];
		latexModule->psdriverCO->addItem(enc);
	}
	// latex
	for (LyXTextClassList::const_iterator cit = textclasslist.begin();
	     cit != textclasslist.end(); ++cit) {
		if (cit->isTeXClassAvailable()) {
			latexModule->classCO->addItem(toqstr(cit->description()));
		} else {
			docstring item =
				bformat(_("Unavailable: %1$s"), lyx::from_utf8(cit->description()));
			latexModule->classCO->addItem(toqstr(item));
		}
	}

	// branches
	branchesModule = new QBranches;
	Q_CONNECT_0(QBranches, branchesModule, changed,
				QDocumentDialog, this, change_adaptor);

	// preamble
	preambleModule = new UiWidget<Ui::PreambleUi>;
	Q_CONNECT_0(QTextEdit, preambleModule->preambleTE, textChanged, 
				QDocumentDialog, this, change_adaptor);


	// bullets
	bulletsModule = new BulletsModule;
	Q_CONNECT_0(BulletsModule, bulletsModule, changed, 
				QDocumentDialog, this, change_adaptor);


	// float
	floatModule = new FloatPlacement;
	Q_CONNECT_0(FloatPlacement, floatModule, changed, 
				QDocumentDialog, this, change_adaptor);

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
#if QT_VERSION >= 0x040200
	docPS->updateGeometry();
#endif
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
	fontModule->fontsizeCO->addItem(qt_("default"));

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
	pageLayoutModule->pagestyleCO->clear();
	pageLayoutModule->pagestyleCO->addItem("default");

	for (int n=0; !token(items,'|',n).empty(); ++n)
		pageLayoutModule->pagestyleCO->
			addItem(toqstr(token(items,'|',n)));

	for (int n = 0; n<pageLayoutModule->pagestyleCO->count(); ++n) {
		if (fromqstr(pageLayoutModule->pagestyleCO->itemText(n))==sel) {
			pageLayoutModule->pagestyleCO->setCurrentIndex(n);
			break;
		}
	}
}


void QDocumentDialog::classChanged()
{
	ControlDocument & cntrl = form_->controller();
	BufferParams & params = cntrl.params();

	lyx::textclass_type const tc = latexModule->classCO->currentIndex();

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
	LyXTextClass const & tclass =
		form_->controller().params().getLyXTextClass();

	numberingModule->tocTW->setUpdatesEnabled(false);
	numberingModule->tocTW->clear();

	int const depth = numberingModule->depthSL->value();
	int const toc = numberingModule->tocSL->value();
	QString const no = qt_("No");
	QString const yes = qt_("Yes");
	LyXTextClass::const_iterator end = tclass.end();
	LyXTextClass::const_iterator cit = tclass.begin();
	QTreeWidgetItem * item = 0;
	for ( ; cit != end ; ++cit) {
		int const toclevel = (*cit)->toclevel;
		if (toclevel != LyXLayout::NOT_IN_TOC 
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
	params.preamble =
		fromqstr(preambleModule->preambleTE->document()->toPlainText());

	// biblio
	params.cite_engine = biblio::ENGINE_BASIC;

	if (biblioModule->citeNatbibRB->isChecked()) {
		bool const use_numerical_citations =
			biblioModule->citeStyleCO->currentIndex();
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
	params.language = languages.getLanguage(lang_[pos]);

	// numbering
	if (params.getLyXTextClass().hasTocLevels()) {
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
		fromqstr(latexModule->psdriverCO->currentText());

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

	params.pagestyle =
		fromqstr(pageLayoutModule->pagestyleCO->currentText());

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
	preambleModule->preambleTE->document()->setPlainText(preamble);

	// biblio
	biblioModule->citeDefaultRB->setChecked(
		params.cite_engine == biblio::ENGINE_BASIC);

	biblioModule->citeNatbibRB->setChecked(
		params.cite_engine == biblio::ENGINE_NATBIB_NUMERICAL ||
		params.cite_engine == biblio::ENGINE_NATBIB_AUTHORYEAR);

	biblioModule->citeStyleCO->setCurrentIndex(
		params.cite_engine == biblio::ENGINE_NATBIB_NUMERICAL);

	biblioModule->citeJurabibRB->setChecked(
		params.cite_engine == biblio::ENGINE_JURABIB);

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
	QString text = toqstr(params.graphicsDriver);
	int nitem = latexModule->psdriverCO->count();
	for (int n = 0; n < nitem ; ++n) {
		QString enc = tex_graphics[n];
		if (enc == text) {
			latexModule->psdriverCO->setCurrentIndex(n);
		}
	}


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

#include "QDocumentDialog_moc.cpp"
