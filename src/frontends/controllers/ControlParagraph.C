// -*- C++ -*-
/**
 * \file ControlParagraph.C
 * Copyright 2002 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlParagraph.h"
#include "ButtonControllerBase.h"
#include "ViewBase.h"
#include "ParagraphParameters.h"
#include "Liason.h"
#include "LyXView.h"
#include "BufferView.h"
#include "gettext.h"
#include "buffer.h"
#include "lyxtext.h"
#include "support/LAssert.h"
#include "Dialogs.h"
#include <boost/bind.hpp>

using Liason::setMinibuffer;


ControlParagraph::ControlParagraph(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d), pp_(0), ininset_(false)
{
     d_.updateParagraph.connect(
	     boost::bind(&ControlParagraph::changedParagraph, this));

}


ControlParagraph::~ControlParagraph()
{}


ParagraphParameters & ControlParagraph::params()
{
	lyx::Assert(pp_.get());
	return *pp_;
}


bool ControlParagraph::inInset() const
{
	return ininset_;
}


LyXAlignment ControlParagraph::alignPossible() const
{
	return alignpos_;
}


void ControlParagraph::apply()
{
	if (!lv_.view()->available())
		return;

	view().apply();

	LyXText * text(lv_.view()->getLyXText());
	text->setParagraph(lv_.view().get(),
			   pp_->lineTop(),
			   pp_->lineBottom(),
			   pp_->pagebreakTop(),
			   pp_->pagebreakBottom(),
			   pp_->spaceTop(),
			   pp_->spaceBottom(),
			   pp_->spacing(),
			   pp_->align(),
			   pp_->labelWidthString(),
			   pp_->noindent());

	// Actually apply these settings
	lv_.view()->update(text,
			   BufferView::SELECT |
			   BufferView::FITCUR |
			   BufferView::CHANGE);

	lv_.buffer()->markDirty();

	setMinibuffer(&lv_, _("Paragraph layout set"));
}


void ControlParagraph::setParams()
{
	if (!pp_.get())
		pp_.reset(new ParagraphParameters());

	/// get paragraph
	Paragraph const * par_ = lv_.view()->getLyXText()->cursor.par();

	/// Set the paragraph parameters
	*pp_ = par_->params();

	/// this needs to be done separately
	pp_->labelWidthString(par_->getLabelWidthString());

	/// alignment
	LyXLayout_ptr const & layout = par_->layout();
	if (pp_->align() == LYX_ALIGN_LAYOUT)
		pp_->align(layout->align);

	/// is alignment possible
	alignpos_ = layout->alignpossible;

	/// is paragraph in inset
	ininset_ = par_->inInset();
}

void ControlParagraph::changedParagraph()
{
	/// get paragraph
	Paragraph const * p = lv_.view()->getLyXText()->cursor.par();

	if (p == 0) // this is wrong as we don't set par_ here! /* || p == par_) */
		return;

	// For now, don't bother checking if the params are different.
	// Will the underlying paragraph accept our changes?
	Inset * const inset = p->inInset();
	bool const accept = !(inset && inset->forceDefaultParagraphs(inset));
	bc().valid(accept);
}
