/**
 * \file QParagraph.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include "QParagraphDialog.h"

#include "QParagraph.h"
#include "Dialogs.h"
#include "Liason.h"
#include "gettext.h"
#include "buffer.h"
#include "QtLyXView.h"
#include "lyxtextclasslist.h"
#include "lyxtext.h"
#include "debug.h"
#include "BufferView.h"
#include "ParagraphParameters.h"

using SigC::slot;
using Liason::setMinibuffer;
using std::endl;


QParagraph::QParagraph(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	d->showParagraph.connect(slot(this, &QParagraph::show));
}


QParagraph::~QParagraph()
{
	delete dialog_;
}


void QParagraph::update(bool switched)
{
	if (switched) {
		hide();
		return;
	}

	if (!lv_->view()->available())
		return;

	Buffer * buf = lv_->view()->buffer();

	if (readonly!=buf->isReadonly()) {
		readonly = buf->isReadonly();
		dialog_->setReadOnly(readonly);
	}

	LyXText * text(lv_->view()->getLyXText());
	Paragraph * par = text->cursor.par();

	int align = par->getAlign();

	if (align==LYX_ALIGN_LAYOUT)
		align = textclasslist.Style(buf->params.textclass, par->getLayout()).align;

	ParagraphParameters * params = &(par->params());

	if (params->spaceTop().kind() == VSpace::LENGTH) {
		LyXGlueLength above = params->spaceTop().length();
		lyxerr[Debug::GUI] << "Reading above space : \"" << params->spaceTop().length().asString() << "\"" << endl;
		dialog_->setAboveLength(above.len().value(), above.plus().value(), above.minus().value(),
			above.len().unit(), above.plus().unit(), above.minus().unit());
	} else
		dialog_->setAboveLength(0.0, 0.0, 0.0, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE);

	if (params->spaceBottom().kind() == VSpace::LENGTH) {
		LyXGlueLength below = params->spaceBottom().length();
		lyxerr[Debug::GUI] << "Reading below space : \"" << params->spaceBottom().length().asString() << "\"" << endl;
		dialog_->setBelowLength(below.len().value(), below.plus().value(), below.minus().value(),
			below.len().unit(), below.plus().unit(), below.minus().unit());
	} else
		dialog_->setBelowLength(0.0, 0.0, 0.0, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE, LyXLength::UNIT_NONE);

	dialog_->setLabelWidth(text->cursor.par()->getLabelWidthString().c_str());
	dialog_->setAlign(align);
	dialog_->setChecks(params->lineTop(), params->lineBottom(),
		params->pagebreakTop(), params->pagebreakBottom(), params->noindent());
	dialog_->setSpace(params->spaceTop().kind(), params->spaceBottom().kind(),
		params->spaceTop().keep(), params->spaceBottom().keep());

	// now the extras page

	LyXLength extrawidth;
	float val = 0.0;
	LyXLength::UNIT unit = LyXLength::CM;
	params = &(par->params());
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
		static_cast<Paragraph::PEXTRA_TYPE>(params->pextraType()));
}


void QParagraph::apply()
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

	LyXText * text(lv_->view()->getLyXText());
	text->setParagraph(lv_->view(),
			   dialog_->getLineAbove(), dialog_->getLineBelow(),
			   dialog_->getPagebreakAbove(), dialog_->getPagebreakBelow(),
			   spaceabove, spacebelow, Spacing(), dialog_->getAlign(),
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

	lv_->view()->update(text,
			    BufferView::SELECT |
			    BufferView::FITCUR |
			    BufferView::CHANGE);

	lv_->buffer()->markDirty();
	setMinibuffer(lv_, _("Paragraph layout set"));
}


void QParagraph::show()
{
	if (!dialog_)
		dialog_ = new QParagraphDialog(this, 0,
					       _("LyX: Paragraph Settings"),
					       false);

	if (!dialog_->isVisible())
		h_ = d_->hideBufferDependent
			.connect(slot(this, &QParagraph::hide));

	dialog_->raise();
	dialog_->setActiveWindow();
	update();

	dialog_->show();
}


void QParagraph::close()
{
	h_.disconnect();
}


void QParagraph::hide()
{
	dialog_->hide();
	close();
}
