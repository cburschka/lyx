#include <config.h>

#include "debug.h"
#include "support/LAssert.h"

#include <exception>

using std::endl;

namespace boost {

void throw_exception(std::exception const & e)
{
	lyxerr << "Exception caught:\n"
	    << e.what() << endl;
	lyx::Assert(false);
}


}
