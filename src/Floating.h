// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1998-2001 The LyX Team.
 *
 * ======================================================
 */

#ifndef FLOATING_H
#define FLOATING_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

/** This is a "float layout" object. It contains the parameters for how to
    handle the different kinds of floats, default ones and user created ones.
    Objects of this class is stored in a container in FloatList. The different
    InsetFloat(s) have a pointer/reference through the name of the Floating
    so that it knows how the different floats should be handled.
*/
class Floating {
public:
	///
	Floating();
	///
	Floating(string const & type, string const & placement,
		 string const & ext, string const & within,
		 string const & style, string const & name,
		 bool builtin = false);
	///
	string const & type() const;
	///
	string const & placement() const;
	///
	string const & ext() const;
	///
	string const & within() const;
	///
	string const & style() const;
	///
	string const & name() const;
	///
	bool builtin() const;
private:
	///
	string type_;
	///
	string placement_;
	///
	string ext_;
	///
	string within_;
	///
	string style_;
	///
	string name_;
	///
	bool builtin_;
};

#endif
