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


inline
Floating::Floating() 
{}


inline
Floating::Floating(string const & type, string const & placement,
		   string const & ext, string const & within,
		   string const & style, string const & name,
		   bool builtin)
	: type_(type), placement_(placement), ext_(ext), within_(within),
	  style_(style), name_(name), builtin_(builtin)
{}


inline
string const & Floating::type() const
{
	return type_;
}


inline
string const & Floating::placement() const
{
	return placement_;
}


inline
string const & Floating::name() const
{
	return name_;
}


inline
bool Floating::builtin() const
{
	return builtin_;
}

#endif
