// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *          Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1996-1999 The LyX Team.
 * 
 * ======================================================*/

// Created by Bernhard 970908

#include <config.h>

#ifdef __GNUG__
#pragma implementation "insetloa.h"
#pragma implementation "insetlof.h"
#pragma implementation "insetlot.h"
#endif

#include "LaTeXFeatures.h"
#include "insetloa.h"
#include "insetlof.h"
#include "insetlot.h"

void InsetLOA::Validate(LaTeXFeatures & features) const 
{
	features.algorithm = true;
}
