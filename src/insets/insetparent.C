/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1997-2001 The LyX Team.
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
#include "frontends/LyXView.h"
#include "support/LOstream.h"
#include "commandtags.h"
#include "buffer.h"
#include "gettext.h"
#include "lyxfunc.h"

using std::ostream;


InsetParent::InsetParent(InsetCommandParams const & p, Buffer const & bf, bool)
	: InsetCommand(p)
{
	string const fn = p.getContents();
	setContents(MakeAbsPath(fn, bf.filePath()));
}


string const InsetParent::getScreenLabel(Buffer const *) const
{
	return string(_("Parent:")) + getContents();
}


void InsetParent::edit(BufferView * bv, int, int, mouse_button::state)
{    
	bv->owner()->getLyXFunc()->
		dispatch(LFUN_CHILDOPEN, getContents());
}


void InsetParent::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}


// LaTeX must just ignore this command
int InsetParent::latex(Buffer const * buf, ostream & os,
		       bool fragile, bool free_spc) const
{
	os << "%%#{lyx}";
	InsetCommand::latex(buf, os, fragile, free_spc);
	return 0;
}
