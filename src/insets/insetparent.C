/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright (C) 1997-1999 LyX Team
 * 
 * ======================================================*/


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


extern BufferView * current_view;


InsetParent::InsetParent(string fn, Buffer * owner): InsetCommand("lyxparent")
{
    if (owner)
	setContents(MakeAbsPath(fn, OnlyPath(owner->getFileName())));
    else
	setContents(fn);
}

void InsetParent::Edit(int, int)
{    
    current_view->getOwner()->getLyXFunc()->Dispatch(LFUN_CHILDOPEN, 
						     getContents().c_str());
}

// LaTeX must just ignore this command
int InsetParent::Latex(FILE * file, signed char fragile)
{
    fprintf(file, "%%#{lyx}");
    InsetCommand::Latex(file, fragile);
    return 0;
}

// LaTeX must just ignore this command
int InsetParent::Latex(string & file, signed char fragile)
{
    file += "%%#{lyx}";
    InsetCommand::Latex(file, fragile);
    return 0;
}
