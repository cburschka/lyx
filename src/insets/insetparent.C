/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1997-2000 The LyX Team.
 * 
 * ====================================================== */


// Created by asierra 970813

#include <config.h>


#ifdef __GNUG__
#pragma implementation
#endif

#include "insetparent.h"
#include "support/filetools.h"
#include "BufferView.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "commandtags.h"


InsetParent::InsetParent(string const & fn, Buffer * owner)
	: InsetCommand("lyxparent")
{
	if (owner)
		setContents(MakeAbsPath(fn, OnlyPath(owner->fileName())));
	else
		setContents(fn);
}


void InsetParent::Edit(BufferView * bv, int, int, unsigned int)
{    
	bv->owner()->getLyXFunc()->
		Dispatch(LFUN_CHILDOPEN, getContents().c_str());
}


// LaTeX must just ignore this command
int InsetParent::Latex(ostream & os, signed char fragile, bool free_spc) const
{
	os << "%%#{lyx}";
	InsetCommand::Latex(os, fragile, free_spc);
	return 0;
}


#ifndef USE_OSTREAM_ONLY
// LaTeX must just ignore this command
int InsetParent::Latex(string & file, signed char fragile, bool free_spc) const
{
	file += "%%#{lyx}";
	InsetCommand::Latex(file, fragile, free_spc);
	return 0;
}
#endif
