/**
 * \file GuiERT.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiERT.h"
#include "gettext.h"

#include <QRadioButton>
#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiERTDialog
//
/////////////////////////////////////////////////////////////////////


GuiERTDialog::GuiERTDialog(GuiERT * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(collapsedRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(openRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
}


void GuiERTDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiERTDialog::change_adaptor()
{
	form_->changed();
}


/////////////////////////////////////////////////////////////////////
//
// GuiERT
//
/////////////////////////////////////////////////////////////////////

GuiERT::GuiERT(GuiDialog & parent)
	: GuiView<GuiERTDialog>(parent, _("TeX Code Settings"))
{
}


void GuiERT::build_dialog()
{
	dialog_.reset(new GuiERTDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
}


void GuiERT::applyView()
{
	if (dialog_->openRB->isChecked())
		controller().setStatus(Inset::Open);
	else
		controller().setStatus(Inset::Collapsed);
}


void GuiERT::update_contents()
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

#include "GuiERT_moc.cpp"
