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

#if defined(LYX_USE_TR1) && defined(LYX_USE_TR1_REGEX)
#  ifdef _MSC_VER
#    include <regex>
#    define match_partial _Match_partial
namespace lyx {
  // inheriting 'private' to see which functions are used and if there are
  // other ECMAScrip defaults
  class regex : private std::tr1::regex
  {
  public:
    regex() {}
    regex(const regex& rhs) : std::tr1::regex(rhs) {}
    template<class T>
    regex(T t) : std::tr1::regex(t, std::tr1::regex_constants::grep) {}
    template<class T>
    void assign(T t) { std::tr1::regex::assign(t, std::tr1::regex_constants::grep); }
    template<class T, class V>
    void assign(T t, V v) { std::tr1::regex::assign(t, v); }
    const std::tr1::regex& toTr1() const { return *this; }
  };
  template<class T>
  bool regex_match(T t, const regex& r) { return std::tr1::regex_match(t, r.toTr1()); }
  template<class T, class V>
  bool regex_match(T t, V v, const regex& r) { return std::tr1::regex_match(t, v, r.toTr1()); }
  template<class T, class V, class U, class H>
  bool regex_match(T t, V v, H h, const regex& r, U u) { return std::tr1::regex_match(t, v, h, r.toTr1(), u); }
  template<class T, class V>
  std::string regex_replace(T t, const regex& r, V v) { return std::tr1::regex_replace(t, r.toTr1(), v); }
  //template<class T, class V, class U, class H>
  //std::string regex_replace(T t, V v, U u, const regex& r, H h) { return std::tr1::regex_replace(t, v, u, r.toTr1(), h); }
  template<class T>
  bool regex_search(T t, const regex& r) { return std::tr1::regex_search(t, r.toTr1()); }
  template<class T, class V>
  bool regex_search(T t, V v, const regex& r) { return std::tr1::regex_search(t, v, r.toTr1()); }
  template<class T, class V, class U>
  bool regex_search(T t, V v, U u, const regex& r) { return std::tr1::regex_search(t, v, u, r.toTr1()); }

  struct sregex_iterator : std::tr1::sregex_iterator
  {
    sregex_iterator() {}
    template<class T, class V>
    sregex_iterator(T t, V v, const regex& r) : std::tr1::sregex_iterator(t, v, r.toTr1()) {}
  };
}
#  else
#    include <tr1/regex>
//   TODO no match_partial in TR1, how to replace?
#  endif
#  define LR_NS std::tr1
namespace lyx {
using LR_NS::regex;
using LR_NS::regex_match;
using LR_NS::sregex_iterator;
}
#elif LYX_USE_TR1_REGEX
#  include <regex>
// <regex> in gcc is unusable in versions less than 4.9.0
// see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
// TODO no match_partial in std, how to replace?
#  define LR_NS std
namespace lyx {
using LR_NS::regex;
using LR_NS::regex_match;
using LR_NS::sregex_iterator;
}
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
