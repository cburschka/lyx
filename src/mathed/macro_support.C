#include <config.h>

#include "support/LOstream.h"
#include "macro_support.h"

using std::ostream;

ostream & operator<<(ostream & o, MathMacroFlag mmf)
{
	return o << int(mmf);
}
