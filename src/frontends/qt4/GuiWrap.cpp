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

#include "ControlWrap.h"
#include "LengthCombo.h"
#include "qt_helpers.h"

#include "insets/InsetWrap.h"

#include "support/lstrings.h"

#include <QLineEdit>
#include <QCloseEvent>
#include <QPushButton>

using std::string;


namespace lyx {
namespace frontend {

GuiWrapDialog::GuiWrapDialog(LyXView & lv)
	: GuiDialog(lv, "wrap")
{
	setupUi(this);
	setViewTitle(_("Wrap Float Settings"));
	setController(new ControlWrap(*this));

	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(widthED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(unitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(valignCO, SIGNAL(highlighted(const QString &)),
		this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setRestore(restorePB);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	bc().addReadOnly(widthED);
	bc().addReadOnly(unitsLC);
	bc().addReadOnly(valignCO);
}


ControlWrap & GuiWrapDialog::controller()
{
	return static_cast<ControlWrap &>(GuiDialog::controller());
}


void GuiWrapDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiWrapDialog::change_adaptor()
{
	changed();
}


void GuiWrapDialog::applyView()
{
	double const value = widthED->text().toDouble();
	Length::UNIT unit = unitsLC->currentLengthItem();
	if (widthED->text().isEmpty())
		unit = Length::UNIT_NONE;

	InsetWrapParams & params = controller().params();

	params.width = Length(value, unit);

	switch (valignCO->currentIndex()) {
	case 0:
		params.placement = "o";
		break;
	case 1:
		params.placement = "i";
		break;
	case 2:
		params.placement = "l";
		break;
	case 3:
		params.placement = "r";
		break;
	}
}


void GuiWrapDialog::updateContents()
{
	InsetWrapParams & params = controller().params();

	Length len(params.width);
	//0pt is a legal width now, it yields a
	//wrapfloat just wide enough for the contents.
	widthED->setText(QString::number(len.value()));
	unitsLC->setCurrentItem(len.unit());

	int item = 0;
	if (params.placement == "i")
		item = 1;
	else if (params.placement == "l")
		item = 2;
	else if (params.placement == "r")
		item = 3;

	valignCO->setCurrentIndex(item);
}

} // namespace frontend
} // namespace lyx


#include "GuiWrap_moc.cpp"
