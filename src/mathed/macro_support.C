#include <config.h>

#include "support/LOstream.h"
#include "macro_support.h"

using std::ostream;

ostream & operator<<(ostream & o, MathedMacroFlag mmf)
{
	return o << int(mmf);
}
