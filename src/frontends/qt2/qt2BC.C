/**
 * \file qt2BC.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae <rae@lyx.org>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "qt2BC.h"

#include <qbutton.h>

qt2BC::qt2BC(string const & cancel, string const & close)
	: GuiBC<QButton, QWidget>(cancel, close)
{}


void qt2BC::setButtonEnabled(QButton * obj, bool enabled)
{
	obj->setEnabled(enabled);
}


void qt2BC::setWidgetEnabled(QWidget * obj, bool enabled)
{
	obj->setEnabled(enabled);
}


void qt2BC::setButtonLabel(QButton * obj, string const & label)
{
	obj->setText(label.c_str());
}
