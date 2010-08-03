/**
 * \file AppleSpeller.m
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

#import <AvailabilityMacros.h>

#include "support/AppleSpeller.h"

typedef struct AppleSpellerRec {
	NSSpellChecker * checker;
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 1050)
	NSInteger doctag;
#else
	int doctag;
#endif
	char ** suggestions;
	size_t numsug;
} AppleSpellerRec ;


static void freeSuggestionsAppleSpeller(AppleSpeller speller)
{
	if (speller->suggestions) {
		while (speller->numsug--) {
			free(speller->suggestions[speller->numsug]);
		}
		free(speller->suggestions);
		speller->suggestions = 0;
	}
}


AppleSpeller newAppleSpeller(void)
{
	AppleSpeller speller = calloc(1, sizeof(AppleSpellerRec));
	speller->checker = [NSSpellChecker sharedSpellChecker];
	speller->suggestions = 0;
	speller->doctag = [NSSpellChecker uniqueSpellDocumentTag];
	return speller;
}


void freeAppleSpeller(AppleSpeller speller)
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	freeSuggestionsAppleSpeller(speller);
	[speller->checker closeSpellDocumentWithTag:speller->doctag];

	[pool release];

	free(speller);
}


static NSString * toString(AppleSpeller speller, const char * word)
{
	return [[NSString alloc] initWithBytes:word length:strlen(word) encoding:NSUTF8StringEncoding];
}


int checkAppleSpeller(AppleSpeller speller, const char * word, const char * lang)
{
	if (!speller->checker || !lang || !word)
		return 0;

	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSString * word_ = toString(speller, word);
	NSString * lang_ = toString(speller, lang);

	NSRange result = [speller->checker
		checkSpellingOfString:word_
		startingAt:0
		language:lang_
		wrap:NO
		inSpellDocumentWithTag:speller->doctag
		wordCount:NULL];

	[word_ release];
	[lang_ release];
	[pool release];

	return (result.length ? 0 : 1);
}


void ignoreAppleSpeller(AppleSpeller speller, const char * word, const char * lang)
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSString * word_ = toString(speller, word);

	[speller->checker ignoreWord:word_ inSpellDocumentWithTag:(speller->doctag)];

	[word_ release];
	[pool release];
}


size_t makeSuggestionAppleSpeller(AppleSpeller speller, const char * word, const char * lang)
{
	if (!speller->checker || !word || !lang)
		return 0;

	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSString * word_ = toString(speller, word);
	NSString * lang_ = toString(speller, lang);
	NSArray * result ;

#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 1060)
	// Mac OS X 10.6 only
	NSInteger slen = [word_ length];
	NSRange range = { 0, slen };
	
	if ([NSSpellChecker instancesRespondToSelector:@selector(guessesForWordRange:)]) {
		result = [speller->checker guessesForWordRange:range
			inString:word_
			language:lang_
			inSpellDocumentWithTag:speller->doctag];
	} else {
		[speller->checker setLanguage:lang_];
		result = [speller->checker guessesForWord:word_];
	}
#else
	[speller->checker setLanguage:lang_];
	result = [speller->checker guessesForWord:word_];
#endif

	[word_ release];
	[lang_ release];

	freeSuggestionsAppleSpeller(speller);

	speller->numsug = [result count];
	if (speller->numsug) {
		speller->suggestions = calloc(speller->numsug + 1, sizeof(char *));
		if (speller->suggestions) {
			size_t i;
			for (i = 0; i < speller->numsug; i++) {
				NSString * str = [result objectAtIndex:i];
				speller->suggestions[i] = strdup([str UTF8String]);
			}
			speller->suggestions[speller->numsug] = 0;
		}
	}
	[pool release];
	return speller->numsug;
}


const char * getSuggestionAppleSpeller(AppleSpeller speller, size_t pos)
{
	const char * result = 0;
	if (pos < speller->numsug && speller->suggestions) {
		result = speller->suggestions[pos] ;
	}
	return result;
}


void learnAppleSpeller(AppleSpeller speller, const char * word, const char * lang)
{
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 1050)
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSString * word_ = toString(speller, word);

	if ([NSSpellChecker instancesRespondToSelector:@selector(learnWord)])
		[speller->checker learnWord:word_];
	
	[word_ release];
	[pool release];
#endif
}


int hasLanguageAppleSpeller(AppleSpeller speller, const char * lang)
{
	BOOL result = NO;
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 1050)
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSString * lang_ = toString(speller, lang);
	if ([NSSpellChecker instancesRespondToSelector:@selector(availableLanguages)]) {
		NSArray * languages = [speller->checker availableLanguages];

		for (NSString *element in languages) {
			result = [element isEqualToString:lang_] || [lang_ hasPrefix:element];
			if (result) break;
		}
	}

	[lang_ release];
	[pool release];
#endif

	return result ? 1 : 0;
}
