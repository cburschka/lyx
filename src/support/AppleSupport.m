/**
 * \file AppleSupport.m
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#import <Cocoa/Cocoa.h>
#include "AppleSupport.h"


void appleCleanupEditMenu() {

	// Remove (disable) the "Start Dictation..." and "Emoji & Symbols" menu items
	// from the "Edit" menu

	[[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledDictationMenuItem"];
	[[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledCharacterPaletteMenuItem"];

}
