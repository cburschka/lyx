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

#ifdef LYX_USE_STD_REGEX
#  include <regex>
// <regex> in gcc is unusable in versions less than 4.9.0
// see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
#  define LR_NS std
#else
#  include <boost/regex.hpp>
#  define LR_NS boost
#endif

namespace lyx {
using LR_NS::regex;
using LR_NS::regex_match;
using LR_NS::regex_replace;
using LR_NS::regex_search;
using LR_NS::sregex_iterator;
using LR_NS::smatch;
using LR_NS::basic_regex;
using LR_NS::regex_error;
using LR_NS::match_results;

namespace regex_constants
{
using namespace LR_NS::regex_constants;
using LR_NS::regex_constants::match_flag_type;
}

}

#undef LR_NS

#endif
