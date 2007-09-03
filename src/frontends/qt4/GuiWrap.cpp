/**
 * \file GuiWrap.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiWrap.h"

#include "LengthCombo.h"
#include "qt_helpers.h"

#include "insets/InsetWrap.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include <QLineEdit>
#include <QCloseEvent>
#include <QPushButton>


using std::string;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiWrapDialog
//
/////////////////////////////////////////////////////////////////////


GuiWrapDialog::GuiWrapDialog(GuiWrap * form)
	: form_(form)
{
	setupUi(this);

	connect(restorePB, SIGNAL(clicked()), form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

	connect(widthED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(unitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(valignCO, SIGNAL(highlighted(const QString &)),
		this, SLOT(change_adaptor()));
}


void GuiWrapDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiWrapDialog::change_adaptor()
{
	form_->changed();
}

/////////////////////////////////////////////////////////////////////
//
// GuiWrap
//
/////////////////////////////////////////////////////////////////////

GuiWrap::GuiWrap(GuiDialog & parent)
	: GuiView<GuiWrapDialog>(parent, _("Text Wrap Settings"))
{
}


void GuiWrap::build_dialog()
{
	dialog_.reset(new GuiWrapDialog(this));

	bc().setRestore(dialog_->restorePB);
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);

	bc().addReadOnly(dialog_->widthED);
	bc().addReadOnly(dialog_->unitsLC);
	bc().addReadOnly(dialog_->valignCO);
}


void GuiWrap::apply()
{
	double const value = convert<double>(fromqstr(dialog_->widthED->text()));
	Length::UNIT unit = dialog_->unitsLC->currentLengthItem();
	if (dialog_->widthED->text().isEmpty())
		unit = Length::UNIT_NONE;

	InsetWrapParams & params = controller().params();

	params.width = Length(value, unit);

	switch (dialog_->valignCO->currentIndex()) {
	case 0:
		params.placement.erase();
		break;
	case 1:
		params.placement = "l";
		break;
	case 2:
		params.placement = "r";
		break;
	case 3:
		params.placement = "p";
		break;
	}
}


static string const numtostr(double val)
{
	string a = convert<string>(val);
	// FIXME: Will this test ever trigger? (Lgb)
	if (a == "0")
		a.erase();
	return a;
}


void GuiWrap::update_contents()
{
	InsetWrapParams & params = controller().params();

	Length len(params.width);
	dialog_->widthED->setText(toqstr(numtostr(len.value())));
	dialog_->unitsLC->setCurrentItem(len.unit());

	int item = 0;
	if (params.placement == "l")
		item = 1;
	else if (params.placement == "r")
		item = 2;
	else if (params.placement == "p")
		item = 3;

	dialog_->valignCO->setCurrentIndex(item);
}

} // namespace frontend
} // namespace lyx


#include "GuiWrap_moc.cpp"
