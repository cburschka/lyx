/**
 * \file FormParagraph.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "paradlg.h"

#include "Dialogs.h"
#include "FormParagraph.h"
#include "Liason.h"
#include "gettext.h"
#include "buffer.h"
#include "QtLyXView.h"
#include "lyxtext.h"
#include "debug.h"

using Liason::setMinibuffer;
using std::endl;

FormParagraph::FormParagraph(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	d->showLayoutParagraph.connect(slot(this, &FormParagraph::show));
}


FormParagraph::~FormParagraph()
{
	delete dialog_;
}


void FormParagraph::update(bool switched)
{
	if (switched) {
		hide();
		return;
	}

	if (!lv_->view()->available())
		return;

	Buffer *buf = lv_->view()->buffer();
	
	if (readonly != buf->isReadonly()) {
		readonly = buf->isReadonly();
		dialog_->setReadOnly(readonly);
	}

	LyXText * text = 0;

	if (lv_->view()->theLockingInset())
		text = lv_->view()->theLockingInset()->getLyXText(lv_->view());

	if (!text)
		text = lv_->view()->text;

	LyXParagraph const * par = text->cursor.par();

	int align = par->GetAlign();
	
	if (align==LYX_ALIGN_LAYOUT)
		align = textclasslist.Style(buf->params.textclass, par->GetLayout()).align;

#if 0
	// Just remove this and change physpar to par
	LyXParagraph const * physpar = par;

	if (physpar->params.spaceTop().kind() == VSpace::LENGTH) {
		LyXGlueLength above = physpar->params.spaceTop().length();
		lyxerr[Debug::GUI] << "Reading above space : \"" 
			<< physpar->params.spaceTop().length().asString() << "\"" << endl;
 
		dialog_->setAboveLength(above.value(), above.plusValue(), above.minusValue(),
			above.unit(), above.plusUnit(), above.minusUnit());
	} else
		dialog_->setAboveLength(0.0, 0.0, 0.0, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE);

	if (physpar->params.spaceBottom().kind() == VSpace::LENGTH) {
		LyXGlueLength below = physpar->params.spaceBottom().length();
		lyxerr[Debug::GUI] << "Reading below space : \"" 
			<< physpar->params.spaceBottom().length().asString() << "\"" << endl;

		dialog_->setBelowLength(below.value(), below.plusValue(), below.minusValue(),
			below.unit(), below.plusUnit(), below.minusUnit());
	} else
		dialog_->setBelowLength(0.0, 0.0, 0.0, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE);

	dialog_->setLabelWidth(text->cursor.par()->GetLabelWidthString().c_str());
	dialog_->setAlign(align);

	dialog_->setChecks(physpar->params.lineTop(),
			   physpar->params.lineBottom(),
			   physpar->params.pagebreakTop(),
			   physpar->params.pagebreakBottom(),
			   physpar->params.noindent());
 
	dialog_->setSpace(physpar->params.spaceTop().kind(),
			  physpar->params.spaceBottom().kind(),
			  physpar->params.spaceTop().keep(),
			  physpar->params.spaceBottom().keep());
#else
	if (par->params.spaceTop().kind() == VSpace::LENGTH) {
		LyXGlueLength above = par->params.spaceTop().length();
		lyxerr[Debug::GUI] << "Reading above space : \"" 
			<< par->params.spaceTop().length().asString() << "\"" << endl;
 
		dialog_->setAboveLength(above.value(), above.plusValue(),
					above.minusValue(),
					above.unit(), above.plusUnit(),
					above.minusUnit());
	} else
		dialog_->setAboveLength(0.0, 0.0, 0.0,
					LyXLength::UNIT_NONE,
					LyXLength::UNIT_NONE,
					LyXLength::UNIT_NONE);

	if (par->params.spaceBottom().kind() == VSpace::LENGTH) {
		LyXGlueLength below = par->params.spaceBottom().length();
		lyxerr[Debug::GUI] << "Reading below space : \"" 
			<< below.asString() << "\"" << endl;

		dialog_->setBelowLength(below.value(),
					below.plusValue(),
					below.minusValue(),
					below.unit(),
					below.plusUnit(),
					below.minusUnit());
	} else
		dialog_->setBelowLength(0.0, 0.0, 0.0,
					LyXLength::UNIT_NONE,
					LyXLength::UNIT_NONE,
					LyXLength::UNIT_NONE);

	dialog_->setLabelWidth(text->cursor.par()->GetLabelWidthString());
	dialog_->setAlign(align);

	dialog_->setChecks(par->params.lineTop(),
			   par->params.lineBottom(),
			   par->params.pagebreakTop(),
			   par->params.pagebreakBottom(),
			   par->params.noindent());
 
	dialog_->setSpace(par->params.spaceTop().kind(),
			  par->params.spaceBottom().kind(),
			  par->params.spaceTop().keep(),
			  par->params.spaceBottom().keep());
#endif
}


void FormParagraph::apply()
{
	if (readonly)
		return;

	VSpace spaceabove;
	VSpace spacebelow;

	if (dialog_->getSpaceAboveKind() == VSpace::LENGTH)
		spaceabove = VSpace(dialog_->getAboveLength());
	else
		spaceabove = VSpace(dialog_->getSpaceAboveKind());

	if (dialog_->getSpaceBelowKind() == VSpace::LENGTH)
		spacebelow = VSpace(dialog_->getBelowLength());
	else
		spacebelow = VSpace(dialog_->getSpaceBelowKind());

	spaceabove.setKeep(dialog_->getAboveKeep());
	spacebelow.setKeep(dialog_->getBelowKeep());

	lyxerr[Debug::GUI] << "Setting above space \"" 
		<< LyXGlueLength(spaceabove.length().asString()).asString() << "\"" << endl;
	lyxerr[Debug::GUI] << "Setting below space \"" 
		<< LyXGlueLength(spacebelow.length().asString()).asString() << "\"" << endl;

	lv_->view()->text->SetParagraph(lv_->view(),
		dialog_->getLineAbove(), dialog_->getLineBelow(),
		dialog_->getPagebreakAbove(), dialog_->getPagebreakBelow(),
		spaceabove, spacebelow, dialog_->getAlign(),
		dialog_->getLabelWidth(), dialog_->getNoIndent());

	lv_->view()->update(lv_->view()->text, 
			    BufferView::SELECT |
			    BufferView::FITCUR |
			    BufferView::CHANGE);
	lv_->buffer()->markDirty();
	setMinibuffer(lv_, _("Paragraph layout set"));
}


void FormParagraph::show()
{
	if (!dialog_)
		dialog_ = new ParaDialog(this, 0,
					 _("LyX: Paragraph Options"), false);

	if (!dialog_->isVisible())
		h_ = d_->hideBufferDependent
			.connect(slot(this, &FormParagraph::hide));

	dialog_->raise();
	dialog_->setActiveWindow();
	update();
	 
	dialog_->show();
}


void FormParagraph::close()
{
	h_.disconnect();
}


void FormParagraph::hide()
{
	dialog_->hide();
	close();
}
