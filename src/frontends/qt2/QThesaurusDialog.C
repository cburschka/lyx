/**
 * \file QThesaurusDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <vector>

#include "LString.h" 
 
#include "ControlThesaurus.h" 
#include "QThesaurusDialog.h"
#include "Dialogs.h"
#include "QThesaurus.h"

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h> 

QThesaurusDialog::QThesaurusDialog(QThesaurus * form)
	: QThesaurusDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}

 
void QThesaurusDialog::change_adaptor()
{
	form_->changed();
}

 
void QThesaurusDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QThesaurusDialog::entryChanged()
{
	updateLists();
}

 
void QThesaurusDialog::replaceClicked()
{
	form_->replace();
}

 
void QThesaurusDialog::selectionChanged(const QString & str)
{
	string const entry(str.latin1());
	entryED->setText(entry.c_str());
	updateLists();
}

 
void QThesaurusDialog::updateLists()
{
	ControlThesaurus & control(form_->controller());
	string const entry(entryED->text().latin1());

	std::vector<string> matches;

	matches = control.getNouns(entry);
	for (std::vector<string>::const_iterator cit = matches.begin();
		cit != matches.end(); ++cit)
		nounsLB->insertItem(cit->c_str());
	matches = control.getVerbs(entry);
	for (std::vector<string>::const_iterator cit = matches.begin();
		cit != matches.end(); ++cit)
		verbsLB->insertItem(cit->c_str());
	matches = control.getAdjectives(entry);
	for (std::vector<string>::const_iterator cit = matches.begin();
		cit != matches.end(); ++cit)
		adjectivesLB->insertItem(cit->c_str());
	matches = control.getAdverbs(entry);
	for (std::vector<string>::const_iterator cit = matches.begin();
		cit != matches.end(); ++cit)
		adverbsLB->insertItem(cit->c_str());
	matches = control.getOthers(entry);
	for (std::vector<string>::const_iterator cit = matches.begin();
		cit != matches.end(); ++cit)
		otherLB->insertItem(cit->c_str());
}
