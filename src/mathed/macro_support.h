// -*- C++ -*-
#ifndef MACROSUPPORT_H
#define MACROSUPPORT_H

#include <iosfwd>

enum MathedMacroFlag {
	MMF_Env  = 1,
	MMF_Exp  = 2,
	MMF_Edit = 4
};

std::ostream & operator<<(std::ostream & o, MathedMacroFlag mmf);
#endif
