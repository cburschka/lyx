/**
 * \file QPreambleDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QPreambleDIAPreamble_H
#define QPreambleDIAPreamble_H

#include <config.h>

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
