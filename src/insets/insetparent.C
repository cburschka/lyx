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
#include "buffer.h"
#include "gettext.h"

using std::ostream;

InsetParent::InsetParent(InsetCommandParams const & p, Buffer * bf)
	: InsetCommand(p)
{
	string fn = p.getContents();
	if (bf)
		setContents(MakeAbsPath(fn, OnlyPath(bf->fileName())));
	else
		setContents(fn);
}


string InsetParent::getScreenLabel() const 
{
	return string(_("Parent:")) + getContents();
}


void InsetParent::Edit(BufferView * bv, int, int, unsigned int)
{    
	bv->owner()->getLyXFunc()->
		Dispatch(LFUN_CHILDOPEN, getContents().c_str());
}


// LaTeX must just ignore this command
int InsetParent::Latex(Buffer const * buf, ostream & os,
		       bool fragile, bool free_spc) const
{
	os << "%%#{lyx}";
	InsetCommand::Latex(buf, os, fragile, free_spc);
	return 0;
}
