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

/** This is a "float layout" object. It contains the parameters for how to
    handle the different kinds of floats, default ones and user created ones.
    Objects of this class is stored in a container in FloatList. The different
    InsetFloat(s) have a pointer/reference through the name of the Floating
    so that it knows how the different floats should be handled.
*/
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
