// -*- C++ -*-
/**
 * \file regexp.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_REGEXP_H
#define LYX_REGEXP_H

#if __cplusplus >= 201103L && defined(LYX_USE_STD_REGEX)
#  include <regex>
#  ifdef _MSC_VER
namespace lyx {
  // inheriting 'private' to see which functions are used and if there are
  // other ECMAScrip defaults
  class regex : private std::regex
  {
  public:
    regex() {}
    regex(const regex& rhs) : std::regex(rhs) {}
    template<class T>
    regex(T t) : std::regex(t, std::regex_constants::grep) {}
    template<class T>
    void assign(T t) { std::regex::assign(t, std::regex_constants::grep); }
    template<class T, class V>
    void assign(T t, V v) { std::regex::assign(t, v); }
    const std::regex& toStd() const { return *this; }
  };
  template<class T>
  bool regex_match(T t, const regex& r) { return std::regex_match(t, r.toStd()); }
  template<class T, class V>
  bool regex_match(T t, V v, const regex& r) { return std::regex_match(t, v, r.toStd()); }
  template<class T, class V, class U, class H>
  bool regex_match(T t, V v, H h, const regex& r, U u) { return std::regex_match(t, v, h, r.toStd(), u); }
  template<class T, class V>
  std::string regex_replace(T t, const regex& r, V v) { return std::regex_replace(t, r.toStd(), v); }
  //template<class T, class V, class U, class H>
  //std::string regex_replace(T t, V v, U u, const regex& r, H h) { return std::regex_replace(t, v, u, r.toStd(), h); }
  template<class T>
  bool regex_search(T t, const regex& r) { return std::regex_search(t, r.toStd()); }
  template<class T, class V>
  bool regex_search(T t, V v, const regex& r) { return std::regex_search(t, v, r.toStd()); }
  template<class T, class V, class U>
  bool regex_search(T t, V v, U u, const regex& r) { return std::regex_search(t, v, u, r.toStd()); }

  struct sregex_iterator : std::sregex_iterator
  {
    sregex_iterator() {}
    template<class T, class V>
    sregex_iterator(T t, V v, const regex& r) : std::sregex_iterator(t, v, r.toStd()) {}
  };
}
#  else
// <regex> in gcc is unusable in versions less than 4.9.0
// see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
#  define LR_NS std
namespace lyx {
using LR_NS::regex;
using LR_NS::regex_match;
using LR_NS::sregex_iterator;
}
#  endif
#else
#  include <boost/regex.hpp>
#  define LR_NS boost
namespace lyx {
using LR_NS::regex;
using LR_NS::regex_match;
using LR_NS::sregex_iterator;
}
#endif

namespace lyx {
using LR_NS::smatch;
using LR_NS::regex_replace;
using LR_NS::basic_regex;
using LR_NS::regex_error;
using LR_NS::regex_search;
using LR_NS::match_results;

namespace regex_constants
{
using namespace LR_NS::regex_constants;
using LR_NS::regex_constants::match_flag_type;
}

}

#undef LR_NS

#endif
