#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "kdeBC.h"
#include <qpushbutton.h>

kdeBC::kdeBC(string const & cancel, string const & close)
	: GuiBC<QPushButton, QWidget>(cancel, close)
{}


void setButtonEnabled(QPushButton * btn, bool enabled)
{
	btn->setEnabled(true);
}


void setWidgetEnabled(QWidget * obj, bool enabled)
{
	obj->setEnabled(true);
}


void setButtonLabel(QPushButton * btn, string const & label)
{
	btn->setText(label.c_str());
}
