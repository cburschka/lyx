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

#include "ui/ClassModuleBase.h"
#include "ui/PackagesModuleBase.h"
#include "ui/PaperModuleBase.h"
#include "ui/LanguageModuleBase.h"
#include "ui/BulletsModuleBase.h"
#include "BulletsModule.h"
#include "ui/BiblioModuleBase.h"
#include "ui/NumberingModuleBase.h"
#include "ui/MarginsModuleBase.h"
#include "ui/PreambleModuleBase.h"

#include "Spacing.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "buffer.h"

#include <qwidgetstack.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include "lengthcombo.h"


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
}


QPrefsDialog::~QPrefsDialog()
{
}


void QPrefsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
