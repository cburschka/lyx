#include <config.h>

#include "BoostFormat.h"

#if USE_BOOST_FORMAT

namespace boost
{

using boost::io::out_of_range_bit;

template class basic_format<char>;

} // namespace boost

#endif // USE_BOOST_FORMAT
