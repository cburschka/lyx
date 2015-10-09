/**
 * \file LinkBackProxy.m
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/linkback/LinkBackProxy.h"

#include "support/linkback/LinkBack.h"

///////////////////////////////////////////////////////////////////////

@interface LyXLinkBackClient : NSObject <LinkBackClientDelegate> {
	NSMutableSet * keys;
}

- (LyXLinkBackClient *)init;
- (BOOL)edit:(NSString *)fileName;
@end

@implementation LyXLinkBackClient

- (LyXLinkBackClient *)init
{
	self = [super init];
	if (self != nil)
		keys = [[NSMutableSet alloc] init];
	return self;
}

- (void)dealloc {
	// close all links
	NSArray * allKeys = [keys allObjects];
	unsigned i;
	for (i = 0; i < [allKeys count]; ++i) {
		LinkBack * link
		= [LinkBack activeLinkBackForItemKey:[allKeys objectAtIndex:i]];
		[link closeLink];
	}
	[keys release];

	[super dealloc];
}

- (BOOL)edit:(NSString *)fileName
{
	if ([LinkBack activeLinkBackForItemKey:fileName])
		return YES;

	@try {
		// get put data, i.e. PDF + LinkBack + 4 bytes PDF-length
		NSData * data = [NSData dataWithContentsOfFile:fileName];
		if (data == nil) {
			NSLog(@"Cannot read file %@", fileName);
			return NO;
		}
		
		// Get linkback data which comes behind the pdf data.
		// The pdf data length are the last 4 bytes.
		UInt32 pdfLen = 0;
		pdfLen = *(UInt32 const *)(((UInt8 const *)[data bytes]) + [data length] - 4);
		pdfLen = NSSwapBigLongToHost(pdfLen); // make it big endian
		if (pdfLen >= [data length] - 4) {
			NSLog(@"Invalid file %@ for LinkBack", fileName);
			return NO;
		}
			
		NSData * linkBackData 
		= [data subdataWithRange:NSMakeRange(pdfLen, [data length] - pdfLen - 4)]; 
		if (linkBackData == nil) {
			NSLog(@"Invalid file %@ for LinkBack", fileName);
			return NO;
		}
		
		NSMutableDictionary * linkBackDataDict
		= [NSUnarchiver unarchiveObjectWithData:linkBackData];
		if (linkBackDataDict == nil) {
			NSLog(@"LinkBack data in %@ corrupted", fileName);
			return NO;
		}
		
		// create the link to the LinkBack server
		LinkBack * link = [LinkBack editLinkBackData:linkBackDataDict
			sourceName:fileName delegate:self itemKey:fileName];
		if (link == nil) {
			NSLog(@"Failed to create LinkBack link for %@", fileName);
			return NO;
		}
		[keys addObject:fileName];
	}
	@catch (NSException * exception) {
		NSLog(@"LinkBack exception: %@", exception);
		return NO;
	}
	
	return YES;
}

- (void)linkBackDidClose:(LinkBack*)link
{
	NSString * fileName = [link itemKey];
	if (fileName) {
		[keys removeObject:fileName];
		NSLog(@"LinkBack link for %@ closed", fileName);
	}
}

- (void)linkBackServerDidSendEdit:(LinkBack*)link
{
	@try {
		// get pasteboard and check that linkback and pdf data is available
		NSPasteboard * pboard = [link pasteboard];
		NSArray * linkBackType = [NSArray arrayWithObjects: LinkBackPboardType, nil];
		NSArray * pdfType = [NSArray arrayWithObjects: NSPDFPboardType, nil];
		if ([pboard availableTypeFromArray:linkBackType] == nil
		    || [pboard availableTypeFromArray:pdfType] == nil) {
			NSLog(@"No PDF or LinkBack data in pasteboard");
			return;
		}
			
		// get new linkback data
		id linkBackDataDict = [pboard propertyListForType:LinkBackPboardType];
		if (linkBackDataDict == nil) {
			NSLog(@"Cannot get LinkBack data from pasteboard");
			return;
		}
		NSData * linkBackData = [NSArchiver archivedDataWithRootObject:linkBackDataDict];
		if (linkBackData == nil) {
			NSLog(@"Cannot archive LinkBack data");
			return;
		}
		
		// get pdf
		NSData * pdfData = [pboard dataForType:NSPDFPboardType];
		if (pdfData == nil) {
			NSLog(@"Cannot get PDF data from pasteboard");
			return;
		}
		
		// update the file
		NSString * fileName = [link itemKey];
		NSFileHandle * file = [NSFileHandle fileHandleForUpdatingAtPath:fileName];
		if (file == nil) {
			NSLog(@"LinkBack file %@ disappeared.", fileName);
			return;
		}
		[file truncateFileAtOffset:0];
		[file writeData:pdfData];
		[file writeData:linkBackData];
		
		UInt32 pdfLen = NSSwapHostLongToBig([pdfData length]); // big endian
		NSData * lenData = [NSData dataWithBytes:&pdfLen length:4];
		[file writeData:lenData];
		[file closeFile];
	}
	@catch (NSException * exception) {
		NSLog(@"LinkBack exception in linkBackServerDidSendEdit: %@", exception);
	}
}

@end


///////////////////////////////////////////////////////////////////////

static LyXLinkBackClient * linkBackClient = nil;

int isLinkBackDataInPasteboard()
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	NSArray * linkBackType = [NSArray arrayWithObjects: LinkBackPboardType, nil];
	NSString * ret = [[NSPasteboard generalPasteboard] availableTypeFromArray:linkBackType];
	[pool release];
	return ret != nil;
}


void getLinkBackData(void const * * buf, unsigned * len)
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	// get linkback data from pasteboard
	NSPasteboard * pboard = [NSPasteboard generalPasteboard];
	id linkBackData = [pboard propertyListForType:LinkBackPboardType];
		
	NSData * nsdata = [NSArchiver archivedDataWithRootObject:linkBackData];
	if (nsdata == nil) {
		*buf = 0;
		*len = 0;
	} else {
		*buf = [nsdata bytes];
		*len = [nsdata length];
	}
	[pool release];
}


int editLinkBackFile(char const * docName)
{
	// setup Obj-C and our client
	if (linkBackClient == nil)
		linkBackClient = [[LyXLinkBackClient alloc] init];
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	// FIXME: really UTF8 here?
	NSString * nsDocName = [NSString stringWithUTF8String:docName];
	int result = [linkBackClient edit:nsDocName] == YES;
	[pool release];
	return result;
}


void closeAllLinkBackLinks()
{
	if (linkBackClient != nil) {
		[linkBackClient release];
		linkBackClient = nil;
	}
}

