// -*- C++ -*-
/**
 * \file Floating.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FLOATING_H
#define FLOATING_H

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
		 string const & listName, bool builtin = false);
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
	string const & listName() const;
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
	string listName_;
	///
	bool builtin_;
};

#endif
