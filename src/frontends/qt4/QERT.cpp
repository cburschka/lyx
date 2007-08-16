/**
 * \file QERT.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QERT.h"
#include "Qt2BC.h"

#include "controllers/ControlERT.h"

#include <QRadioButton>
#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QERTDialog
//
/////////////////////////////////////////////////////////////////////


QERTDialog::QERTDialog(QERT * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(collapsedRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(openRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
}


void QERTDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QERTDialog::change_adaptor()
{
	form_->changed();
}


/////////////////////////////////////////////////////////////////////
//
// QERT
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlERT, QView<QERTDialog> > ERTBase;


QERT::QERT(Dialog & parent)
	: ERTBase(parent, _("TeX Code Settings"))
{
}


void QERT::build_dialog()
{
	dialog_.reset(new QERTDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
}


void QERT::apply()
{
	if (dialog_->openRB->isChecked())
		controller().setStatus(Inset::Open);
	else
		controller().setStatus(Inset::Collapsed);
}


void QERT::update_contents()
{
	QRadioButton * rb = 0;

	switch (controller().status()) {
		case InsetERT::Open: rb = dialog_->openRB; break;
		case InsetERT::Collapsed: rb = dialog_->collapsedRB; break;
	}

	rb->setChecked(true);
}

} // namespace frontend
} // namespace lyx

#include "QERT_moc.cpp"
