// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#ifdef __GNUG__
#pragma interface
#endif

/* These functions are defined in lyx_cb.C */

class BufferView;

/** This function has to be implemented by the spell checker.
    It will show the spellcheker form*/ 
void ShowSpellChecker(BufferView *);
///
void SpellCheckerOptions();

#endif
