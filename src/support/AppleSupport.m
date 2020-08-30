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


void appleCleanupViewMenu() {

	// Remove the "Show Tab Bar" menu item from the "View" menu, if supported
	// See the Apple developer release notes:
	// What should an application which already has support for tabbing do?
	// - The application should explicitly opt-out of automatic window tabbing...
	// It should respect the userTabbingPreference... see below
	// https://developer.apple.com/library/archive/releasenotes/AppKit/RN-AppKitOlderNotes/index.html
	if ([NSWindow respondsToSelector:@selector(allowsAutomaticWindowTabbing)])
		[NSWindow setAllowsAutomaticWindowTabbing:NO];

	// Remove the "Enter Full Screen" menu item from the "View" menu
	[[NSUserDefaults standardUserDefaults] setBool:NO forKey:@"NSFullScreenMenuItemEverywhere"];
}


bool appleUserTabbingPreferenceAlways() {
	return [NSWindow respondsToSelector:@selector(userTabbingPreference)] &&
		[NSWindow userTabbingPreference] == NSWindowUserTabbingPreferenceAlways;
}
