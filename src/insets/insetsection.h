// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 *======================================================
 */


#ifndef INSETSECTION_H
#define INSETSECTION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "insettext.h"

/** A colapsable text inset
*/
class InsetSection : public InsetText {
public:
protected:
private:
	string type_;
};

#endif
