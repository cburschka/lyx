// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998-2000 The LyX Team.
 *
 * ======================================================
 */

#ifndef FLOATING_H
#define FLOATING_H

#ifdef __GNUG__
#pragma interface
#endif

class Floating {
public:
	///
	string type;
	///
	string placement;
	///
	string ext;
	///
	string within;
	///
	string style;
	///
	string name;
	///
	bool builtin;
};
#endif
