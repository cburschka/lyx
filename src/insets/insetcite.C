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

InsetCitation::InsetCitation(string const & key, string const & note)
		: InsetCommand("cite", key, note)
{
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
