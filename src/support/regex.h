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

#include <regex>

namespace lyx {

using std::regex;
using std::regex_match;
using std::regex_replace;
using std::regex_search;
using std::sregex_iterator;
using std::smatch;
using std::basic_regex;
using std::regex_error;
using std::match_results;

namespace regex_constants
{

using namespace std::regex_constants;
using std::regex_constants::match_flag_type;

} // namespace regex_constants

} // namespace lyx

// Match Begin and End of String when using ECMAScript (default std::regex)
#define REGEX_BOS "^"
#define REGEX_EOS "$"

#endif // LYX_REGEXP_H
