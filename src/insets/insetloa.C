// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1995 Matthias Ettrich
 *          Copyright 1996-2000 The LyX Team.
 * 
 * ====================================================== */

// Created by Bernhard 970908

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"
#include "insetloa.h"
#include "LaTeXFeatures.h"
#include "BufferView.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "commandtags.h"

void InsetLOA::Validate(LaTeXFeatures & features) const 
{
	features.algorithm = true;
}


string InsetLOA::getScreenLabel() const 
{
	return _("List of Algorithms");
}

void InsetLOA::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getLyXFunc()->Dispatch(LFUN_LOAVIEW);
}
