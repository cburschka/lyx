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

#include "insetparent.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "support/LOstream.h"
#include "funcrequest.h"
#include "buffer.h"
#include "gettext.h"

#include "support/filetools.h"
#include "support/lstrings.h"

using std::ostream;


InsetParent::InsetParent(InsetCommandParams const & p, Buffer const & bf, bool)
	: InsetCommand(p)
{
	string const fn = p.getContents();
	setContents(MakeAbsPath(fn, bf.filePath()));
}


string const InsetParent::getScreenLabel(Buffer const *) const
{
	return bformat(_("Parent: %1$s"), getContents());
}


dispatch_result InsetParent::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {
		case LFUN_INSET_EDIT:
			cmd.view()->owner()->dispatch(FuncRequest(LFUN_CHILDOPEN, getContents()));
			return DISPATCHED;
		default:
			return UNDISPATCHED;
	}
}


// LaTeX must just ignore this command
int InsetParent::latex(Buffer const * buf, ostream & os, LatexRunParams const & runparams,
		       bool fragile, bool free_spc) const
{
	os << "%%#{lyx}";
	InsetCommand::latex(buf, os, runparams, fragile, free_spc);
	return 0;
}
