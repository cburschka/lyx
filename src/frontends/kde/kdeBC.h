// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ======================================================
 *
 * Author: Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef KDEBC_H
#define KDEBC_H

#include <list>

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonControllerBase.h"
#include "ButtonController.h"

class QWidget;
class QPushButton;

class kdeBC : public GuiBC<QPushButton, Qwidget>
{
public:
	///
	kdeBC(string const & cancel, string const & close);

private:
	/// Updates the button sensitivity (enabled/disabled)
	void setButtonEnabled(QPushButton * btn, bool enabled);

	/// Updates the widget sensitivity (enabled/disabled)
	void setWidgetEnabled(QWidget * obj, bool enabled);

	/// Set the label on the button
	void setButtonLabel(QPushButton * btn, string const & label);
};

#endif // KDEBC_H
