/**
 * \file QPrefsDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include "debug.h"
#include "qt_helpers.h"

#include "QPrefsDialog.h"
#include "QPrefs.h"

#include "panelstack.h"
#include "qcoloritem.h"
#include "qfontexample.h"

#include "ui/QPrefAsciiModule.h"
#include "ui/QPrefDateModule.h"
#include "ui/QPrefKeyboardModule.h"
#include "ui/QPrefLatexModule.h"
#include "ui/QPrefScreenFontsModule.h"
#include "ui/QPrefColorsModule.h"
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#include "ui/QPrefCygwinPathModule.h"
#endif
#include "ui/QPrefDisplayModule.h"
#include "ui/QPrefPathsModule.h"
#include "ui/QPrefSpellcheckerModule.h"
#include "ui/QPrefConvertersModule.h"
#include "ui/QPrefCopiersModule.h"
#include "ui/QPrefFileformatsModule.h"
#include "ui/QPrefLanguageModule.h"
#include "ui/QPrefPrinterModule.h"
#include "ui/QPrefUIModule.h"
#include "ui/QPrefIdentityModule.h"

#include "gettext.h"
#include "LColor.h"

#include "controllers/ControlPrefs.h"

#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qfontdatabase.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qvalidator.h>

using std::string;

namespace lyx {
namespace frontend {

QPrefsDialog::QPrefsDialog(QPrefs * form)
	: QPrefsDialogBase(0, 0, false, 0), form_(form)
{
	connect(savePB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));

	asciiModule = new QPrefAsciiModule(this);
	dateModule = new QPrefDateModule(this);
	keyboardModule = new QPrefKeyboardModule(this);
	latexModule = new QPrefLatexModule(this);
	screenfontsModule = new QPrefScreenFontsModule(this);
	colorsModule = new QPrefColorsModule(this);
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	cygwinpathModule = new QPrefCygwinPathModule(this);
#endif
	displayModule = new QPrefDisplayModule(this);
	pathsModule = new QPrefPathsModule(this);
	spellcheckerModule = new QPrefSpellcheckerModule(this);
	convertersModule = new QPrefConvertersModule(this);
	copiersModule = new QPrefCopiersModule(this);
	fileformatsModule = new QPrefFileformatsModule(this);
	languageModule = new QPrefLanguageModule(this);
	printerModule = new QPrefPrinterModule(this);
	uiModule = new QPrefUIModule(this);
	identityModule = new QPrefIdentityModule(this);

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
	prefsPS->addPanel(spellcheckerModule, _("Spell-checker"), ls);

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

	// FIXME: put in controller
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

		colors_.push_back(lc);
		string const x11name(lcolor.getX11Name(lc));
		string const guiname(lcolor.getGUIName(lc));
		QColorItem * ci(new QColorItem(QColor(toqstr(x11name)),
				toqstr(guiname)));
		colorsModule->lyxObjectsLB->insertItem(ci);
	}

	QFontDatabase fontdb;
	QStringList families(fontdb.families());

	connect(screenfontsModule->screenRomanCO, SIGNAL(activated(const QString&)), this, SLOT(select_roman(const QString&)));
	connect(screenfontsModule->screenSansCO, SIGNAL(activated(const QString&)), this, SLOT(select_sans(const QString&)));
	connect(screenfontsModule->screenTypewriterCO, SIGNAL(activated(const QString&)), this, SLOT(select_typewriter(const QString&)));

	for (QStringList::Iterator it = families.begin(); it != families.end(); ++it) {
		screenfontsModule->screenRomanCO->insertItem(*it);
		screenfontsModule->screenSansCO->insertItem(*it);
		screenfontsModule->screenTypewriterCO->insertItem(*it);
	}

	connect(uiModule->uiFilePB, SIGNAL(clicked()), this, SLOT(select_ui()));
	connect(uiModule->bindFilePB, SIGNAL(clicked()), this, SLOT(select_bind()));

	connect(keyboardModule->firstKeymapPB, SIGNAL(clicked()), this, SLOT(select_keymap1()));
	connect(keyboardModule->secondKeymapPB, SIGNAL(clicked()), this, SLOT(select_keymap2()));

	connect(spellcheckerModule->persDictionaryPB, SIGNAL(clicked()), this, SLOT(select_dict()));

	connect(pathsModule->templateDirPB, SIGNAL(clicked()), this, SLOT(select_templatedir()));
	connect(pathsModule->tempDirPB, SIGNAL(clicked()), this, SLOT(select_tempdir()));
	connect(pathsModule->backupDirPB, SIGNAL(clicked()), this, SLOT(select_backupdir()));
	connect(pathsModule->workingDirPB, SIGNAL(clicked()), this, SLOT(select_workingdir()));
	connect(pathsModule->lyxserverDirPB, SIGNAL(clicked()), this, SLOT(select_lyxpipe()));

	connect(colorsModule->colorChangePB, SIGNAL(clicked()), this, SLOT(change_color()));
	connect(colorsModule->lyxObjectsLB, SIGNAL(selected(int)), this, SLOT(change_color()));

	connect(fileformatsModule->formatNewPB, SIGNAL(clicked()), this, SLOT(new_format()));
	connect(fileformatsModule->formatRemovePB, SIGNAL(clicked()), this, SLOT(remove_format()));
	connect(fileformatsModule->formatModifyPB, SIGNAL(clicked()), this, SLOT(modify_format()));
	connect(fileformatsModule->formatsLB, SIGNAL(highlighted(int)), this, SLOT(switch_format(int)));
	connect(fileformatsModule->formatED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->guiNameED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->shortcutED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->extensionED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->viewerED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));
	connect(fileformatsModule->editorED, SIGNAL(textChanged(const QString&)), this, SLOT(fileformat_changed()));

	connect(convertersModule->converterNewPB, SIGNAL(clicked()), this, SLOT(new_converter()));
	connect(convertersModule->converterRemovePB, SIGNAL(clicked()), this, SLOT(remove_converter()));
	connect(convertersModule->converterModifyPB, SIGNAL(clicked()), this, SLOT(modify_converter()));
	connect(convertersModule->convertersLB, SIGNAL(highlighted(int)), this, SLOT(switch_converter(int)));
	connect(convertersModule->converterFromCO, SIGNAL(activated(const QString&)), this, SLOT(converter_changed()));
	connect(convertersModule->converterToCO, SIGNAL(activated(const QString&)), this, SLOT(converter_changed()));
	connect(convertersModule->converterED, SIGNAL(textChanged(const QString&)), this, SLOT(converter_changed()));
	connect(convertersModule->converterFlagED, SIGNAL(textChanged(const QString&)), this, SLOT(converter_changed()));

	// Qt really sucks. This is as ugly as it looks, but the alternative
	// means having to derive every module == bloat

	connect(convertersModule->converterNewPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(convertersModule->converterRemovePB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(convertersModule->converterModifyPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	connect(copiersModule->copierNewPB, SIGNAL(clicked()), this, SLOT(new_copier()));
	connect(copiersModule->copierRemovePB, SIGNAL(clicked()), this, SLOT(remove_copier()));
	connect(copiersModule->copierModifyPB, SIGNAL(clicked()), this, SLOT(modify_copier()));
	connect(copiersModule->AllCopiersLB, SIGNAL(highlighted(int)), this, SLOT(switch_copierLB(int)));
	connect(copiersModule->copierFormatCO, SIGNAL(activated(int)), this, SLOT(switch_copierCO(int)));
	connect(copiersModule->copierNewPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(copiersModule->copierRemovePB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(copiersModule->copierModifyPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(copiersModule->copierFormatCO, SIGNAL(activated(const QString&)), this, SLOT(copiers_changed()));
	connect(copiersModule->copierED, SIGNAL(textChanged(const QString&)), this, SLOT(copiers_changed()));

	connect(fileformatsModule->formatNewPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(fileformatsModule->formatRemovePB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(fileformatsModule->formatModifyPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
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
	connect(uiModule->uiFileED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(uiModule->bindFileED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(uiModule->cursorFollowsCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(uiModule->wheelMouseSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(uiModule->autoSaveSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(uiModule->autoSaveCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(uiModule->lastfilesSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(keyboardModule->keymapCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(keyboardModule->firstKeymapED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(keyboardModule->secondKeymapED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(asciiModule->asciiLinelengthSB, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(asciiModule->asciiRoffED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(dateModule->DateED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	connect(cygwinpathModule->pathCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
#endif
	connect(latexModule->latexEncodingED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexChecktexED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexBibtexED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexIndexED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexAutoresetCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(latexModule->latexDviPaperED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexPaperSizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(displayModule->instantPreviewCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(displayModule->displayGraphicsCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pathsModule->workingDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->templateDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->backupDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->tempDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->lyxserverDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->pathPrefixED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
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
	connect(identityModule->nameED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(identityModule->emailED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));

	// initialize the validators
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
	QPrefConvertersModule * convertmod(convertersModule);

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

	convertmod->convertersLB->clear();

	Converters::const_iterator ccit = form_->converters().begin();
	Converters::const_iterator cend = form_->converters().end();
	for (; ccit != cend; ++ccit) {
		std::string const name = ccit->From->prettyname() + " -> "
			+ ccit->To->prettyname();
		convertmod->convertersLB->insertItem(toqstr(name));
	}

	// restore selection
	if (!current.isEmpty()) {
		QListBoxItem * item = convertmod->convertersLB->findItem(current);
		convertmod->convertersLB->setCurrentItem(item);
	}
	// select first element if restoring failed
	if (convertmod->convertersLB->currentItem() == -1)
		convertmod->convertersLB->setCurrentItem(0);

	updateConverterButtons();
}


void QPrefsDialog::switch_converter(int nr)
{
	Converter const & c(form_->converters().get(nr));
	convertersModule->converterFromCO->setCurrentItem(form_->formats().getNumber(c.from));
	convertersModule->converterToCO->setCurrentItem(form_->formats().getNumber(c.to));
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
		convertersModule->convertersLB->currentItem()));
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
	convertersModule->convertersLB->setCurrentItem(convertersModule->convertersLB->count() - 1);
}


void QPrefsDialog::modify_converter()
{
	int const top_item = convertersModule->convertersLB->topItem();
	QString const current_text =
		convertersModule->convertersLB->currentText();

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

	QListBoxItem * const item =
		convertersModule->convertersLB->findItem(current_text);
	convertersModule->convertersLB->setCurrentItem(item);
	convertersModule->convertersLB->setTopItem(top_item);
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
	copiersModule->AllCopiersLB->clear();

	for (Movers::iterator it = form_->movers().begin(),
		     end = form_->movers().end();
	     it != end; ++it) {
		std::string const & command = it->second.command();
		if (command.empty())
			continue;
		std::string const & fmt = it->first;
		std::string const & pretty = form_->formats().prettyName(fmt);

		copiersModule->AllCopiersLB->insertItem(toqstr(pretty));
	}

	// restore selection
	if (!current.isEmpty()) {
		QListBoxItem * item = copiersModule->AllCopiersLB->findItem(current);
		copiersModule->AllCopiersLB->setCurrentItem(item);
	}
	// select first element if restoring failed
	if (copiersModule->AllCopiersLB->currentItem() == -1)
		copiersModule->AllCopiersLB->setCurrentItem(0);
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


void QPrefsDialog::switch_copierLB(int nr)
{
	std::string const browser_text =
		fromqstr(copiersModule->AllCopiersLB->currentText());
	lyxerr << "switch_copierLB(" << nr << ")\n"
	       << "browser_text " << browser_text << std::endl;
	Format const * fmt = getFormat(browser_text);
	if (fmt == 0)
		return;

	string const & fmt_name = fmt->name();
	string const & gui_name = fmt->prettyname();
	string const & command = form_->movers().command(fmt_name);

	lyxerr << "switch_copierLB(" << nr << ")\n"
	       << "fmt_name " << fmt_name << '\n'
	       << "gui_name " << gui_name << '\n'
	       << "command " << command << std::endl;

	copiersModule->copierED->clear();
	int const combo_size = copiersModule->copierFormatCO->count();
	for (int i = 0; i < combo_size; ++i) {
		QString const qtext = copiersModule->copierFormatCO->text(i);
		std::string const text = fromqstr(qtext);
		if (text == gui_name) {
			copiersModule->copierFormatCO->setCurrentItem(i);
			copiersModule->copierED->setText(toqstr(command));
			lyxerr << "found combo item " << i << std::endl;
			break;
		}
	}
	updateCopierButtons();
}


void QPrefsDialog::switch_copierCO(int nr)
{
	std::string const combo_text =
		fromqstr(copiersModule->copierFormatCO->currentText());
	lyxerr << "switch_copierCO(" << nr << ")\n"
	       << "combo_text " << combo_text << std::endl;
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	string const & fmt_name = fmt->name();
	string const & gui_name = fmt->prettyname();
	string const & command = form_->movers().command(fmt_name);

	lyxerr << "switch_copierCO(" << nr << ")\n"
	       << "fmt_name " << fmt_name << '\n'
	       << "gui_name " << gui_name << '\n'
	       << "command " << command << std::endl;

	copiersModule->copierED->setText(toqstr(command));

	int const index = copiersModule->AllCopiersLB->currentItem();
	if (index >= 0)
		copiersModule->AllCopiersLB->setSelected(index, false);

	int const browser_size = copiersModule->AllCopiersLB->count();
	for (int i = 0; i < browser_size; ++i) {
		QString const qtext = copiersModule->AllCopiersLB->text(i);
		std::string const text = fromqstr(qtext);
		if (text == gui_name) {
			copiersModule->AllCopiersLB->setSelected(i, true);
			int top = std::max(i - 5, 0);
			copiersModule->AllCopiersLB->setTopItem(top);
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
	for (unsigned int i = 0; i != copiersModule->AllCopiersLB->count(); i++) {
		if (copiersModule->AllCopiersLB->text(i) == selected)
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
	int const last = copiersModule->AllCopiersLB->count() - 1;
	copiersModule->AllCopiersLB->setCurrentItem(last);

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
	QPrefFileformatsModule * formatmod(fileformatsModule);

	// save current selection
	QString current = formatmod->guiNameED->text();

	formatmod->formatsLB->clear();

	Formats::const_iterator cit = form_->formats().begin();
	Formats::const_iterator end = form_->formats().end();
	for (; cit != end; ++cit) {
		formatmod->formatsLB->insertItem(toqstr(cit->prettyname()));
	}

	// restore selection
	if (!current.isEmpty()) {
		QListBoxItem * item = formatmod->formatsLB->findItem(current);
		formatmod->formatsLB->setCurrentItem(item);
	}
	// select first element if restoring failed
	if (formatmod->formatsLB->currentItem() == -1)
		formatmod->formatsLB->setCurrentItem(0);
}


void QPrefsDialog::switch_format(int nr)
{
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
	for (unsigned int i = 0; i != fileformatsModule->formatsLB->count(); i++) {
		if (fileformatsModule->formatsLB->text(i) == gui_name) {
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
		fileformatsModule->formatsLB->currentItem()));
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
	fileformatsModule->formatsLB->setCurrentItem(form_->formats().getNumber(name));
	form_->converters().update(form_->formats());

	updateConverters();
	updateFormatsButtons();
}


void QPrefsDialog::modify_format()
{
	int const top_item = fileformatsModule->formatsLB->topItem();
	int const current_item = fileformatsModule->formatsLB->currentItem();
	QString const current_text =
		fileformatsModule->formatsLB->currentText();

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

	fileformatsModule->formatsLB->setUpdatesEnabled(false);
	updateFormats();
	fileformatsModule->formatsLB->setUpdatesEnabled(true);
	fileformatsModule->formatsLB->update();

	updateConverters();
	updateFormatsButtons();

	QListBoxItem * const item =
		fileformatsModule->formatsLB->findItem(current_text);
	fileformatsModule->formatsLB->setCurrentItem(item);
	fileformatsModule->formatsLB->setTopItem(top_item);
}


void QPrefsDialog::remove_format()
{
	int const nr(fileformatsModule->formatsLB->currentItem());
	if (nr < 0)
		return;
	form_->formats().erase(form_->formats().get(nr).name());
	updateFormats();
	form_->converters().update(form_->formats());

	updateConverters();
	updateFormatsButtons();
}


void QPrefsDialog::change_color()
{
	QListBox * lb(colorsModule->lyxObjectsLB);
	if (lb->currentItem() < 0)
		return;
	QListBoxItem * ib(lb->item(lb->currentItem()));
	QColorItem * ci(static_cast<QColorItem*>(ib));
	QColor c(QColorDialog::getColor(ci->color(), qApp->focusWidget() ? qApp->focusWidget() : qApp->mainWidget()));
	if (c.isValid()) {
		ci->color(c);
		lb->update();
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

} // namespace frontend
} // namespace lyx
