/**
 * \file citationdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef CITATIONDIALOG_H
#define CITATIONDIALOG_H

#include "FormCitation.h" 
#include "dlg/citationdlgdata.h"
#include "controllers/ButtonPolicies.h" 

class CitationDialog : public CitationDialogData
{
	Q_OBJECT

public:
	CitationDialog(FormCitation * form, QWidget * parent = 0, char const * name = 0);

private slots:
	virtual void clicked_ok() { form_->OKButton(); };
	virtual void clicked_apply() { form_->ApplyButton(); };
	virtual void clicked_restore() { form_->RestoreButton(); form_->updateButtons(); };
	virtual void clicked_cancel() { form_->CancelButton(); };
 
	virtual void clicked_add() {
		form_->bc().input(form_->add());
		form_->updateButtons(); 
	}

	virtual void clicked_up() {
		form_->bc().input(form_->up());
		form_->updateButtons(); 
	}

	virtual void clicked_down() {
		form_->bc().input(form_->down());
		form_->updateButtons(); 
	}

	virtual void clicked_remove() {
		form_->bc().input(form_->remove());
		form_->updateButtons(); 
	}

	virtual void clicked_search();
 
	virtual void select_available_adaptor(const char * key) {
		form_->bc().input(form_->select_key(key));
		form_->updateButtons(); 
	}

	virtual void highlight_available_adaptor(const char * key) {
		form_->highlight_key(key);
	}

	virtual void highlight_chosen_adaptor(const char * key) {
		form_->highlight_chosen(key);
	}

	virtual void after_changed(const char *) {
		form_->bc().valid(true);
		form_->updateButtons(); 
	}

protected slots:
	virtual void closeEvent(QCloseEvent *e);

private:
	FormCitation * form_;

};
#endif // CITATIONDIALOG_H
