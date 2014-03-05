/**
 * \file AppleScript.m
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Benjamin Piwowarski
 *
 * Full author contact details are available in file CREDITS.
 */
 
#import <Cocoa/Cocoa.h>
#include "AppleScript.h"



@interface LyxCommand : NSScriptCommand {
}
- (id)performDefaultImplementation;
@end

@implementation LyxCommand
- (id)performDefaultImplementation {
	// Get the command and argument
	NSDictionary * theArguments = [self evaluatedArguments];

  NSString * directParameter = [self directParameter];
  NSString *arg = [theArguments objectForKey: @"arg"];

    
  // Execute the command
  LyXFunctionResult result = applescript_execute_command([directParameter UTF8String], [arg UTF8String]);
  
  // Construct the result record
  NSString *message = [NSString stringWithCString:result.message encoding:NSUTF8StringEncoding];
  free(result.message); 
  
  if (result.code != 0) {
    NSScriptCommand* c = [NSScriptCommand currentCommand]; 
    [c setScriptErrorNumber:result.code]; 
    [c setScriptErrorString:message];
    return NULL;
  }
 
  return message;
}

@end

/// Needed by AppleScript in order to discover LyxCommand
void setupApplescript() {
}

