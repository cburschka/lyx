/**
 * \file kdeBC.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "kdeBC.h"
#include <qpushbutton.h>

kdeBC::kdeBC(string const & cancel, string const & close)
	: GuiBC<QPushButton, QWidget>(cancel, close)
{}


void kdeBC::setButtonEnabled(QPushButton * btn, bool enabled)
{
	btn->setEnabled(enabled);
}


void kdeBC::setWidgetEnabled(QWidget * obj, bool enabled)
{
	obj->setEnabled(enabled);
}


void kdeBC::setButtonLabel(QPushButton * btn, string const & label)
{
	btn->setText(label.c_str());
}
