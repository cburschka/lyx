// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LANGUAGE_H
#define LANGUAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
#include "LString.h"

class Encoding;

///
class Language {
public:
	///
	Language() : RightToLeft_(false) {}
	///
	Language(string const & l, string const & b, string const & d,
		 bool rtl, Encoding const * e, string const & c)
		: lang_(l), babel_(b), display_(d), RightToLeft_(rtl),
		  encoding_(e), code_(c)
		{}
	///
	string const & lang() const {
		return lang_;
	}
	///
	string const & babel() const {
		return babel_;
	}
	///
	string const & display() const {
		return display_;
	}
	///
	bool RightToLeft() const {
		return RightToLeft_;
	}
	///
	Encoding const * encoding() const {
		return encoding_;
	}
	///
	string const & code() const {
		return code_;
	}
private:
	///
	string lang_;
	///
	string babel_;
	///
	string display_;
	///
	bool RightToLeft_;
	///
	Encoding const * encoding_;
	///
	string code_;
};

#if 0
///
bool operator==(Language const & l1, Language const & l2) 
{
	return l1.lang == l2.lang
		&& l1.display_ == l2.display_
		&& l1.RightToLeft_ == l2.RightToLeft_
		&& l1.encoding_ == l2.encoding_;
}

///
bool operator!=(Language const l1, Language const & l2)
{
	return !(l1 == l2);

}
#endif
///
typedef std::map<string, Language> Languages;
///
extern Languages languages;
///
extern Language const * default_language;
///
extern Language const *ignore_language;

#endif
