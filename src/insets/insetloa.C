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
#pragma implementation "insetloa.h"
#pragma implementation "insetlof.h"
#pragma implementation "insetlot.h"
#endif

#include "insetloa.h"
#include "insetlof.h"
#include "insetlot.h"
#include "LaTeXFeatures.h"

void InsetLOA::Validate(LaTeXFeatures & features) const 
{
	features.algorithm = true;
}
