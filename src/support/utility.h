//  boost utility.hpp header file  -------------------------------------------//

//  (C) Copyright boost.org 1999. Permission to copy, use, modify, sell
//  and distribute this software is granted provided this copyright
//  notice appears in all copies. This software is provided "as is" without
//  express or implied warranty, and with no claim as to its suitability for
//  any purpose.

//  See http://www.boost.org for most recent version including documentation.

//  Classes appear in alphabetical order

//  Revision History
//  21 May 01  checked_delete() and checked_array_delete() added (Beman Dawes,
//             suggested by Dave Abrahams, generalizing idea from Vladimir Prus)
//  21 May 01  made next() and prior() inline (Beman Dawes)
//  26 Jan 00  protected noncopyable destructor added (Miki Jovanovic)
//  10 Dec 99  next() and prior() templates added (Dave Abrahams)
//  30 Aug 99  moved cast templates to cast.hpp (Beman Dawes)
//   3 Aug 99  cast templates added
//  20 Jul 99  name changed to utility.hpp
//   9 Jun 99  protected noncopyable default ctor
//   2 Jun 99  Initial Version. Class noncopyable only contents (Dave Abrahams)

#ifndef LYX_UTILITY_H
#define LYX_UTILITY_H

#include <boost/config.hpp>        // broken compiler workarounds
#include <boost/static_assert.hpp>
//#include <cstddef>                 // for size_t
//#include <utility>                 // for std::pair

namespace lyx
{
//  checked_delete() and checked_array_delete()  -----------------------------//

    // verify that types are complete for increased safety

    template< typename T >
    inline void checked_delete(T * x)
    {
	BOOST_STATIC_ASSERT( sizeof(T) != 0 ); // assert type complete at point
					       // of instantiation
	free(x);
    }

} // namespace boost

#endif  // BOOST_UTILITY_HPP
