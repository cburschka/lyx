// -*- C++ -*-
/**
 * \file QMiniBuffer.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QMiniBuffer.h"

QMiniBuffer::QMiniBuffer(LyXView * o)
	: MiniBuffer(o)
{
}


QMiniBuffer::~QMiniBuffer()
{
}

 
bool QMiniBuffer::isEditingMode() const
{
	return false;
}
 
 
void QMiniBuffer::editingMode()
{
}


 
void QMiniBuffer::messageMode()
{
}
 
 
void QMiniBuffer::set_input(string const &)
{
}
