// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000 The LyX Team.
 * 
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcite.h"
#include "BufferView.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"
#include "support/lstrings.h"

InsetCitation::InsetCitation(InsetCommandParams const & p)
		: InsetCommand(p)
{}

string InsetCitation::getScreenLabel() const
{
	string keys(getContents());

	// If keys is "too long" then only print out the first few tokens
	string label;
	if( contains( keys, "," ) ) {
		// Final comma allows while loop to cover all keys
		keys = frontStrip( split( keys, label, ',' ) ) + ",";

		const int maxSize( 40 );
		while( contains( keys, "," ) ) {
			string key;
			keys = frontStrip( split( keys, key, ',' ) );

			int size = label.size() + 2 + key.size();
			if( size >= maxSize ) {
				label += ", ...";
				break;
			}
			label += ", " + key;
		}
	} else {
		label = keys;
	}

	if( !getOptions().empty() )
		label += ", " + getOptions();

	return '[' + label + ']';
}

void InsetCitation::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showCitation( this );
}

