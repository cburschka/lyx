// -*- C++ -*-
#ifndef MACRO_SUPPORT_H
#define MACRO_SUPPORT_H

#include <iosfwd>

enum MathMacroFlag {
	MMF_Env  = 1,
	MMF_Exp  = 2,
	MMF_Edit = 4
};

std::ostream & operator<<(std::ostream & o, MathMacroFlag mmf);
#endif
