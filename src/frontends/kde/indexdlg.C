/**
 * \file indexdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <gettext.h>

#include "FormIndex.h"
#include "indexdlg.h"

#include <qtooltip.h>

IndexDialog::IndexDialog(FormIndex * f, QWidget * p, char const * name)
	: IndexDialogData(p, name), form_(f)
{
	setCaption(name);

	// tooltips

	QToolTip::add(label_index,_("Index entry"));
	QToolTip::add(line_index,_("Index entry"));

	setMinimumSize(200, 65);
	setMaximumSize(32767, 65);
}
