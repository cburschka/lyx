/**
 * \file indexdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef INDEXDIALOG_H
#define INDEXDIALOG_H

#include "dlg/indexdlgdata.h"

class FormIndex;

class IndexDialog : public IndexDialogData
{
Q_OBJECT

public:
	IndexDialog (FormIndex * form, QWidget * parent = NULL, char const * name = NULL);
	virtual ~IndexDialog();

	void setIndexText(char const * str) { index->setText(str); }
	char const * getIndexText() { return index->text(); }
	void setReadOnly(bool);

private slots:
	void clickedOK();
	void clickedCancel();

private:
	FormIndex * form_;

};
#endif // INDEXDIALOG_H
