#include <config.h>

#include <cstdio>
#include "Spacing.h"
using std::ios;

/// how can I put this inside of Spacing (class)
static
char const * spacing_string[] = {"single", "onehalf", "double", "other"};


void Spacing::writeFile(ostream & os)
{
	if (getSpace() == Spacing::Other) {
		os.setf(ios::showpoint|ios::fixed);
		os.precision(2);
		os << "\\spacing " << spacing_string[getSpace()]
		   << " " << getValue() << " \n";
	} else {
		os << "\\spacing " << spacing_string[getSpace()] << " \n";
	}	
}
