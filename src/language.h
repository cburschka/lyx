// -*- C++ -*-
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <map>
#include "LString.h"

struct Language {
	Language() : RightToLeft(false) {}
	Language(string const & l, string const & d, bool rtl)
		: lang(l), display(d), RightToLeft(rtl) {}
	string lang;
	string display;
	bool RightToLeft;
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
