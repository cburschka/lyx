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

using std::map;
using std::endl;

QPrefsDialog::QPrefsDialog(QPrefs * form)
	: QPrefsDialogBase(0, 0, false, 0), form_(form)
{
	// FIXME: wrong
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
	QListViewItem * adv(new QListViewItem(prefsLV, lan, _("Advanced settings")));
	adv->setSelectable(false);

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

	// advanced settings

	i = new QListViewItem(adv, _("Converters"));
	pane_map_[i] = convertersModule;
	i = new QListViewItem(adv, i, _("File formats"));
	pane_map_[i] = fileformatsModule;
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

	// rag bag of crap

	i = new QListViewItem(prefsLV, lan, _("Ascii output"));
	pane_map_[i] = asciiModule;
	i = new QListViewItem(prefsLV, i, _("Date output"));
	pane_map_[i] = dateModule;
	i = new QListViewItem(prefsLV, i, _("Keyboard"));
	pane_map_[i] = keyboardModule;
	i = new QListViewItem(prefsLV, i, _("LaTeX"));
	pane_map_[i] = latexModule;
	i = new QListViewItem(prefsLV, i, _("Paths"));
	pane_map_[i] = pathsModule;
	i = new QListViewItem(prefsLV, i, _("Printer"));
	pane_map_[i] = printerModule;

	prefsLV->setMinimumSize(prefsLV->sizeHint());

	// Qt sucks
	resize(minimumSize());
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
