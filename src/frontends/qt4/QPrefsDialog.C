/**
 * \file QPrefsDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include "debug.h"
#include "qt_helpers.h"

#include "lcolorcache.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "debug.h"
#include "session.h"
#include "LColor.h"
#include "lyxfont.h"

#include "support/lstrings.h"
#include "support/os.h"

#include "controllers/ControlPrefs.h"
#include "controllers/frnt_lang.h"
#include "controllers/helper_funcs.h"

#include "frontends/Alert.h"
#include "frontends/lyx_gui.h"

#include "QPrefsDialog.h"
#include "QPrefs.h"

#include "panelstack.h"
#include "qfontexample.h"

#include "ui/QPrefAsciiUi.h"
#include "ui/QPrefDateUi.h"
#include "ui/QPrefKeyboardUi.h"
#include "ui/QPrefLatexUi.h"
#include "ui/QPrefScreenFontsUi.h"
#include "ui/QPrefColorsUi.h"
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#include "ui/QPrefCygwinPathUi.h"
#endif
#include "ui/QPrefDisplayUi.h"
#include "ui/QPrefPathsUi.h"
#include "ui/QPrefSpellcheckerUi.h"
#include "ui/QPrefConvertersUi.h"
#include "ui/QPrefCopiersUi.h"
#include "ui/QPrefFileformatsUi.h"
#include "ui/QPrefLanguageUi.h"
#include "ui/QPrefPrinterUi.h"
#include "ui/QPrefUi.h"
#include "ui/QPrefIdentityUi.h"

#include "gettext.h"
#include "LColor.h"
#include "lcolorcache.h"

#include "controllers/ControlPrefs.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QFontDatabase>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QValidator>
#include <QCloseEvent>

#include <boost/tuple/tuple.hpp>
#include <iomanip>
#include <sstream>

using lyx::support::compare_no_case;

using std::distance;
using std::endl;
using std::setfill;
using std::setw;
using std::string;
using std::ostringstream;
using std::pair;
using std::vector;


namespace lyx {
namespace frontend {

QPrefsDialog::QPrefsDialog(QPrefs * form)
	: form_(form)
{
	setupUi(this);
	QDialog::setModal(true);

	connect(savePB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));



	asciiModule = new UiWidget<Ui::QPrefAsciiUi>;
	connect(asciiModule->asciiLinelengthSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(asciiModule->asciiRoffED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));


	dateModule = new UiWidget<Ui::QPrefDateUi>;
	connect(dateModule->DateED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));



	keyboardModule = new UiWidget<Ui::QPrefKeyboardUi>;
	connect( keyboardModule->keymapCB, SIGNAL( toggled(bool) ),
		 keyboardModule->firstKeymapLA, SLOT( setEnabled(bool) ) );
	connect( keyboardModule->keymapCB, SIGNAL( toggled(bool) ),
		keyboardModule->secondKeymapLA, SLOT( setEnabled(bool) ) );
	connect( keyboardModule->keymapCB, SIGNAL( toggled(bool) ),
		keyboardModule->firstKeymapED, SLOT( setEnabled(bool) ) );
	connect( keyboardModule->keymapCB, SIGNAL( toggled(bool) ),
		keyboardModule->secondKeymapED, SLOT( setEnabled(bool) ) );
	connect( keyboardModule->keymapCB, SIGNAL( toggled(bool) ),
		keyboardModule->firstKeymapPB, SLOT( setEnabled(bool) ) );
	connect( keyboardModule->keymapCB, SIGNAL( toggled(bool) ),
		keyboardModule->secondKeymapPB, SLOT( setEnabled(bool) ) );
	connect(keyboardModule->firstKeymapPB, SIGNAL(clicked()), this, SLOT(select_keymap1()));
	connect(keyboardModule->secondKeymapPB, SIGNAL(clicked()), this, SLOT(select_keymap2()));
	connect(keyboardModule->keymapCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(keyboardModule->firstKeymapED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(keyboardModule->secondKeymapED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));



	latexModule = new UiWidget<Ui::QPrefLatexUi>;
	connect(latexModule->latexEncodingED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexChecktexED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexBibtexED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexIndexED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexAutoresetCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(latexModule->latexDviPaperED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexPaperSizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));




	screenfontsModule = new UiWidget<Ui::QPrefScreenFontsUi>;
	connect(screenfontsModule->screenRomanCO, SIGNAL(activated(const QString&)), this, SLOT(select_roman(const QString&)));
	connect(screenfontsModule->screenSansCO, SIGNAL(activated(const QString&)), this, SLOT(select_sans(const QString&)));
	connect(screenfontsModule->screenTypewriterCO, SIGNAL(activated(const QString&)), this, SLOT(select_typewriter(const QString&)));

	QFontDatabase fontdb;
	QStringList families(fontdb.families());
	for (QStringList::Iterator it = families.begin(); it != families.end(); ++it) {
		screenfontsModule->screenRomanCO->insertItem(*it);
		screenfontsModule->screenSansCO->insertItem(*it);
		screenfontsModule->screenTypewriterCO->insertItem(*it);
	}
	connect(screenfontsModule->screenRomanCO, SIGNAL(activated(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenSansCO, SIGNAL(activated(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenTypewriterCO, SIGNAL(activated(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenZoomSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenDpiSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenTinyED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenSmallestED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenSmallerED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenSmallED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenNormalED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenLargeED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenLargerED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenLargestED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenHugeED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenHugerED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));

	screenfontsModule->screenTinyED->setValidator(new QDoubleValidator(
		screenfontsModule->screenTinyED));
	screenfontsModule->screenSmallestED->setValidator(new QDoubleValidator(
		screenfontsModule->screenSmallestED));
	screenfontsModule->screenSmallerED->setValidator(new QDoubleValidator(
		screenfontsModule->screenSmallerED));
	screenfontsModule->screenSmallED->setValidator(new QDoubleValidator(
		screenfontsModule->screenSmallED));
	screenfontsModule->screenNormalED->setValidator(new QDoubleValidator(
		screenfontsModule->screenNormalED));
	screenfontsModule->screenLargeED->setValidator(new QDoubleValidator(
		screenfontsModule->screenLargeED));
	screenfontsModule->screenLargerED->setValidator(new QDoubleValidator(
		screenfontsModule->screenLargerED));
	screenfontsModule->screenLargestED->setValidator(new QDoubleValidator(
		screenfontsModule->screenLargestED));
	screenfontsModule->screenHugeED->setValidator(new QDoubleValidator(
		screenfontsModule->screenHugeED));
	screenfontsModule->screenHugerED->setValidator(new QDoubleValidator(
		screenfontsModule->screenHugerED));

	// FIXME: put in controller
	colorsModule = new UiWidget<Ui::QPrefColorsUi>;
	for (int i = 0; i < LColor::ignore; ++i) {
		LColor::color lc = static_cast<LColor::color>(i);
		if (lc == LColor::none
			|| lc == LColor::black
			|| lc == LColor::white
			|| lc == LColor::red
			|| lc == LColor::green
			|| lc == LColor::blue
			|| lc == LColor::cyan
			|| lc == LColor::magenta
			|| lc == LColor::yellow
			|| lc == LColor::inherit
			|| lc == LColor::ignore) continue;

		lcolors_.push_back(lc);
		QColor color = QColor(lcolorcache.get(lc));
		prefcolors_.push_back(color.name());
		QPixmap coloritem(32, 32);
		coloritem.fill(color);
		QListWidgetItem * newItem = new QListWidgetItem(QIcon(coloritem),
			toqstr(lcolor.getGUIName(lc)), colorsModule->lyxObjectsLW);
	}
	newcolors_ = prefcolors_;

	connect(colorsModule->colorChangePB, SIGNAL(clicked()), 
		this, SLOT(change_color()));
	connect(colorsModule->lyxObjectsLW, SIGNAL(itemActivated(QListWidgetItem*)), 
		this, SLOT(change_color()));




#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	cygwinpathModule = new UiWidget<Ui::QPrefCygwinPathUi>;
	connect(cygwinpathModule->pathCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
#endif



	displayModule = new UiWidget<Ui::QPrefDisplayUi>;
	connect(displayModule->instantPreviewCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(displayModule->displayGraphicsCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));



	pathsModule = new UiWidget<Ui::QPrefPathsUi>;
	connect(pathsModule->templateDirPB, SIGNAL(clicked()), this, SLOT(select_templatedir()));
	connect(pathsModule->tempDirPB, SIGNAL(clicked()), this, SLOT(select_tempdir()));
	connect(pathsModule->backupDirPB, SIGNAL(clicked()), this, SLOT(select_backupdir()));
	connect(pathsModule->workingDirPB, SIGNAL(clicked()), this, SLOT(select_workingdir()));
	connect(pathsModule->lyxserverDirPB, SIGNAL(clicked()), this, SLOT(select_lyxpipe()));
	connect(pathsModule->workingDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->templateDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->backupDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->tempDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->lyxserverDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->pathPrefixED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));



	spellcheckerModule = new UiWidget<Ui::QPrefSpellcheckerUi>;
	connect(spellcheckerModule->persDictionaryPB, SIGNAL(clicked()), this, SLOT(select_dict()));
#if defined (USE_ISPELL)
	connect(spellcheckerModule->spellCommandCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
#else
	spellcheckerModule->spellCommandCO->setEnabled(false);
#endif
	connect(spellcheckerModule->altLanguageED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(spellcheckerModule->escapeCharactersED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(spellcheckerModule->persDictionaryED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(spellcheckerModule->compoundWordCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(spellcheckerModule->inputEncodingCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	spellcheckerModule->spellCommandCO->insertItem(qt_("ispell"));
	spellcheckerModule->spellCommandCO->insertItem(qt_("aspell"));
	spellcheckerModule->spellCommandCO->insertItem(qt_("hspell"));
#ifdef USE_PSPELL
	spellcheckerModule->spellCommandCO->insertItem(qt_("pspell (library)"));
#else
#ifdef USE_ASPELL
	spellcheckerModule->spellCommandCO->insertItem(qt_("aspell (library)"));
#endif
#endif



	convertersModule = new UiWidget<Ui::QPrefConvertersUi>;
	connect(convertersModule->converterNewPB, SIGNAL(clicked()), 
		this, SLOT(new_converter()));
	connect(convertersModule->converterRemovePB, SIGNAL(clicked()), 
		this, SLOT(remove_converter()));
	connect(convertersModule->converterModifyPB, SIGNAL(clicked()), 
		this, SLOT(modify_converter()));
	connect(convertersModule->convertersLW, SIGNAL(currentRowChanged(int)), 
		this, SLOT(switch_converter(int)));
	connect(convertersModule->converterFromCO, SIGNAL(activated(const QString&)), 
		this, SLOT(converter_changed()));
	connect(convertersModule->converterToCO, SIGNAL(activated(const QString&)), 
		this, SLOT(converter_changed()));
	connect(convertersModule->converterED, SIGNAL(textChanged(const QString&)), 
		this, SLOT(converter_changed()));
	connect(convertersModule->converterFlagED, SIGNAL(textChanged(const QString&)), 
		this, SLOT(converter_changed()));
	connect(convertersModule->converterNewPB, SIGNAL(clicked()), 
		this, SLOT(change_adaptor()));
	connect(convertersModule->converterRemovePB, SIGNAL(clicked()), 
		this, SLOT(change_adaptor()));
	connect(convertersModule->converterModifyPB, SIGNAL(clicked()), 
		this, SLOT(change_adaptor()));



	copiersModule = new UiWidget<Ui::QPrefCopiersUi>;
	connect(copiersModule->copierNewPB, SIGNAL(clicked()), this, SLOT(new_copier()));
	connect(copiersModule->copierRemovePB, SIGNAL(clicked()), this, SLOT(remove_copier()));
	connect(copiersModule->copierModifyPB, SIGNAL(clicked()), this, SLOT(modify_copier()));
	connect(copiersModule->AllCopiersLW, SIGNAL(currentRowChanged(int)), 
		this, SLOT(switch_copierLB(int)));
	connect(copiersModule->copierFormatCO, SIGNAL(activated(int)), this, SLOT(switch_copierCO(int)));
	connect(copiersModule->copierNewPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(copiersModule->copierRemovePB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(copiersModule->copierModifyPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(copiersModule->copierFormatCO, SIGNAL(activated(const QString&)), this, SLOT(copiers_changed()));
	connect(copiersModule->copierED, SIGNAL(textChanged(const QString&)), this, SLOT(copiers_changed()));




	fileformatsModule = new UiWidget<Ui::QPrefFileformatsUi>;
	connect(fileformatsModule->formatNewPB, SIGNAL(clicked()), this, SLOT(new_format()));
	connect(fileformatsModule->formatRemovePB, SIGNAL(clicked()), this, SLOT(remove_format()));
	connect(fileformatsModule->formatModifyPB, SIGNAL(clicked()), this, SLOT(modify_format()));
	connect(fileformatsModule->formatsLW, SIGNAL(currentRowChanged(int)), 
		this, SLOT(switch_format(int)));
	connect(fileformatsModule->formatED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->guiNameED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->shortcutED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->extensionED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->viewerED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->editorED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->formatNewPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(fileformatsModule->formatRemovePB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(fileformatsModule->formatModifyPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));




	languageModule = new UiWidget<Ui::QPrefLanguageUi>;
	connect(languageModule->rtlCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(languageModule->markForeignCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(languageModule->autoBeginCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(languageModule->autoEndCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(languageModule->useBabelCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(languageModule->globalCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(languageModule->languagePackageED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(languageModule->startCommandED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(languageModule->endCommandED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(languageModule->defaultLanguageCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	languageModule->defaultLanguageCO->clear();

	// store the lang identifiers for later
	using lyx::frontend::LanguagePair;
	std::vector<LanguagePair> const langs =
		lyx::frontend::getLanguageData(false);
	lang_ = getSecond(langs);

	std::vector<LanguagePair>::const_iterator lit  = langs.begin();
	std::vector<LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		languageModule->defaultLanguageCO->insertItem(toqstr(lit->first));
	}





	printerModule = new UiWidget<Ui::QPrefPrinterUi>;
	connect(printerModule->printerAdaptCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(printerModule->printerCommandED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerNameED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerPageRangeED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerCopiesED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerReverseED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerToPrinterED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerExtensionED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerSpoolCommandED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerPaperTypeED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerEvenED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerOddED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerCollatedED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerLandscapeED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerToFileED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerExtraED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerSpoolPrefixED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(printerModule->printerPaperSizeED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));




	uiModule = new UiWidget<Ui::QPrefUi>;
	connect(uiModule->autoSaveCB, SIGNAL( toggled(bool) ), uiModule->autoSaveLA, SLOT( setEnabled(bool) ) );
	connect(uiModule->autoSaveCB, SIGNAL( toggled(bool) ), uiModule->autoSaveSB, SLOT( setEnabled(bool) ) );
	connect(uiModule->autoSaveCB, SIGNAL( toggled(bool) ), uiModule->TextLabel1, SLOT( setEnabled(bool) ) );
	connect(uiModule->uiFilePB, SIGNAL(clicked()), this, SLOT(select_ui()));
	connect(uiModule->bindFilePB, SIGNAL(clicked()), this, SLOT(select_bind()));
	connect(uiModule->uiFileED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(uiModule->bindFileED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(uiModule->restoreCursorCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(uiModule->loadSessionCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(uiModule->cursorFollowsCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(uiModule->autoSaveSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(uiModule->autoSaveCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(uiModule->lastfilesSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	uiModule->lastfilesSB->setMaxValue(maxlastfiles);




	identityModule = new UiWidget<Ui::QPrefIdentityUi>;
	connect(identityModule->nameED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(identityModule->emailED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));


	string const laf = _("Look and feel");
	prefsPS->addCategory(laf);
	prefsPS->addPanel(uiModule, _("User interface"), laf);
	prefsPS->addPanel(screenfontsModule, _("Screen fonts"), laf);
	prefsPS->addPanel(colorsModule, _("Colors"), laf);
	prefsPS->addPanel(displayModule, _("Graphics"), laf);
	prefsPS->addPanel(keyboardModule, _("Keyboard"), laf);

	string const ls = _("Language settings");
	prefsPS->addCategory(ls);
	prefsPS->addPanel(languageModule, _("Language"), ls);
	prefsPS->addPanel(spellcheckerModule, _("Spellchecker"), ls);

	string const op = _("Outputs");
	prefsPS->addCategory(op);
	prefsPS->addPanel(asciiModule, _("Plain text"), op);
	prefsPS->addPanel(dateModule, _("Date format"), op);
	prefsPS->addPanel(latexModule, _("LaTeX"), op);
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	prefsPS->addPanel(cygwinpathModule, _("Paths"), op);
#endif
	prefsPS->addPanel(printerModule, _("Printer"), op);

	prefsPS->addPanel(identityModule, _("Identity"));
	prefsPS->addPanel(pathsModule, _("Paths"));
	prefsPS->addPanel(fileformatsModule, _("File formats"));
	prefsPS->addPanel(convertersModule, _("Converters"));
	prefsPS->addPanel(copiersModule, _("Copiers"));

	prefsPS->setCurrentPanel(_("User interface"));

	form_->bcview().setOK(savePB);
	form_->bcview().setApply(applyPB);
	form_->bcview().setCancel(closePB);
	form_->bcview().setRestore(restorePB);
}




QPrefsDialog::~QPrefsDialog()
{
}


void QPrefsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QPrefsDialog::change_adaptor()
{
	form_->changed();
}


void QPrefsDialog::updateConverters()
{
	Ui::QPrefConvertersUi* convertmod(convertersModule);

	// save current selection
	QString current = convertmod->converterFromCO->currentText()
		+ " -> " + convertmod->converterToCO->currentText();

	convertmod->converterFromCO->clear();
	convertmod->converterToCO->clear();

	Formats::const_iterator cit = form_->formats().begin();
	Formats::const_iterator end = form_->formats().end();
	for (; cit != end; ++cit) {
		convertmod->converterFromCO->insertItem(toqstr(cit->prettyname()));
		convertmod->converterToCO->insertItem(toqstr(cit->prettyname()));
	}

	convertmod->convertersLW->clear();

	Converters::const_iterator ccit = form_->converters().begin();
	Converters::const_iterator cend = form_->converters().end();
	for (; ccit != cend; ++ccit) {
		std::string const name = ccit->From->prettyname() + " -> "
			+ ccit->To->prettyname();
		convertmod->convertersLW->addItem(toqstr(name));
	}
	convertmod->convertersLW->sortItems(Qt::AscendingOrder);

	// restore selection
	if (!current.isEmpty()) {
		QList<QListWidgetItem *> const item = 
			convertmod->convertersLW->findItems(current, Qt::MatchExactly);
		if (item.size()>0)
			convertmod->convertersLW->setCurrentItem(item.at(0));
	}
	// select first element if restoring failed
	if (convertmod->convertersLW->currentRow() == -1)
		convertmod->convertersLW->setCurrentRow(0);

	updateConverterButtons();
}


void QPrefsDialog::switch_converter(int nr)
{
	if (nr<0)
		return;

	Converter const & c(form_->converters().get(nr));
	convertersModule->converterFromCO->setCurrentIndex(form_->formats().getNumber(c.from));
	convertersModule->converterToCO->setCurrentIndex(form_->formats().getNumber(c.to));
	convertersModule->converterED->setText(toqstr(c.command));
	convertersModule->converterFlagED->setText(toqstr(c.flags));

	updateConverterButtons();
}


void QPrefsDialog::converter_changed()
{
	updateConverterButtons();
}


void QPrefsDialog::updateConverterButtons()
{
	Format const & from(form_->formats().get(
		convertersModule->converterFromCO->currentItem()));
	Format const & to(form_->formats().get(
		convertersModule->converterToCO->currentItem()));
	int const sel = form_->converters().getNumber(from.name(), to.name());
	bool const known = !(sel < 0);
	bool const valid = !(convertersModule->converterED->text().isEmpty()
		|| from.name() == to.name());

	Converter const & c(form_->converters().get(
		convertersModule->convertersLW->currentRow()));
	string const old_command = c.command;
	string const old_flag = c.flags;
	string const new_command(fromqstr(convertersModule->converterED->text()));
	string const new_flag(fromqstr(convertersModule->converterFlagED->text()));

	bool modified = ((old_command != new_command) || (old_flag != new_flag));

	convertersModule->converterModifyPB->setEnabled(valid && known && modified);
	convertersModule->converterNewPB->setEnabled(valid && !known);
	convertersModule->converterRemovePB->setEnabled(known);
}


// FIXME: user must
// specify unique from/to or it doesn't appear. This is really bad UI
void QPrefsDialog::new_converter()
{
	Format const & from(form_->formats().get(convertersModule->converterFromCO->currentItem()));
	Format const & to(form_->formats().get(convertersModule->converterToCO->currentItem()));
	string const command(fromqstr(convertersModule->converterED->text()));
	string const flags(fromqstr(convertersModule->converterFlagED->text()));

	Converter const * old = form_->converters().getConverter(from.name(), to.name());
	form_->converters().add(from.name(), to.name(), command, flags);
	if (!old) {
		form_->converters().updateLast(form_->formats());
	}
	updateConverters();
	convertersModule->convertersLW->setCurrentRow(convertersModule->convertersLW->count() - 1);
}


void QPrefsDialog::modify_converter()
{
	QString const current_text =
		convertersModule->convertersLW->currentItem()->text();

	Format const & from(form_->formats().get(convertersModule->converterFromCO->currentItem()));
	Format const & to(form_->formats().get(convertersModule->converterToCO->currentItem()));
	string flags(fromqstr(convertersModule->converterFlagED->text()));
	string name(fromqstr(convertersModule->converterED->text()));

	Converter const * old = form_->converters().getConverter(from.name(), to.name());
	form_->converters().add(from.name(), to.name(), name, flags);
	if (!old) {
		form_->converters().updateLast(form_->formats());
	}
	updateConverters();

	QList<QListWidgetItem *> const item =
		convertersModule->convertersLW->findItems(current_text, Qt::MatchExactly);
	if (item.size()>0)
		convertersModule->convertersLW->setCurrentItem(item.at(0));
}


void QPrefsDialog::remove_converter()
{
	Format const & from(form_->formats().get(convertersModule->converterFromCO->currentItem()));
	Format const & to(form_->formats().get(convertersModule->converterToCO->currentItem()));
	form_->converters().erase(from.name(), to.name());
	updateConverters();
}


void QPrefsDialog::updateCopiers()
{
	// The choice widget
	// save current selection
	QString current = copiersModule->copierFormatCO->currentText();
	copiersModule->copierFormatCO->clear();

	for (Formats::const_iterator it = form_->formats().begin(),
		     end = form_->formats().end();
	     it != end; ++it) {
		copiersModule->copierFormatCO->insertItem(toqstr(it->prettyname()));
	}

	// The browser widget
	copiersModule->AllCopiersLW->clear();

	for (Movers::iterator it = form_->movers().begin(),
		     end = form_->movers().end();
	     it != end; ++it) {
		std::string const & command = it->second.command();
		if (command.empty())
			continue;
		QString const pretty = toqstr(form_->formats().prettyName(it->first));
		copiersModule->AllCopiersLW->addItem(pretty);
	}
	copiersModule->AllCopiersLW->sortItems(Qt::AscendingOrder);

	// restore selection
	if (!current.isEmpty()) {
		QList<QListWidgetItem *> item = 
			copiersModule->AllCopiersLW->findItems(current, Qt::MatchExactly);
		if (item.size()>0)
			copiersModule->AllCopiersLW->setCurrentItem(item.at(0));
	}
	// select first element if restoring failed
	if (copiersModule->AllCopiersLW->currentRow() == -1)
		copiersModule->AllCopiersLW->setCurrentRow(0);
}


namespace {

class SamePrettyName {
public:
	SamePrettyName(string const & n) : pretty_name_(n) {}

	bool operator()(::Format const & fmt) const {
		return fmt.prettyname() == pretty_name_;
	}

private:
	string const pretty_name_;
};


Format const * getFormat(std::string const & prettyname)
{
	Formats::const_iterator it = ::formats.begin();
	Formats::const_iterator const end = ::formats.end();
	it = std::find_if(it, end, SamePrettyName(prettyname));
	return it == end ? 0 : &*it;
}

} // namespace anon


void QPrefsDialog::switch_copierLB(int row)
{
	if (row<0)
		return;

	std::string const browser_text =
		fromqstr(copiersModule->AllCopiersLW->currentItem()->text());
	Format const * fmt = getFormat(browser_text);
	if (fmt == 0)
		return;

	QString const gui_name = toqstr(fmt->prettyname());
	QString const command = toqstr(form_->movers().command(fmt->name()));

	copiersModule->copierED->clear();
	int const combo_size = copiersModule->copierFormatCO->count();
	for (int i = 0; i < combo_size; ++i) {
		QString const text = copiersModule->copierFormatCO->text(i);
		if (text == gui_name) {
			copiersModule->copierFormatCO->setCurrentIndex(i);
			copiersModule->copierED->setText(command);
			break;
		}
	}
	updateCopierButtons();
}


void QPrefsDialog::switch_copierCO(int row)
{
	if (row<0)
		return;

	std::string const combo_text =
		fromqstr(copiersModule->copierFormatCO->currentText());
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	QString const command = toqstr(form_->movers().command(fmt->name()));
	copiersModule->copierED->setText(command);

	QListWidgetItem * const index = copiersModule->AllCopiersLW->currentItem();
	if (index >= 0)
		copiersModule->AllCopiersLW->setItemSelected(index, false);

	QString const gui_name = toqstr(fmt->prettyname());
	int const browser_size = copiersModule->AllCopiersLW->count();
	for (int i = 0; i < browser_size; ++i) {
		QString const text = copiersModule->AllCopiersLW->item(i)->text();
		if (text == gui_name) {
			QListWidgetItem * item = copiersModule->AllCopiersLW->item(i);
			copiersModule->AllCopiersLW->setItemSelected(item, true);
			break;
		}
	}
}


void QPrefsDialog::copiers_changed()
{
	updateCopierButtons();
}


void QPrefsDialog::updateCopierButtons()
{
	QString selected = copiersModule->copierFormatCO->currentText();

	bool known = false;
	for (unsigned int i = 0; i != copiersModule->AllCopiersLW->count(); i++) {
		if (copiersModule->AllCopiersLW->item(i)->text() == selected)
			known = true;
	}

	bool const valid = !copiersModule->copierED->text().isEmpty();

	Format const * fmt = getFormat(fromqstr(selected));
	string const old_command = form_->movers().command(fmt->name());
	string const new_command(fromqstr(copiersModule->copierED->text()));

	bool modified = (old_command != new_command);

	copiersModule->copierModifyPB->setEnabled(valid && known && modified);
	copiersModule->copierNewPB->setEnabled(valid && !known);
	copiersModule->copierRemovePB->setEnabled(known);
}


void QPrefsDialog::new_copier()
{
	std::string const combo_text =
		fromqstr(copiersModule->copierFormatCO->currentText());
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	string const command = fromqstr(copiersModule->copierED->text());
	if (command.empty())
		return;

	form_->movers().set(fmt->name(), command);

	updateCopiers();
	int const last = copiersModule->AllCopiersLW->count() - 1;
	copiersModule->AllCopiersLW->setCurrentRow(last);

	updateCopierButtons();
}


void QPrefsDialog::modify_copier()
{
	std::string const combo_text =
		fromqstr(copiersModule->copierFormatCO->currentText());
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	string const command = fromqstr(copiersModule->copierED->text());
	form_->movers().set(fmt->name(), command);

	updateCopiers();
	updateCopierButtons();
}


void QPrefsDialog::remove_copier()
{
	std::string const combo_text =
		fromqstr(copiersModule->copierFormatCO->currentText());
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	string const & fmt_name = fmt->name();
	form_->movers().set(fmt_name, string());

	updateCopiers();
	updateCopierButtons();
}


void QPrefsDialog::updateFormats()
{
	Ui::QPrefFileformatsUi * formatmod(fileformatsModule);

	// save current selection
	QString current = formatmod->guiNameED->text();

	formatmod->formatsLW->clear();

	Formats::const_iterator cit = form_->formats().begin();
	Formats::const_iterator end = form_->formats().end();
	for (; cit != end; ++cit) {
		formatmod->formatsLW->addItem(toqstr(cit->prettyname()));
	}
	formatmod->formatsLW->sortItems(Qt::AscendingOrder);

	// restore selection
	if (!current.isEmpty()) {
		QList<QListWidgetItem *>  item = formatmod->formatsLW->findItems(current, Qt::MatchExactly);
		if (item.size()>0)
			formatmod->formatsLW->setCurrentItem(item.at(0));
	}
	// select first element if restoring failed
	if (formatmod->formatsLW->currentRow() == -1)
		formatmod->formatsLW->setCurrentRow(0);
}


void QPrefsDialog::switch_format(int nr)
{
	if (nr<0)
		return;

	Format const & f(form_->formats().get(nr));
	fileformatsModule->formatED->setText(toqstr(f.name()));
	fileformatsModule->guiNameED->setText(toqstr(f.prettyname()));
	fileformatsModule->extensionED->setText(toqstr(f.extension()));
	fileformatsModule->shortcutED->setText(toqstr(f.shortcut()));
	fileformatsModule->viewerED->setText(toqstr(f.viewer()));
	fileformatsModule->editorED->setText(toqstr(f.editor()));
	fileformatsModule->formatRemovePB->setEnabled(
		!form_->converters().formatIsUsed(f.name()));

	updateFormatsButtons();
}


void QPrefsDialog::fileformat_changed()
{
	updateFormatsButtons();
}


void QPrefsDialog::updateFormatsButtons()
{
	QString const format = fileformatsModule->formatED->text();
	QString const gui_name = fileformatsModule->guiNameED->text();
	int const sel = form_->formats().getNumber(fromqstr(format));
	bool gui_name_known = false;
	int where = sel;
	for (unsigned int i = 0; i != fileformatsModule->formatsLW->count(); i++) {
		if (fileformatsModule->formatsLW->item(i)->text() == gui_name) {
			gui_name_known = true;
			where = i;
		}
	}

	// assure that a gui name cannot be chosen twice
	bool const known_otherwise = gui_name_known && (where != sel);

	bool const known = !(sel < 0);
	bool const valid = (!fileformatsModule->formatED->text().isEmpty()
		&& !fileformatsModule->guiNameED->text().isEmpty());

	Format const & f(form_->formats().get(
		fileformatsModule->formatsLW->currentRow()));
	string const old_pretty(f.prettyname());
	string const old_shortcut(f.shortcut());
	string const old_extension(f.extension());
	string const old_viewer(f.viewer());
	string const old_editor(f.editor());

	string const new_pretty(fromqstr(gui_name));
	string const new_shortcut(fromqstr(fileformatsModule->shortcutED->text()));
	string const new_extension(fromqstr(fileformatsModule->extensionED->text()));
	string const new_viewer(fromqstr(fileformatsModule->viewerED->text()));
	string const new_editor(fromqstr(fileformatsModule->editorED->text()));

	bool modified = ((old_pretty != new_pretty) || (old_shortcut != new_shortcut)
		|| (old_extension != new_extension) || (old_viewer != new_viewer)
		|| (old_editor != new_editor));

	fileformatsModule->formatModifyPB->setEnabled(
		valid && known && modified && !known_otherwise);
	fileformatsModule->formatNewPB->setEnabled(valid && !known && !gui_name_known);
	fileformatsModule->formatRemovePB->setEnabled(known);
}


void QPrefsDialog::new_format()
{
	string const name = fromqstr(fileformatsModule->formatED->text());
	string const prettyname = fromqstr(fileformatsModule->guiNameED->text());
	string const extension = fromqstr(fileformatsModule->extensionED->text());
	string const shortcut = fromqstr(fileformatsModule->shortcutED->text());
	string const viewer = fromqstr(fileformatsModule->viewerED->text());
	string const editor = fromqstr(fileformatsModule->editorED->text());

	form_->formats().add(name, extension, prettyname, shortcut, viewer, editor);
	form_->formats().sort();
	updateFormats();
	fileformatsModule->formatsLW->setCurrentRow(form_->formats().getNumber(name));
	form_->converters().update(form_->formats());

	updateConverters();
	updateFormatsButtons();
}


void QPrefsDialog::modify_format()
{
	int const current_item = fileformatsModule->formatsLW->currentRow();
	QString const current_text =
		fileformatsModule->formatsLW->currentItem()->text();

	Format const & oldformat(form_->formats().get(current_item));
	string const oldpretty(oldformat.prettyname());
	string const name(fromqstr(fileformatsModule->formatED->text()));
	form_->formats().erase(oldformat.name());

	string const prettyname = fromqstr(fileformatsModule->guiNameED->text());
	string const extension = fromqstr(fileformatsModule->extensionED->text());
	string const shortcut = fromqstr(fileformatsModule->shortcutED->text());
	string const viewer = fromqstr(fileformatsModule->viewerED->text());
	string const editor = fromqstr(fileformatsModule->editorED->text());

	form_->formats().add(name, extension, prettyname, shortcut, viewer, editor);
	form_->formats().sort();

	fileformatsModule->formatsLW->setUpdatesEnabled(false);
	updateFormats();
	fileformatsModule->formatsLW->setUpdatesEnabled(true);
	fileformatsModule->formatsLW->update();

	updateConverters();
	updateFormatsButtons();

	QList<QListWidgetItem *>  const item =
		fileformatsModule->formatsLW->findItems(current_text, Qt::MatchExactly);
	if (item.size()>0)
		fileformatsModule->formatsLW->setCurrentItem(item.at(0));
}


void QPrefsDialog::remove_format()
{
	int const nr(fileformatsModule->formatsLW->currentRow());
	if (nr < 0)
		return;
	string const current_text = form_->formats().get(nr).name();
	if (form_->converters().formatIsUsed(current_text)) {
		Alert::error(_("Format in use"),
				_("Cannot remove a Format used by a Converter. "
				      "Remove the converter first."));
		return;
	}
	form_->formats().erase(current_text);
	updateFormats();
	form_->converters().update(form_->formats());

	updateConverters();
	updateFormatsButtons();
}


void QPrefsDialog::change_color()
{
	int const row = colorsModule->lyxObjectsLW->currentRow();
	QString color = newcolors_[row];
	QColor c(QColorDialog::getColor(QColor(color), 
		qApp->focusWidget() ? qApp->focusWidget() : qApp->mainWidget()));

	if (c.name()!=color) {
		newcolors_[row] = c.name();
		QPixmap coloritem(32, 32);
		coloritem.fill(c);
		colorsModule->lyxObjectsLW->currentItem()->setIcon(QIcon(coloritem));
		change_adaptor();
	}
}


void QPrefsDialog::select_ui()
{
	string file(form_->controller().browseUI(fromqstr(uiModule->uiFileED->text())));
	if (!file.empty())
		uiModule->uiFileED->setText(toqstr(file));
}


void QPrefsDialog::select_bind()
{
	string file(form_->controller().browsebind(fromqstr(uiModule->bindFileED->text())));
	if (!file.empty())
		uiModule->bindFileED->setText(toqstr(file));
}


void QPrefsDialog::select_keymap1()
{
	string file(form_->controller().browsekbmap(fromqstr(keyboardModule->firstKeymapED->text())));
	if (!file.empty())
		keyboardModule->firstKeymapED->setText(toqstr(file));
}


void QPrefsDialog::select_keymap2()
{
	string file(form_->controller().browsekbmap(fromqstr(keyboardModule->secondKeymapED->text())));
	if (!file.empty())
		keyboardModule->secondKeymapED->setText(toqstr(file));
}


void QPrefsDialog::select_dict()
{
	string file(form_->controller().browsedict(fromqstr(spellcheckerModule->persDictionaryED->text())));
	if (!file.empty())
		spellcheckerModule->persDictionaryED->setText(toqstr(file));
}


// NB: the _() is OK here because it gets passed back and we toqstr() them

void QPrefsDialog::select_templatedir()
{
	string file(form_->controller().browsedir(fromqstr(pathsModule->templateDirED->text()), _("Select a document templates directory")));
	if (!file.empty())
		pathsModule->templateDirED->setText(toqstr(file));
}


void QPrefsDialog::select_tempdir()
{
	string file(form_->controller().browsedir(fromqstr(pathsModule->tempDirED->text()), _("Select a temporary directory")));
	if (!file.empty())
		pathsModule->tempDirED->setText(toqstr(file));
}


void QPrefsDialog::select_backupdir()
{
	string file(form_->controller().browsedir(fromqstr(pathsModule->backupDirED->text()), _("Select a backups directory")));
	if (!file.empty())
		pathsModule->backupDirED->setText(toqstr(file));
}


void QPrefsDialog::select_workingdir()
{
	string file(form_->controller().browsedir(fromqstr(pathsModule->workingDirED->text()), _("Select a document directory")));
	if (!file.empty())
		pathsModule->workingDirED->setText(toqstr(file));
}


void QPrefsDialog::select_lyxpipe()
{
	string file(form_->controller().browse(fromqstr(pathsModule->lyxserverDirED->text()), _("Give a filename for the LyX server pipe")));
	if (!file.empty())
		pathsModule->lyxserverDirED->setText(toqstr(file));
}


void QPrefsDialog::select_roman(const QString& name)
{
	screenfontsModule->screenRomanFE->set(QFont(name), name);
}


void QPrefsDialog::select_sans(const QString& name)
{
	screenfontsModule->screenSansFE->set(QFont(name), name);
}


void QPrefsDialog::select_typewriter(const QString& name)
{
	screenfontsModule->screenTypewriterFE->set(QFont(name), name);
}

namespace {

string const internal_path(QString const & input)
{
	return lyx::support::os::internal_path(fromqstr(input));
}

}

void QPrefsDialog::apply(LyXRC & rc) const
{
	// FIXME: remove rtl_support bool
	rc.rtl_support = languageModule->rtlCB->isChecked();
	rc.mark_foreign_language = languageModule->markForeignCB->isChecked();
	rc.language_auto_begin = languageModule->autoBeginCB->isChecked();
	rc.language_auto_end = languageModule->autoEndCB->isChecked();
	rc.language_use_babel = languageModule->useBabelCB->isChecked();
	rc.language_global_options = languageModule->globalCB->isChecked();
	rc.language_package = fromqstr(languageModule->languagePackageED->text());
	rc.language_command_begin = fromqstr(languageModule->startCommandED->text());
	rc.language_command_end = fromqstr(languageModule->endCommandED->text());
	rc.default_language = lang_[languageModule->defaultLanguageCO->currentItem()];


	rc.ui_file = internal_path(uiModule->uiFileED->text());
	rc.bind_file = internal_path(uiModule->bindFileED->text());
	rc.use_lastfilepos = uiModule->restoreCursorCB->isChecked();
	rc.load_session = uiModule->loadSessionCB->isChecked();
	rc.cursor_follows_scrollbar = uiModule->cursorFollowsCB->isChecked();
	rc.autosave = uiModule->autoSaveSB->value() * 60;
	rc.make_backup = uiModule->autoSaveCB->isChecked();
	rc.num_lastfiles = uiModule->lastfilesSB->value();


	// FIXME: can derive CB from the two EDs
	rc.use_kbmap = keyboardModule->keymapCB->isChecked();
	rc.primary_kbmap = internal_path(keyboardModule->firstKeymapED->text());
	rc.secondary_kbmap = internal_path(keyboardModule->secondKeymapED->text());


	rc.ascii_linelen = asciiModule->asciiLinelengthSB->value();
	rc.ascii_roff_command = fromqstr(asciiModule->asciiRoffED->text());


	rc.date_insert_format = fromqstr(dateModule->DateED->text());


#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	rc.cygwin_path_fix = cygwinpathModule->pathCB->isChecked();
#endif


	rc.fontenc = fromqstr(latexModule->latexEncodingED->text());
	rc.chktex_command = fromqstr(latexModule->latexChecktexED->text());
	rc.bibtex_command = fromqstr(latexModule->latexBibtexED->text());
	rc.index_command = fromqstr(latexModule->latexIndexED->text());
	rc.auto_reset_options = latexModule->latexAutoresetCB->isChecked();
	rc.view_dvi_paper_option = fromqstr(latexModule->latexDviPaperED->text());
	rc.default_papersize =
		form_->controller().toPaperSize(latexModule->latexPaperSizeCO->currentItem());


	switch (displayModule->instantPreviewCO->currentItem()) {
	case 0: rc.preview = LyXRC::PREVIEW_OFF; break;
	case 1:	rc.preview = LyXRC::PREVIEW_NO_MATH; break;
	case 2:	rc.preview = LyXRC::PREVIEW_ON;	break;
	}

	lyx::graphics::DisplayType dtype;
	switch (displayModule->displayGraphicsCO->currentItem()) {
	case 3:	dtype = lyx::graphics::NoDisplay; break;
	case 2:	dtype = lyx::graphics::ColorDisplay; break;
	case 1: dtype = lyx::graphics::GrayscaleDisplay;	break;
	case 0: dtype = lyx::graphics::MonochromeDisplay; break;
	default: dtype = lyx::graphics::GrayscaleDisplay;
	}
	rc.display_graphics = dtype;

#ifdef WITH_WARNINGS
#warning FIXME!! The graphics cache no longer has a changeDisplay method.
#endif
#if 0
	if (old_value != rc.display_graphics) {
		lyx::graphics::GCache & gc = lyx::graphics::GCache::get();
		gc.changeDisplay();
	}
#endif


	rc.document_path = internal_path(pathsModule->workingDirED->text());
	rc.template_path = internal_path(pathsModule->templateDirED->text());
	rc.backupdir_path = internal_path(pathsModule->backupDirED->text());
	rc.tempdir_path = internal_path(pathsModule->tempDirED->text());
	rc.path_prefix = fromqstr(pathsModule->pathPrefixED->text());
	// FIXME: should be a checkbox only
	rc.lyxpipes = internal_path(pathsModule->lyxserverDirED->text());


	switch (spellcheckerModule->spellCommandCO->currentItem()) {
		case 0:
		case 1:
		case 2:
			rc.use_spell_lib = false;
			rc.isp_command = fromqstr(spellcheckerModule->spellCommandCO->currentText());
			break;
		case 3:
			rc.use_spell_lib = true;
			break;
	}

	// FIXME: remove isp_use_alt_lang
	rc.isp_alt_lang = fromqstr(spellcheckerModule->altLanguageED->text());
	rc.isp_use_alt_lang = !rc.isp_alt_lang.empty();
	// FIXME: remove isp_use_esc_chars
	rc.isp_esc_chars = fromqstr(spellcheckerModule->escapeCharactersED->text());
	rc.isp_use_esc_chars = !rc.isp_esc_chars.empty();
	// FIXME: remove isp_use_pers_dict
	rc.isp_pers_dict = internal_path(spellcheckerModule->persDictionaryED->text());
	rc.isp_use_pers_dict = !rc.isp_pers_dict.empty();
	rc.isp_accept_compound = spellcheckerModule->compoundWordCB->isChecked();
	rc.isp_use_input_encoding = spellcheckerModule->inputEncodingCB->isChecked();



	rc.print_adapt_output = printerModule->printerAdaptCB->isChecked();
	rc.print_command = fromqstr(printerModule->printerCommandED->text());
	rc.printer = fromqstr(printerModule->printerNameED->text());

	rc.print_pagerange_flag = fromqstr(printerModule->printerPageRangeED->text());
	rc.print_copies_flag = fromqstr(printerModule->printerCopiesED->text());
	rc.print_reverse_flag = fromqstr(printerModule->printerReverseED->text());
	rc.print_to_printer = fromqstr(printerModule->printerToPrinterED->text());
	rc.print_file_extension = fromqstr(printerModule->printerExtensionED->text());
	rc.print_spool_command = fromqstr(printerModule->printerSpoolCommandED->text());
	rc.print_paper_flag = fromqstr(printerModule->printerPaperTypeED->text());
	rc.print_evenpage_flag = fromqstr(printerModule->printerEvenED->text());
	rc.print_oddpage_flag = fromqstr(printerModule->printerOddED->text());
	rc.print_collcopies_flag = fromqstr(printerModule->printerCollatedED->text());
	rc.print_landscape_flag = fromqstr(printerModule->printerLandscapeED->text());
	rc.print_to_file = internal_path(printerModule->printerToFileED->text());
	rc.print_extra_options = fromqstr(printerModule->printerExtraED->text());
	rc.print_spool_printerprefix = fromqstr(printerModule->printerSpoolPrefixED->text());
	rc.print_paper_dimension_flag = fromqstr(printerModule->printerPaperSizeED->text());



	rc.user_name = fromqstr(identityModule->nameED->text());
	rc.user_email = fromqstr(identityModule->emailED->text());



	LyXRC const oldrc(rc);

	boost::tie(rc.roman_font_name, rc.roman_font_foundry)
		= parseFontName(fromqstr(screenfontsModule->screenRomanCO->currentText()));
	boost::tie(rc.sans_font_name, rc.sans_font_foundry) =
		parseFontName(fromqstr(screenfontsModule->screenSansCO->currentText()));
	boost::tie(rc.typewriter_font_name, rc.typewriter_font_foundry) =
		parseFontName(fromqstr(screenfontsModule->screenTypewriterCO->currentText()));

	rc.zoom = screenfontsModule->screenZoomSB->value();
	rc.dpi = screenfontsModule->screenDpiSB->value();
	rc.font_sizes[LyXFont::SIZE_TINY] = fromqstr(screenfontsModule->screenTinyED->text());
	rc.font_sizes[LyXFont::SIZE_SCRIPT] = fromqstr(screenfontsModule->screenSmallestED->text());
	rc.font_sizes[LyXFont::SIZE_FOOTNOTE] = fromqstr(screenfontsModule->screenSmallerED->text());
	rc.font_sizes[LyXFont::SIZE_SMALL] = fromqstr(screenfontsModule->screenSmallED->text());
	rc.font_sizes[LyXFont::SIZE_NORMAL] = fromqstr(screenfontsModule->screenNormalED->text());
	rc.font_sizes[LyXFont::SIZE_LARGE] = fromqstr(screenfontsModule->screenLargeED->text());
	rc.font_sizes[LyXFont::SIZE_LARGER] = fromqstr(screenfontsModule->screenLargerED->text());
	rc.font_sizes[LyXFont::SIZE_LARGEST] = fromqstr(screenfontsModule->screenLargestED->text());
	rc.font_sizes[LyXFont::SIZE_HUGE] = fromqstr(screenfontsModule->screenHugeED->text());
	rc.font_sizes[LyXFont::SIZE_HUGER] = fromqstr(screenfontsModule->screenHugerED->text());

	if (rc.font_sizes != oldrc.font_sizes
		|| rc.roman_font_name != oldrc.roman_font_name
		|| rc.sans_font_name != oldrc.sans_font_name
		|| rc.typewriter_font_name != oldrc.typewriter_font_name
		|| rc.zoom != oldrc.zoom || rc.dpi != oldrc.dpi) {
		form_->controller().updateScreenFonts();
	}


	for (int i = 0; i < lcolors_.size(); ++i) {
		if (prefcolors_[i]!=newcolors_[i])
			form_->controller().setColor(lcolors_[i], fromqstr(newcolors_[i]));
	}
}

// FIXME: move to helper_funcs.h
namespace {

template<class A>
typename std::vector<A>::size_type
findPos(std::vector<A> const & vec, A const & val)
{
	typedef typename std::vector<A>::const_iterator Cit;

	Cit it = std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return distance(vec.begin(), it);
}

void setComboxFont(QComboBox * cb, string const & family, string const & foundry)
{
	string const name = makeFontName(family, foundry);
	for (int i = 0; i < cb->count(); ++i) {
		if (fromqstr(cb->text(i)) == name) {
			cb->setCurrentItem(i);
			return;
		}
	}

	// Try matching without foundry name

	// We count in reverse in order to prefer the Xft foundry
	for (int i = cb->count() - 1; i >= 0; --i) {
		pair<string, string> tmp = parseFontName(fromqstr(cb->text(i)));
		if (compare_no_case(tmp.first, family) == 0) {
			cb->setCurrentItem(i);
			return;
		}
	}

	// family alone can contain e.g. "Helvetica [Adobe]"
	pair<string, string> tmpfam = parseFontName(family);

	// We count in reverse in order to prefer the Xft foundry
	for (int i = cb->count() - 1; i >= 0; --i) {
		pair<string, string> tmp = parseFontName(fromqstr(cb->text(i)));
		if (compare_no_case(tmp.first, tmpfam.first) == 0) {
			cb->setCurrentItem(i);
			return;
		}
	}

	// Bleh, default fonts, and the names couldn't be found. Hack
	// for bug 1063. Qt makes baby Jesus cry.

	QFont font;

	if (family == lyx_gui::roman_font_name()) {
		font.setStyleHint(QFont::Serif);
		font.setFamily(family.c_str());
	} else if (family == lyx_gui::sans_font_name()) {
		font.setStyleHint(QFont::SansSerif);
		font.setFamily(family.c_str());
	} else if (family == lyx_gui::typewriter_font_name()) {
		font.setStyleHint(QFont::TypeWriter);
		font.setFamily(family.c_str());
	} else {
		lyxerr << "FAILED to find the default font: '"
		       << foundry << "', '" << family << '\''<< endl;
		return;
	}

	QFontInfo info(font);
	pair<string, string> tmp = parseFontName(fromqstr(info.family()));
	string const & default_font_name = tmp.first;
	lyxerr << "Apparent font is " << default_font_name << endl;

	for (int i = 0; i < cb->count(); ++i) {
		lyxerr << "Looking at " << fromqstr(cb->text(i)) << endl;
		if (compare_no_case(fromqstr(cb->text(i)),
				    default_font_name) == 0) {
			cb->setCurrentItem(i);
			return;
		}
	}

	lyxerr << "FAILED to find the font: '"
	       << foundry << "', '" << family << '\'' <<endl;
}

} // end namespace anon

namespace {

QString const external_path(string const & input)
{
	return toqstr(lyx::support::os::external_path(input));
}

}

void QPrefsDialog::update(LyXRC const & rc)
{
	// FIXME: remove rtl_support bool
	languageModule->rtlCB->setChecked(rc.rtl_support);
	languageModule->markForeignCB->setChecked(rc.mark_foreign_language);
	languageModule->autoBeginCB->setChecked(rc.language_auto_begin);
	languageModule->autoEndCB->setChecked(rc.language_auto_end);
	languageModule->useBabelCB->setChecked(rc.language_use_babel);
	languageModule->globalCB->setChecked(rc.language_global_options);
	languageModule->languagePackageED->setText(toqstr(rc.language_package));
	languageModule->startCommandED->setText(toqstr(rc.language_command_begin));
	languageModule->endCommandED->setText(toqstr(rc.language_command_end));

	int const pos = int(findPos(lang_, rc.default_language));
	languageModule->defaultLanguageCO->setCurrentIndex(pos);

	uiModule->uiFileED->setText(external_path(rc.ui_file));
	uiModule->bindFileED->setText(external_path(rc.bind_file));
	uiModule->restoreCursorCB->setChecked(rc.use_lastfilepos);
	uiModule->loadSessionCB->setChecked(rc.load_session);
	uiModule->cursorFollowsCB->setChecked(rc.cursor_follows_scrollbar);
	// convert to minutes
	int mins(rc.autosave / 60);
	if (rc.autosave && !mins)
		mins = 1;
	uiModule->autoSaveSB->setValue(mins);
	uiModule->autoSaveCB->setChecked(rc.make_backup);
	uiModule->lastfilesSB->setValue(rc.num_lastfiles);


	identityModule->nameED->setText(toqstr(rc.user_name));
	identityModule->emailED->setText(toqstr(rc.user_email));


	// FIXME: can derive CB from the two EDs
	keyboardModule->keymapCB->setChecked(rc.use_kbmap);
	// no idea why we need these. Fscking Qt.
	keyboardModule->firstKeymapED->setEnabled(rc.use_kbmap);
	keyboardModule->firstKeymapPB->setEnabled(rc.use_kbmap);
	keyboardModule->firstKeymapLA->setEnabled(rc.use_kbmap);
	keyboardModule->secondKeymapED->setEnabled(rc.use_kbmap);
	keyboardModule->secondKeymapPB->setEnabled(rc.use_kbmap);
	keyboardModule->secondKeymapLA->setEnabled(rc.use_kbmap);
	keyboardModule->firstKeymapED->setText(external_path(rc.primary_kbmap));
	keyboardModule->secondKeymapED->setText(external_path(rc.secondary_kbmap));



	asciiModule->asciiLinelengthSB->setValue(rc.ascii_linelen);
	asciiModule->asciiRoffED->setText(toqstr(rc.ascii_roff_command));



	dateModule->DateED->setText(toqstr(rc.date_insert_format));



#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	cygwinpathModule->pathCB->setChecked(rc.cygwin_path_fix);
#endif



	latexModule->latexEncodingED->setText(toqstr(rc.fontenc));
	latexModule->latexChecktexED->setText(toqstr(rc.chktex_command));
	latexModule->latexBibtexED->setText(toqstr(rc.bibtex_command));
	latexModule->latexIndexED->setText(toqstr(rc.index_command));
	latexModule->latexAutoresetCB->setChecked(rc.auto_reset_options);
	latexModule->latexDviPaperED->setText(toqstr(rc.view_dvi_paper_option));
	latexModule->latexPaperSizeCO->setCurrentIndex(
		form_->controller().fromPaperSize(rc.default_papersize));



	switch (rc.preview) {
	case LyXRC::PREVIEW_OFF:
		displayModule->instantPreviewCO->setCurrentIndex(0);
		break;
	case LyXRC::PREVIEW_NO_MATH :
		displayModule->instantPreviewCO->setCurrentIndex(1);
		break;
	case LyXRC::PREVIEW_ON :
		displayModule->instantPreviewCO->setCurrentIndex(2);
		break;
	}

	int item = 2;
	switch (rc.display_graphics) {
		case lyx::graphics::NoDisplay:		item = 3; break;
		case lyx::graphics::ColorDisplay:	item = 2; break;
		case lyx::graphics::GrayscaleDisplay:	item = 1; break;
		case lyx::graphics::MonochromeDisplay:	item = 0; break;
		default: break;
	}
	displayModule->displayGraphicsCO->setCurrentIndex(item);



	pathsModule->workingDirED->setText(external_path(rc.document_path));
	pathsModule->templateDirED->setText(external_path(rc.template_path));
	pathsModule->backupDirED->setText(external_path(rc.backupdir_path));
	pathsModule->tempDirED->setText(external_path(rc.tempdir_path));
	pathsModule->pathPrefixED->setText(toqstr(rc.path_prefix));
	// FIXME: should be a checkbox only
	pathsModule->lyxserverDirED->setText(external_path(rc.lyxpipes));



	spellcheckerModule->spellCommandCO->setCurrentIndex(0);

	if (rc.isp_command == "ispell") {
		spellcheckerModule->spellCommandCO->setCurrentIndex(0);
	} else if (rc.isp_command == "aspell") {
		spellcheckerModule->spellCommandCO->setCurrentIndex(1);
	} else if (rc.isp_command == "hspell") {
		spellcheckerModule->spellCommandCO->setCurrentIndex(2);
	}

	if (rc.use_spell_lib) {
#if defined(USE_ASPELL) || defined(USE_PSPELL)
		spellcheckerModule->spellCommandCO->setCurrentIndex(3);
#endif
	}

	// FIXME: remove isp_use_alt_lang
	spellcheckerModule->altLanguageED->setText(toqstr(rc.isp_alt_lang));
	// FIXME: remove isp_use_esc_chars
	spellcheckerModule->escapeCharactersED->setText(toqstr(rc.isp_esc_chars));
	// FIXME: remove isp_use_pers_dict
	spellcheckerModule->persDictionaryED->setText(external_path(rc.isp_pers_dict));
	spellcheckerModule->compoundWordCB->setChecked(rc.isp_accept_compound);
	spellcheckerModule->inputEncodingCB->setChecked(rc.isp_use_input_encoding);




	printerModule->printerAdaptCB->setChecked(rc.print_adapt_output);
	printerModule->printerCommandED->setText(toqstr(rc.print_command));
	printerModule->printerNameED->setText(toqstr(rc.printer));

	printerModule->printerPageRangeED->setText(toqstr(rc.print_pagerange_flag));
	printerModule->printerCopiesED->setText(toqstr(rc.print_copies_flag));
	printerModule->printerReverseED->setText(toqstr(rc.print_reverse_flag));
	printerModule->printerToPrinterED->setText(toqstr(rc.print_to_printer));
	printerModule->printerExtensionED->setText(toqstr(rc.print_file_extension));
	printerModule->printerSpoolCommandED->setText(toqstr(rc.print_spool_command));
	printerModule->printerPaperTypeED->setText(toqstr(rc.print_paper_flag));
	printerModule->printerEvenED->setText(toqstr(rc.print_evenpage_flag));
	printerModule->printerOddED->setText(toqstr(rc.print_oddpage_flag));
	printerModule->printerCollatedED->setText(toqstr(rc.print_collcopies_flag));
	printerModule->printerLandscapeED->setText(toqstr(rc.print_landscape_flag));
	printerModule->printerToFileED->setText(external_path(rc.print_to_file));
	printerModule->printerExtraED->setText(toqstr(rc.print_extra_options));
	printerModule->printerSpoolPrefixED->setText(toqstr(rc.print_spool_printerprefix));
	printerModule->printerPaperSizeED->setText(toqstr(rc.print_paper_dimension_flag));




	setComboxFont(screenfontsModule->screenRomanCO, rc.roman_font_name,
			rc.roman_font_foundry);
	setComboxFont(screenfontsModule->screenSansCO, rc.sans_font_name,
			rc.sans_font_foundry);
	setComboxFont(screenfontsModule->screenTypewriterCO, rc.typewriter_font_name,
			rc.typewriter_font_foundry);

	select_roman(screenfontsModule->screenRomanCO->currentText());
	select_sans(screenfontsModule->screenSansCO->currentText());
	select_typewriter(screenfontsModule->screenTypewriterCO->currentText());

	screenfontsModule->screenZoomSB->setValue(rc.zoom);
	screenfontsModule->screenDpiSB->setValue(rc.dpi);
	screenfontsModule->screenTinyED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_TINY]));
	screenfontsModule->screenSmallestED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_SCRIPT]));
	screenfontsModule->screenSmallerED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_FOOTNOTE]));
	screenfontsModule->screenSmallED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_SMALL]));
	screenfontsModule->screenNormalED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_NORMAL]));
	screenfontsModule->screenLargeED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_LARGE]));
	screenfontsModule->screenLargerED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_LARGER]));
	screenfontsModule->screenLargestED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_LARGEST]));
	screenfontsModule->screenHugeED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_HUGE]));
	screenfontsModule->screenHugerED->setText(toqstr(rc.font_sizes[LyXFont::SIZE_HUGER]));

	updateFormats();
	updateConverters();
	updateCopiers();

}


} // namespace frontend
} // namespace lyx

#include "QPrefsDialog_moc.cpp"
