#include <config.h>


#ifdef HAVE_SSTREAM
#include <sstream>
using std::istringstream;
#else
#include <strstream>
#endif

#include "Spacing.h"

using std::ios;

/// how can I put this inside of Spacing (class)
static
char const * spacing_string[] = {"single", "onehalf", "double", "other"};


float Spacing::getValue() const 
{
	switch(space) {
	case Single: return 1.0;
	case Onehalf: return 1.25;
	case Double: return 1.667;
	case Other: return value;
	}
	return 1.0;
}


void Spacing::set(Spacing::Space sp, float val)
{
	space = sp;
	if (sp == Other) {
		switch(int(val * 1000 + 0.5)) {
		case 1000: space = Single; break;
		case 1250: space = Onehalf; break;
		case 1667: space = Double; break;
		default: value = val; break;
		}
	}
}


void Spacing::set(Spacing::Space sp, char const * val)
{
	float fval;
#ifdef HAVE_SSTREAM
	istringstream istr(val);
#else
	istrstream istr(val);
#endif
	istr >> fval;
	set(sp, fval);
}


void Spacing::writeFile(ostream & os) const
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
