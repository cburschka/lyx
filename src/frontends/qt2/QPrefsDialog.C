/**
 * \file QPrefsDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
#include "qt_helpers.h"

#include "ControlPrefs.h"
#include "QPrefs.h"


#include "QPrefsDialog.h"

#include "ui/QPrefAsciiModule.h"
#include "ui/QPrefDateModule.h"
#include "ui/QPrefKeyboardModule.h"
#include "ui/QPrefLatexModule.h"
#include "ui/QPrefScreenFontsModule.h"
#include "ui/QPrefColorsModule.h"
#include "ui/QPrefDisplayModule.h"
#include "ui/QPrefPathsModule.h"
#include "ui/QPrefSpellcheckerModule.h"
#include "ui/QPrefConvertersModule.h"
#include "ui/QPrefFileformatsModule.h"
#include "ui/QPrefLanguageModule.h"
#include "ui/QPrefPrinterModule.h"
#include "ui/QPrefUIModule.h"
#include "ui/QPrefIdentityModule.h"

#include "support/lstrings.h"
#include "lyxrc.h"
#include "debug.h"
#include "gettext.h"

#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfontdatabase.h>
#include <qstringlist.h>
#include <qcolordialog.h>
#include <qcolor.h>
#include "qcoloritem.h"
#include "qfontexample.h"

using std::map;
using std::endl;

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

	prefsLV->setSorting(-1);

	// OK, Qt is REALLY broken. We have to hard
	// code the menu structure here.

	QListViewItem * lnf(new QListViewItem(prefsLV, qt_("Look and feel")));
	lnf->setSelectable(false);
	lnf->setOpen(true);
	QListViewItem * lan(new QListViewItem(prefsLV, lnf, qt_("Language settings")));
	lan->setSelectable(false);
	QListViewItem * out(new QListViewItem(prefsLV, lan, qt_("Outputs")));
	out->setSelectable(false);

	asciiModule = new QPrefAsciiModule(prefsWS);
	dateModule = new QPrefDateModule(prefsWS);
	keyboardModule = new QPrefKeyboardModule(prefsWS);
	latexModule = new QPrefLatexModule(prefsWS);
	screenfontsModule = new QPrefScreenFontsModule(prefsWS);
	colorsModule = new QPrefColorsModule(prefsWS);
	displayModule = new QPrefDisplayModule(prefsWS);
	pathsModule = new QPrefPathsModule(prefsWS);
	spellcheckerModule = new QPrefSpellcheckerModule(prefsWS);
	convertersModule = new QPrefConvertersModule(prefsWS);
	fileformatsModule = new QPrefFileformatsModule(prefsWS);
	languageModule = new QPrefLanguageModule(prefsWS);
	printerModule = new QPrefPrinterModule(prefsWS);
	uiModule = new QPrefUIModule(prefsWS);
	identityModule = new QPrefIdentityModule(prefsWS);

	prefsWS->addWidget(asciiModule, 0);
	prefsWS->addWidget(dateModule, 1);
	prefsWS->addWidget(keyboardModule, 2);
	prefsWS->addWidget(latexModule, 3);
	prefsWS->addWidget(screenfontsModule, 4);
	prefsWS->addWidget(colorsModule, 5);
	prefsWS->addWidget(displayModule, 6);
	prefsWS->addWidget(pathsModule, 7);
	prefsWS->addWidget(spellcheckerModule, 8);
	prefsWS->addWidget(convertersModule, 9);
	prefsWS->addWidget(fileformatsModule, 10);
	prefsWS->addWidget(languageModule, 11);
	prefsWS->addWidget(printerModule, 12);
	prefsWS->addWidget(uiModule, 13);
	prefsWS->addWidget(identityModule, 14);

	QListViewItem * i;

	// language settings

	i = new QListViewItem(lan, qt_("Language"));
	pane_map_[i] = languageModule;
	i = new QListViewItem(lan, i, qt_("Spellchecker"));
	pane_map_[i] = spellcheckerModule;

	// UI

	i = new QListViewItem(lnf, qt_("User interface"));
	pane_map_[i] = uiModule;
	prefsLV->setCurrentItem(i);

	i = new QListViewItem(lnf, i, qt_("Screen fonts"));
	pane_map_[i] = screenfontsModule;
	i = new QListViewItem(lnf, i, qt_("Colors"));
	pane_map_[i] = colorsModule;
	i = new QListViewItem(lnf, i, qt_("Graphics"));
	pane_map_[i] = displayModule;
	i = new QListViewItem(lnf, i, qt_("Keyboard"));
	pane_map_[i] = keyboardModule;

	// output

	i = new QListViewItem(out, qt_("ASCII"));
	pane_map_[i] = asciiModule;
	i = new QListViewItem(out, i, qt_("Date format"));
	pane_map_[i] = dateModule;
	i = new QListViewItem(out, i, qt_("LaTeX"));
	pane_map_[i] = latexModule;
	i = new QListViewItem(out, i, qt_("Printer"));
	pane_map_[i] = printerModule;

	i = new QListViewItem(prefsLV, out, qt_("Identity"));
	pane_map_[i] = identityModule;
	i = new QListViewItem(prefsLV, out, qt_("Paths"));
	pane_map_[i] = pathsModule;
	i = new QListViewItem(prefsLV, i,  qt_("Converters"));
	pane_map_[i] = convertersModule;
	i = new QListViewItem(prefsLV, i, qt_("File formats"));
	pane_map_[i] = fileformatsModule;

	prefsLV->setMinimumSize(prefsLV->sizeHint());

	// Qt sucks
	resize(minimumSize());

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

	connect(convertersModule->converterNewPB, SIGNAL(clicked()), this, SLOT(new_converter()));
	connect(convertersModule->converterRemovePB, SIGNAL(clicked()), this, SLOT(remove_converter()));
	connect(convertersModule->converterModifyPB, SIGNAL(clicked()), this, SLOT(modify_converter()));
	connect(convertersModule->convertersLB, SIGNAL(highlighted(int)), this, SLOT(switch_converter(int)));

	// Qt really sucks. This is as ugly as it looks, but the alternative
	// means having to derive every module == bloat

	connect(convertersModule->converterNewPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(convertersModule->converterRemovePB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(convertersModule->converterModifyPB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
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
	connect(latexModule->latexEncodingED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexChecktexED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexAutoresetCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(latexModule->latexDviPaperED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->latexPaperSizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(displayModule->previewCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(displayModule->displayGraphicsCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pathsModule->workingDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->templateDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->backupDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->tempDirCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(pathsModule->tempDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pathsModule->lyxserverDirED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(spellcheckerModule->spellCommandCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
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
}


QPrefsDialog::~QPrefsDialog()
{
}


void QPrefsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QPrefsDialog::switchPane(QListViewItem * i)
{
	prefsWS->raiseWidget(pane_map_[i]);
}


void QPrefsDialog::change_adaptor()
{
	form_->changed();
}


void QPrefsDialog::updateConverters()
{
	QPrefConvertersModule * convertmod(convertersModule);

	convertmod->converterFromCO->clear();
	convertmod->converterToCO->clear();

	Formats::const_iterator cit = form_->formats_.begin();
	Formats::const_iterator end = form_->formats_.end();
	for (; cit != end; ++cit) {
		convertmod->converterFromCO->insertItem(toqstr(cit->prettyname()));
		convertmod->converterToCO->insertItem(toqstr(cit->prettyname()));
	}

	convertmod->convertersLB->clear();

	Converters::const_iterator ccit = form_->converters_.begin();
	Converters::const_iterator cend = form_->converters_.end();
	for (; ccit != cend; ++ccit) {
		string const name(ccit->From->prettyname() + " -> " +
			ccit->To->prettyname());
		convertmod->convertersLB->insertItem(toqstr(name));
	}

	if (convertmod->convertersLB->currentItem() == -1)
		convertmod->convertersLB->setCurrentItem(0);
}


void QPrefsDialog::switch_converter(int nr)
{
	Converter const & c(form_->converters_.get(nr));
	convertersModule->converterFromCO->setCurrentItem(form_->formats_.getNumber(c.from));
	convertersModule->converterToCO->setCurrentItem(form_->formats_.getNumber(c.to));
	convertersModule->converterED->setText(toqstr(c.command));
	convertersModule->converterFlagED->setText(toqstr(c.flags));
}


// FIXME: user must
// specify unique from/to or it doesn't appear. This is really bad UI
void QPrefsDialog::new_converter()
{
	Format const & from(form_->formats_.get(convertersModule->converterFromCO->currentItem()));
	Format const & to(form_->formats_.get(convertersModule->converterToCO->currentItem()));

	Converter const * old = form_->converters_.getConverter(from.name(), to.name());
	form_->converters_.add(from.name(), to.name(), "", "");
	if (!old) {
		form_->converters_.updateLast(form_->formats_);
	}
	updateConverters();
	convertersModule->convertersLB->setCurrentItem(convertersModule->convertersLB->count() - 1);
}


void QPrefsDialog::modify_converter()
{
	Format const & from(form_->formats_.get(convertersModule->converterFromCO->currentItem()));
	Format const & to(form_->formats_.get(convertersModule->converterToCO->currentItem()));
	string flags(fromqstr(convertersModule->converterFlagED->text()));
	string name(fromqstr(convertersModule->converterED->text()));

	Converter const * old = form_->converters_.getConverter(from.name(), to.name());
	form_->converters_.add(from.name(), to.name(), name, flags);
	if (!old) {
		form_->converters_.updateLast(form_->formats_);
	}
	updateConverters();
}


void QPrefsDialog::remove_converter()
{
	Format const & from(form_->formats_.get(convertersModule->converterFromCO->currentItem()));
	Format const & to(form_->formats_.get(convertersModule->converterToCO->currentItem()));
	form_->converters_.erase(from.name(), to.name());
	updateConverters();
}


void QPrefsDialog::updateFormats()
{
	QPrefFileformatsModule * formatmod(fileformatsModule);

	formatmod->formatsLB->clear();

	Formats::const_iterator cit = form_->formats_.begin();
	Formats::const_iterator end = form_->formats_.end();
	for (; cit != end; ++cit) {
		formatmod->formatsLB->insertItem(toqstr(cit->prettyname()));
	}

	if (formatmod->formatsLB->currentItem() == -1)
		formatmod->formatsLB->setCurrentItem(0);
}


void QPrefsDialog::switch_format(int nr)
{
	Format const & f(form_->formats_.get(nr));
	fileformatsModule->formatED->setText(toqstr(f.name()));
	fileformatsModule->guiNameED->setText(toqstr(f.prettyname()));
	fileformatsModule->extensionED->setText(toqstr(f.extension()));
	fileformatsModule->shortcutED->setText(toqstr(f.shortcut()));
	fileformatsModule->viewerED->setText(toqstr(f.viewer()));
	fileformatsModule->formatRemovePB->setEnabled(
		!form_->converters_.formatIsUsed(f.name()));
}


void QPrefsDialog::new_format()
{
	form_->formats_.add(_("New"));
	form_->formats_.sort();
	updateFormats();
	fileformatsModule->formatsLB->setCurrentItem(form_->formats_.getNumber(_("New")));
	updateConverters();
}


void QPrefsDialog::modify_format()
{
	Format const & oldformat(form_->formats_.get(fileformatsModule->formatsLB->currentItem()));
	string const oldpretty(oldformat.prettyname());
	string const name(fromqstr(fileformatsModule->formatED->text()));
	form_->formats_.erase(oldformat.name());

	string const prettyname = fromqstr(fileformatsModule->guiNameED->text());
	string const extension = fromqstr(fileformatsModule->extensionED->text());
	string const shortcut = fromqstr(fileformatsModule->shortcutED->text());
	string const viewer = fromqstr(fileformatsModule->viewerED->text());

	form_->formats_.add(name, extension, prettyname, shortcut);
	form_->formats_.sort();
	form_->formats_.setViewer(name, viewer);

	fileformatsModule->formatsLB->setUpdatesEnabled(false);
	updateFormats();
	fileformatsModule->formatsLB->setUpdatesEnabled(true);
	fileformatsModule->formatsLB->update();

	updateConverters();
}


void QPrefsDialog::remove_format()
{
	int const nr(fileformatsModule->formatsLB->currentItem());
	if (nr < 0)
		return;
	form_->formats_.erase(form_->formats_.get(nr).name());
	updateFormats();
	updateConverters();
}


void QPrefsDialog::change_color()
{
	QListBox * lb(colorsModule->lyxObjectsLB);
	if (lb->currentItem() < 0)
		return;
	QListBoxItem * ib(lb->item(lb->currentItem()));
	QColorItem * ci(static_cast<QColorItem*>(ib));
	QColor c(QColorDialog::getColor(ci->color()));
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
