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

SpellCheckResult checkAppleSpeller(AppleSpeller speller, const char * word, const char * lang);
void ignoreAppleSpeller(AppleSpeller speller, const char * word);
size_t makeSuggestionAppleSpeller(AppleSpeller speller, const char * word, const char * lang);
const char * getSuggestionAppleSpeller(AppleSpeller speller, size_t pos);
void learnAppleSpeller(AppleSpeller speller, const char * word);
void unlearnAppleSpeller(AppleSpeller speller, const char * word);
int hasLanguageAppleSpeller(AppleSpeller speller, const char * lang);
int numMisspelledWordsAppleSpeller(AppleSpeller speller);
void misspelledWordAppleSpeller(AppleSpeller speller, int const position, int * start, int * length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
