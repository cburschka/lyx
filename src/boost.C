#include <config.h>

#include "debug.h"
#include "support/LAssert.h"

#include <exception>

using namespace lyx::support;

using std::endl;

namespace boost {

void throw_exception(std::exception const & e)
{
	lyxerr << "Exception caught:\n"
	    << e.what() << endl;
	Assert(false);
}


}
