/**********************************************************************

	--- Qt Architect generated file ---

	File: indexdlg.C
	Last generated: Thu Sep 14 12:08:37 2000

 *********************************************************************/

#include <config.h>
#include <gettext.h>

#include "indexdlg.h"
#include "FormIndex.h"

#include <qtooltip.h>

#define Inherited IndexDialogData

IndexDialog::IndexDialog(FormIndex * f, QWidget *p, const char* name)
	: Inherited( p, name ), form_(f)
{
	setCaption(name);

	// tooltips

	QToolTip::add(labelindex,_("Index entry"));
	QToolTip::add(index,_("Index entry"));

	setMinimumSize( 200, 65 );
	setMaximumSize( 32767, 65 );
}


IndexDialog::~IndexDialog()
{
}


void IndexDialog::clickedOK()
{
    form_->apply();
    form_->close();
    hide();
}


void IndexDialog::clickedCancel()
{
    form_->close();
    hide();
}


void IndexDialog::setReadOnly(bool readonly)
{
    if (readonly) {
	index->setFocusPolicy(QWidget::NoFocus);
	buttonOk->setEnabled(false);
	buttonCancel->setText(_("&Close"));
    } else {
	index->setFocusPolicy(QWidget::StrongFocus);
	index->setFocus();
	buttonOk->setEnabled(true);
	buttonCancel->setText(_("&Cancel"));
    }
}
