// -*- C++ -*-
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <map>
#include "LString.h"

using std::map;

struct Language {
	string lang;
	string display;
	bool RightToLeft;
};

typedef map<string, Language> Languages;
extern Languages languages;
extern Language const * default_language;
extern Language const * ignore_language;

#endif
