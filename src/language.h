// -*- C++ -*-
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <map>
#include "LString.h"

///
class Language {
public:
	///
	Language() : RightToLeft_(false) {}
	///
	Language(string const & l, string const & d, bool rtl)
		: lang_(l), display_(d), RightToLeft_(rtl) {}
	///
	string const & lang() const {
		return lang_;
	}
	///
	string const & display() const {
		return display_;
	}
	///
	bool RightToLeft() const {
		return RightToLeft_;
	}
private:
	///
	string lang_;
	///
	string display_;
	///
	bool RightToLeft_;
};

#if 0
bool operator==(Language const & l1, Language const & l2) 
{
	return l1.lang == l2.lang
		&& l1.display == l2.display
		&& l1.RightToLeft == l2.RightToLeft;
}


bool operator!=(Language const l1, Language const & l2)
{
	return !(l1 == l2);

}
#endif

typedef std::map<string, Language> Languages;
extern Languages languages;
extern Language const * default_language;
extern Language const *ignore_language;

#endif
