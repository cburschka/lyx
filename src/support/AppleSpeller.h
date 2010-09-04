// -*- C++ -*-
/**
 * \file AppleSpeller.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_SUPPORT_SPELLCHECK_H
#define LYX_SUPPORT_SPELLCHECK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SpellCheckResult {
	SPELL_CHECK_FAILED,
	SPELL_CHECK_OK,
	SPELL_CHECK_IGNORED,
	SPELL_CHECK_LEARNED
} SpellCheckResult ;

typedef struct AppleSpellerRec * AppleSpeller ;

AppleSpeller newAppleSpeller(void);
void freeAppleSpeller(AppleSpeller speller);

SpellCheckResult AppleSpeller_check(AppleSpeller speller, const char * word, const char * lang);
void AppleSpeller_ignore(AppleSpeller speller, const char * word);
size_t AppleSpeller_makeSuggestion(AppleSpeller speller, const char * word, const char * lang);
const char * AppleSpeller_getSuggestion(AppleSpeller speller, size_t pos);
void AppleSpeller_learn(AppleSpeller speller, const char * word);
void AppleSpeller_unlearn(AppleSpeller speller, const char * word);
int AppleSpeller_hasLanguage(AppleSpeller speller, const char * lang);
int AppleSpeller_numMisspelledWords(AppleSpeller speller);
void AppleSpeller_misspelledWord(AppleSpeller speller, int index, int * start, int * length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
