// -*- C++ -*-
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <map>
#include "LString.h"

using std::map;

struct Language {
	string lang;
	string display;
	bool leftTOright;
};

typedef map<string, Language> Languages;
extern Languages languages;

#endif
