// -*- C++ -*-
/**
 * \file methash.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 *
 * Code by Tor Brede Vekterli 
 * http://arcticinteractive.com/2009/04/18/compile-time-string-hashing-boost-mpl/
 * (Boost 1.0 license.)
 *
 */

#ifndef LYX_META_HASH_H
#define LYX_META_HASH_H

#include <boost/mpl/string.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/functional/hash.hpp>



namespace lyx {
namespace support {

#ifdef _MSC_VER
#pragma warning(push)
// disable addition overflow warning
#pragma warning(disable:4307)
#endif

    template <typename Seed, typename Value>
    struct hash_combine
    {
      typedef boost::mpl::size_t<
        Seed::value ^ (static_cast<std::size_t>(Value::value)
		+ 0x9e3779b9 + (Seed::value << 6) + (Seed::value >> 2))
      > type;
    };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    // Hash any sequence of integral wrapper types
    template <typename Sequence>
    struct hash_sequence
      : boost::mpl::fold<
            Sequence
          , boost::mpl::size_t<0> 
		  , hash_combine<boost::mpl::_1, boost::mpl::_2>
        >::type
    {};

    // For hashing std::strings et al that don't include the zero-terminator
    template <typename String>
    struct hash_string : hash_sequence<String>
    {};

    // Hash including terminating zero for char arrays
    template <typename String>
    struct hash_cstring
      : hash_combine< hash_sequence<String>, boost::mpl::size_t<0> >::type
    {};

} 
}

#endif
