/**
 * \file QCharacterDialog.h
 * Copyright 2001 the LyX Team
 * see the file COPYING
 *
 * \author Edwin Leuven
 */

#ifndef QCHARACTERDIALOG_H
#define QCHARACTERDIALOG_H

#include <config.h>

#include "ui/QCharacterDialogBase.h"
#include "QCharacter.h"

#include <qevent.h>
#include <qcheckbox.h>
#include <qcombobox.h>

class LyXFont;
 
class QCharacterDialog : public QCharacterDialogBase
{ Q_OBJECT

public:
	QCharacterDialog(QCharacter * form, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QCharacterDialog();
	
	/// FIXME
	LyXFont getChar();
	
	/// return the currently selected language item
	int langItem() {
		return lang->currentItem();
	};
	
	/// return true if toggle all is selected
	bool toggleAll() {
		return toggleall->isChecked();
	};
	
	/// set dialog read only
	void setReadOnly(bool);
	
protected:
	void closeEvent(QCloseEvent * e);

private:
	QCharacter * form_;

protected slots:
	
	void apply_adaptor() {
		form_->apply();
	}
	
	void cancel_adaptor() {
		form_->close();
		hide();
	}
	
	void close_adaptor() {
		form_->apply();
		form_->close();
		hide();
	}

};

#endif // QCHARACTERDIALOG_H
