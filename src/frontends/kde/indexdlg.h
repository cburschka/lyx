/**
 * \file indexdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef INDEXDIALOG_H
#define INDEXDIALOG_H

#include "FormIndex.h" 
#include "dlg/indexdlgdata.h"

// FIXME: closeEvent 

class IndexDialog : public IndexDialogData
{
Q_OBJECT

public:
	IndexDialog (FormIndex * form, QWidget * parent = NULL, char const * name = NULL);

	void setIndexText(char const * str) { line_index->setText(str); }
	char const * getIndexText() { return line_index->text(); }

private slots:
	void clickedOK() { form_->OKButton(); };
	void clickedCancel() { form_->CancelButton(); };

private:
	FormIndex * form_;

};
#endif // INDEXDIALOG_H
