/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file Dialogs.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * Methods common to all frontends' Dialogs that should not be inline
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "support/LAssert.h"

// Signal enabling all visible dialogs to be redrawn if so desired.
// E.g., when the GUI colours have been remapped.
SigC::Signal0<void> Dialogs::redrawGUI;

void Dialogs::add(DialogBase * ptr)
{
	lyx::Assert(ptr);
	dialogs_.push_back(db_ptr(ptr));
}
