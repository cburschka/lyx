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

#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101200)
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

#endif
}


bool appleUserTabbingPreferenceAlways() {
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101200)
	return [NSWindow respondsToSelector:@selector(userTabbingPreference)] &&
		[NSWindow userTabbingPreference] == NSWindowUserTabbingPreferenceAlways;
#else
	return false;
#endif
}


int NSTextInsertionPointBlinkPeriodOn() {
	NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];

	return [prefs objectForKey:@"NSTextInsertionPointBlinkPeriodOn"] == nil ?
		-1 : [prefs floatForKey:@"NSTextInsertionPointBlinkPeriodOn"];
}


int NSTextInsertionPointBlinkPeriodOff() {
	NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];

	return [prefs objectForKey:@"NSTextInsertionPointBlinkPeriodOff"] == nil ?
			-1 : [prefs floatForKey:@"NSTextInsertionPointBlinkPeriodOff"];
}
