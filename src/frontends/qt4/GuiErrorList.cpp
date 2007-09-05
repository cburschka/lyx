/**
 * \file GuiErrorList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiErrorList.h"
#include "ControlErrorList.h"

#include "qt_helpers.h"

#include <QListWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {


GuiErrorListDialog::GuiErrorListDialog(LyXView & lv)
	: GuiDialog(lv, "errorlist")
{
	setupUi(this);
	setController(new ControlErrorList(*this));

	connect(closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));
	connect(errorsLW, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(slotClose()));
	connect( errorsLW, SIGNAL(itemClicked(QListWidgetItem *)),
		this, SLOT(select_adaptor(QListWidgetItem *)));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);
	bc().setCancel(closePB);
}


ControlErrorList & GuiErrorListDialog::controller() const
{
	return static_cast<ControlErrorList &>(Dialog::controller());
}


void GuiErrorListDialog::select_adaptor(QListWidgetItem * item)
{
	select(item);
}


void GuiErrorListDialog::closeEvent(QCloseEvent * e)
{
	slotWMHide();
	e->accept();
}


void GuiErrorListDialog::showEvent(QShowEvent *e)
{
	errorsLW->setCurrentRow(0);
	select(errorsLW->item(0));
	e->accept();
}


void GuiErrorListDialog::select(QListWidgetItem * wi)
{
	int const item = errorsLW->row(wi);
	controller().goTo(item);
	descriptionTB->setPlainText(toqstr(controller().errorList()[item].description));
}


void GuiErrorListDialog::update_contents()
{
	setViewTitle(from_utf8(controller().name()));
	errorsLW->clear();
	descriptionTB->setPlainText(QString());

	ErrorList::const_iterator it = controller().errorList().begin();
	ErrorList::const_iterator end = controller().errorList().end();
	for (; it != end; ++it)
		errorsLW->addItem(toqstr(it->error));
}

} // namespace frontend
} // namespace lyx


#include "GuiErrorList_moc.cpp"
