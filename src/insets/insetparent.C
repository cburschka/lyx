/**
 * \file insetparent.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS
 */


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
#include "funcrequest.h"
#include "buffer.h"
#include "gettext.h"

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
	bv->owner()->dispatch(FuncRequest(LFUN_CHILDOPEN, getContents()));
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
