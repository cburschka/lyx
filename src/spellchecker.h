// -*- C++ -*-
#ifndef _SPELLCHECKER_H
#define _SPELLCHECKER_H

/* These functions are defined in lyx_cb.C */

class LString;


/** The returned word has to be deleted manually
 */
char* NextWord(float &value);


/** MarkLastWord should only be used immidiately after NextWord().
 If you give control back to the user, you _have_ to call EndOfSpellCheck()
 or SelectLastWord(), otherwise segfaults should appear.
 */
void EndOfSpellCheck();
///
void SelectLastWord();
///
void ReplaceWord(LString const & replacestringstring);


/** This function has to be implemented by the spell checker.
  * It will show the spellcheker form*/ 
void ShowSpellChecker();
///
void SpellCheckerOptions();

#endif
