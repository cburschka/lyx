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
#include "ControlERT.h"
#include "gettext.h"

#include <QRadioButton>
#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

GuiERTDialog::GuiERTDialog(LyXView & lv)
	: GuiDialog(lv, "ert")
{
	setupUi(this);
	setViewTitle(_("TeX Code Settings"));
	setController(new ControlERT(*this));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(collapsedRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(openRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
}


ControlERT & GuiERTDialog::controller() const
{
	return static_cast<ControlERT &>(GuiDialog::controller());
}


void GuiERTDialog::closeEvent(QCloseEvent * e)
{
	slotWMHide();
	e->accept();
}


void GuiERTDialog::change_adaptor()
{
	changed();
}


void GuiERTDialog::applyView()
{
	if (openRB->isChecked())
		controller().setStatus(Inset::Open);
	else
		controller().setStatus(Inset::Collapsed);
}


void GuiERTDialog::update_contents()
{
	switch (controller().status()) {
		case InsetERT::Open: openRB->setChecked(true); break;
		case InsetERT::Collapsed: collapsedRB->setChecked(true); break;
	}
}

} // namespace frontend
} // namespace lyx

#include "GuiERT_moc.cpp"
