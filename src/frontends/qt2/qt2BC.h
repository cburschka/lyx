// -*- C++ -*-
/**
 * \file qt2BC.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author Baruch Even, baruch.even@writeme.com
 */

#ifndef QT2BC_H
#define QT2BC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonController.h"

class QWidget;
class QButton;

/** General purpose button controller for up to four buttons.
    Controls the activation of the OK, Apply and Cancel buttons.
    Actually supports 4 buttons in all and it's up to the user to decide on
    the activation policy and which buttons correspond to which output of the
    state machine.
*/
class qt2BC : public GuiBC<QButton, QWidget>
{
public:
	///
	qt2BC(string const &, string const &);

private:
	/// Updates the button sensitivity (enabled/disabled)
	void setButtonEnabled(QButton *, bool enabled);

	/// Updates the widget sensitivity (enabled/disabled)
	void setWidgetEnabled(QWidget *, bool enabled);

	/// Set the label on the button
	void setButtonLabel(QButton *, string const & label);
};

#endif // QT2BC_H
