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

class Languages
{
public:
	///
	typedef std::map<string, Language> LanguageList;
	///
	typedef LanguageList::const_iterator const_iterator;
	///
	void read(string const & filename);
	///
	void setDefaults();
	///
	Language const * getLanguage(string const & language) const;
	///
	const_iterator begin() const {
                return languagelist.begin();
        }
        ///
        const_iterator end() const {
                return languagelist.end();
        }
	///
	
private:
	///
	LanguageList languagelist;
};

extern Languages languages;
extern Language const * default_language;
extern Language const * english_language;
extern Language const *ignore_language;

#endif
