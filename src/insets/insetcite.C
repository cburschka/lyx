// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000 The LyX Team.
 * 
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcite.h"
#include "LyXView.h"
#include "BufferView.h"
#include "frontends/Dialogs.h"

InsetCitation::InsetCitation(string const & key, string const & note)
		: InsetCommand("cite", key, note), dialogs_(0)
{
}

InsetCitation::~InsetCitation()
{
	if( dialogs_ != 0 )
		dialogs_->hideCitation( this );
}

string InsetCitation::getScreenLabel() const
{
	string temp("[");

	temp += getContents();

	if( !getOptions().empty() ) {
		temp += ", " + getOptions();
	}

	return temp + ']';
}

void InsetCitation::Edit(BufferView * bv, int, int, unsigned int)
{
	dialogs_ = bv->owner()->getDialogs();
	dialogs_->showCitation( this );
}

