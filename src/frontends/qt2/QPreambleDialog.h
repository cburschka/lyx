// -*- C++ -*-
/**
 * \file QPreambleDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QPreambleDIAPreamble_H
#define QPreambleDIAPreamble_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QPreambleDialogBase.h"

class QPreamble;

class QPreambleDialog : public QPreambleDialogBase
{ Q_OBJECT

public:
	QPreambleDialog(QPreamble * form);

protected slots:
	virtual void editClicked();
	virtual void change_adaptor();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QPreamble * form_;
};

#endif // QPreambleDIAPreamble_H
