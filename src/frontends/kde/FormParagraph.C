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

#ifdef CXX_WORKING_NAMESPACES
using Liason::setMinibuffer;
#endif

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

#ifndef NEW_INSETS
	LyXParagraph const * physpar = par->FirstPhysicalPar();
#else
	LyXParagraph const * physpar = par;
#endif

	if (physpar->added_space_top.kind() == VSpace::LENGTH) {
		LyXGlueLength above = physpar->added_space_top.length();
		lyxerr[Debug::GUI] << "Reading above space : \"" 
			<< physpar->added_space_top.length().asString() << "\"" << endl;
 
		dialog_->setAboveLength(above.value(), above.plusValue(), above.minusValue(),
			above.unit(), above.plusUnit(), above.minusUnit());
	} else
		dialog_->setAboveLength(0.0, 0.0, 0.0, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE);

	if (physpar->added_space_bottom.kind() == VSpace::LENGTH) {
		LyXGlueLength below = physpar->added_space_bottom.length();
		lyxerr[Debug::GUI] << "Reading below space : \"" 
			<< physpar->added_space_bottom.length().asString() << "\"" << endl;

		dialog_->setBelowLength(below.value(), below.plusValue(), below.minusValue(),
			below.unit(), below.plusUnit(), below.minusUnit());
	} else
		dialog_->setBelowLength(0.0, 0.0, 0.0, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE);

	dialog_->setLabelWidth(text->cursor.par()->GetLabelWidthString().c_str());
	dialog_->setAlign(align);

	dialog_->setChecks(physpar->line_top, physpar->line_bottom,
		physpar->pagebreak_top, physpar->pagebreak_bottom, physpar->noindent);
 
	dialog_->setSpace(physpar->added_space_top.kind(), physpar->added_space_bottom.kind(),
		physpar->added_space_top.keep(), physpar->added_space_bottom.keep());

	// now the extras page

	LyXLength extrawidth;
	float val = 0.0;
	LyXLength::UNIT unit = LyXLength::CM;

	if (isValidLength(par->pextra_width, &extrawidth)) {
		lyxerr[Debug::GUI] << "Reading extra width \"" << extrawidth.asString() << "\"" << endl;
		val = extrawidth.value();
		unit = extrawidth.unit();
	}

	lyxerr[Debug::GUI] << "Reading widthp \"" << par->pextra_widthp << "\"" << endl;

	dialog_->setExtra(val, unit, par->pextra_widthp,
		par->pextra_alignment,
		par->pextra_hfill,
		par->pextra_start_minipage,
		static_cast<LyXParagraph::PEXTRA_TYPE>(par->pextra_type));
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

	// extra stuff

	string width("");
	string widthp("");

	LyXLength extrawidth(dialog_->getExtraWidth());
	if (extrawidth.unit() == LyXLength::UNIT_NONE) {
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

	lv_->view()->update(BufferView::SELECT | BufferView::FITCUR | BufferView::CHANGE);
	lv_->buffer()->markDirty();
	setMinibuffer(lv_, _("Paragraph layout set"));
}


void FormParagraph::show()
{
	if (!dialog_)
		dialog_ = new ParaDialog(this, 0, _("LyX: Paragraph Options"), false);

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
