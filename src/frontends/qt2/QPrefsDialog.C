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
#include "gettext.h"

#include "ControlPrefs.h"
#include "QPrefs.h"

#ifdef __GNUG__
#pragma implementation
#endif

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

#include "support/lstrings.h"
#include "lyxrc.h"
#include "debug.h"

#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcolordialog.h>
#include <qcolor.h>
#include "qcoloritem.h"

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

	QListViewItem * lnf(new QListViewItem(prefsLV, _("Look and feel")));
	lnf->setSelectable(false);
	lnf->setOpen(true);
	QListViewItem * lan(new QListViewItem(prefsLV, lnf, _("Language settings")));
	lan->setSelectable(false);
	QListViewItem * out(new QListViewItem(prefsLV, lan, _("Outputs")));
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

	QListViewItem * i;

	// language settings

	i = new QListViewItem(lan, _("Language"));
	pane_map_[i] = languageModule;
	i = new QListViewItem(lan, i, _("Spellchecker"));
	pane_map_[i] = spellcheckerModule;

	// UI

	i = new QListViewItem(lnf, _("User interface"));
	pane_map_[i] = uiModule;
	prefsLV->setCurrentItem(i);

	i = new QListViewItem(lnf, i, _("Screen fonts"));
	pane_map_[i] = screenfontsModule;
	i = new QListViewItem(lnf, i, _("Colors"));
	pane_map_[i] = colorsModule;
	i = new QListViewItem(lnf, i, _("Graphics"));
	pane_map_[i] = displayModule;
	i = new QListViewItem(lnf, i, _("Keyboard"));
	pane_map_[i] = keyboardModule;

	// output
 
	i = new QListViewItem(out, _("Ascii"));
	pane_map_[i] = asciiModule;
	i = new QListViewItem(out, i, _("Date format"));
	pane_map_[i] = dateModule;
	i = new QListViewItem(out, i, _("LaTeX"));
	pane_map_[i] = latexModule;
	i = new QListViewItem(out, i, _("Printer"));
	pane_map_[i] = printerModule;

	i = new QListViewItem(prefsLV, out, _("Paths"));
	pane_map_[i] = pathsModule;
	i = new QListViewItem(prefsLV, i,  _("Converters"));
	pane_map_[i] = convertersModule;
	i = new QListViewItem(prefsLV, i, _("File formats"));
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
		QColorItem * ci(new QColorItem(QColor(x11name.c_str()), guiname.c_str()));
		colorsModule->lyxObjectsLB->insertItem(ci);
	}

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
	connect(screenfontsModule->screenRomanED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenSansED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(screenfontsModule->screenTypewriterED, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
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
		convertmod->converterFromCO->insertItem(cit->prettyname().c_str());
		convertmod->converterToCO->insertItem(cit->prettyname().c_str());
	}
 
	convertmod->convertersLB->clear(); 

	Converters::const_iterator ccit = form_->converters_.begin();
	Converters::const_iterator cend = form_->converters_.end();
	for (; ccit != cend; ++ccit) {
		string const name(ccit->From->prettyname() + " -> " +
			ccit->To->prettyname()); 
		convertmod->convertersLB->insertItem(name.c_str());
	}
}

 
void QPrefsDialog::switch_converter(int nr)
{
	Converter const & c(form_->converters_.get(nr));
	convertersModule->converterFromCO->setCurrentItem(form_->formats_.getNumber(c.from));
	convertersModule->converterToCO->setCurrentItem(form_->formats_.getNumber(c.to));
	convertersModule->converterED->setText(c.command.c_str());
	convertersModule->converterFlagED->setText(c.flags.c_str());
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
	string flags(convertersModule->converterFlagED->text().latin1());
	string name(convertersModule->converterED->text().latin1());
 
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
		formatmod->formatsLB->insertItem(cit->prettyname().c_str());
	}
}


void QPrefsDialog::switch_format(int nr)
{
	Format const & f(form_->formats_.get(nr));
	fileformatsModule->formatED->setText(f.name().c_str());
	fileformatsModule->guiNameED->setText(f.prettyname().c_str());
	fileformatsModule->extensionED->setText(f.extension().c_str());
	fileformatsModule->shortcutED->setText(f.shortcut().c_str());
	fileformatsModule->viewerED->setText(f.viewer().c_str());
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
	string const name(fileformatsModule->formatED->text().latin1());
	form_->formats_.erase(oldformat.name());
 
        string const prettyname = fileformatsModule->guiNameED->text().latin1();
        string const extension = fileformatsModule->extensionED->text().latin1();
        string const shortcut = fileformatsModule->shortcutED->text().latin1();
        string const viewer = fileformatsModule->viewerED->text().latin1();
 
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
