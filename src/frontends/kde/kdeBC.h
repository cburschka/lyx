/**
 * \file kdeBC.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, <a.leeming@ic.ac.uk>
 */

#ifndef KDEBC_H
#define KDEBC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonControllerBase.h"
#include "ButtonController.h"

class QWidget;
class QPushButton;

class kdeBC : public GuiBC<QPushButton, QWidget>
{
public:
	///
	kdeBC(string const & cancel, string const & close);

private:
	/// enable or disable button
	void setButtonEnabled(QPushButton * btn, bool enabled);

	/// enable or disable button
	void setWidgetEnabled(QWidget * obj, bool enabled);

	/// set the label on the button
	void setButtonLabel(QPushButton * btn, string const & label);
};

#endif // KDEBC_H
