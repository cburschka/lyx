/**
 * \file FormParagraph.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include "paragraphdlgimpl.h"

#include "FormParagraph.h"
#include "Dialogs.h"
#include "Liason.h"
#include "gettext.h"
#include "buffer.h"
#include "QtLyXView.h"
#include "lyxtext.h"
#include "debug.h"

#ifdef CXX_WORKING_NAMESPACES
using Liason::setMinibuffer;
#endif

using std::endl;

FormParagraph::FormParagraph(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
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
	
	if (readonly!=buf->isReadonly()) {
		readonly = buf->isReadonly();
		dialog_->setReadOnly(readonly);
	}

	LyXText *text = 0;

	if (lv_->view()->theLockingInset())
		text = lv_->view()->theLockingInset()->getLyXText(lv_->view());

	if (!text)
		text = lv_->view()->text;

	LyXParagraph *par = text->cursor.par();

	int align = par->GetAlign();
	
	if (align==LYX_ALIGN_LAYOUT)
		align = textclasslist.Style(buf->params.textclass, par->GetLayout()).align;

#ifndef NEW_INSETS
	ParagraphParameters *params = &(par->FirstPhysicalPar()->params);
#else
	ParagraphParameters *params = &(par->params);
#endif
    
	if (params->spaceTop().kind() == VSpace::LENGTH) {
		LyXGlueLength above = params->spaceTop().length();
		lyxerr[Debug::GUI] << "Reading above space : \"" << params->spaceTop().length().asString() << "\"" << endl;
		dialog_->setAboveLength(above.value(), above.plusValue(), above.minusValue(),
			above.unit(), above.plusUnit(), above.minusUnit());
	} else
		dialog_->setAboveLength(0.0, 0.0, 0.0, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE);

	if (params->spaceBottom().kind() == VSpace::LENGTH) {
		LyXGlueLength below = params->spaceBottom().length();
		lyxerr[Debug::GUI] << "Reading below space : \"" << params->spaceBottom().length().asString() << "\"" << endl;
		dialog_->setBelowLength(below.value(), below.plusValue(), below.minusValue(),
			below.unit(), below.plusUnit(), below.minusUnit());
	} else
		dialog_->setBelowLength(0.0, 0.0, 0.0, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE);

	dialog_->setLabelWidth(text->cursor.par()->GetLabelWidthString().c_str());
	dialog_->setAlign(align);
	dialog_->setChecks(params->lineTop(), params->lineBottom(),
		params->pagebreakTop(), params->pagebreakBottom(), params->noindent());
	dialog_->setSpace(params->spaceTop().kind(), params->spaceBottom().kind(),
		params->spaceTop().keep(), params->spaceBottom().keep());

	// now the extras page

	LyXLength extrawidth;
	float val = 0.0;
	LyXLength::UNIT unit = LyXLength::CM;
	params = &(par->params);

	if (isValidLength(params->pextraWidth(), &extrawidth)) {
		lyxerr[Debug::GUI] << "Reading extra width \"" << extrawidth.asString() << "\"" << endl;
		val = extrawidth.value();
		unit = extrawidth.unit();
	}

	lyxerr[Debug::GUI] << "Reading widthp \"" << params->pextraWidthp() << "\"" << endl;

	dialog_->setExtra(val, unit, params->pextraWidthp(),
		params->pextraAlignment(),
		params->pextraHfill(),
		params->pextraStartMinipage(),
		static_cast<LyXParagraph::PEXTRA_TYPE>(params->pextraType()));
}

void FormParagraph::apply()
{
	if (readonly)
		return;

	VSpace spaceabove;
	VSpace spacebelow;

	if (dialog_->getSpaceAboveKind()==VSpace::LENGTH)
		spaceabove = VSpace(dialog_->getAboveLength());
	else
		spaceabove = VSpace(dialog_->getSpaceAboveKind());

	if (dialog_->getSpaceBelowKind()==VSpace::LENGTH)
		spacebelow = VSpace(dialog_->getBelowLength());
	else
		spacebelow = VSpace(dialog_->getSpaceBelowKind());

	spaceabove.setKeep(dialog_->getAboveKeep());
	spacebelow.setKeep(dialog_->getBelowKeep());

	lyxerr[Debug::GUI] << "Setting above space \"" << LyXGlueLength(spaceabove.length().asString()).asString() << "\"" << endl;
	lyxerr[Debug::GUI] << "Setting below space \"" << LyXGlueLength(spacebelow.length().asString()).asString() << "\"" << endl;

	lv_->view()->text->SetParagraph(lv_->view(),
		dialog_->getLineAbove(), dialog_->getLineBelow(),
		dialog_->getPagebreakAbove(), dialog_->getPagebreakBelow(),
		spaceabove, spacebelow, dialog_->getAlign(),
		dialog_->getLabelWidth(), dialog_->getNoIndent());

	// extra stuff

	string width("");
	string widthp("");

	LyXLength extrawidth(dialog_->getExtraWidth());
	if (extrawidth.unit()==LyXLength::UNIT_NONE) {
		widthp = dialog_->getExtraWidthPercent();
	} else
		width = extrawidth.asString();

	lyxerr[Debug::GUI] << "Setting extrawidth \"" << width << "\"" << endl;
	lyxerr[Debug::GUI] << "Setting percent extrawidth \"" << widthp << "\"" << endl;

	lv_->view()->text->SetParagraphExtraOpt(lv_->view(),
		dialog_->getExtraType(), width, widthp,
		dialog_->getExtraAlign(),
		dialog_->getHfillBetween(),
		dialog_->getStartNewMinipage());

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
		dialog_ = new ParagraphDlgImpl(this, 0, _("LyX: Paragraph Options"), false);

	if (!dialog_->isVisible())
		h_ = d_->hideBufferDependent.connect(slot(this, &FormParagraph::hide));

	 
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
