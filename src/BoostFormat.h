// -*- C++ -*-
#ifndef LYX_BOOST_FORMAT_H
#define LYX_BOOST_FORMAT_H

// Only include this if it is possible to use
// Boost.Format at all.
#if USE_BOOST_FORMAT

#include <boost/format.hpp>

// Add explicit instantion

namespace boost
{

  extern
  template basic_format<char>;

} // namespace boost

#endif // USE_BOOST_FORMAT

#endif // LYX_BOOST_FORMAT_H
