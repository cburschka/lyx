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

#include "checktr1.h"


#define LYX_REGEX_TO_LYX(X) \
    using X::regex; \
    using X::smatch; \
    using X::regex_replace; \
    using X::basic_regex; \
    using X::regex_error; \
    using X::regex_search; \
    using X::sregex_iterator; \
    using X::match_results; \
    \
    namespace regex_constants \
    { \
        using namespace X::regex_constants; \
        using X::regex_constants::match_flag_type; \
    } \



// TODO: only tested with msvc10
#if defined(LYX_USE_TR1) && defined(_MSC_VER)

#ifdef _MSC_VER
#include <regex>
#define match_partial _Match_partial // why is match_partial not public?
#else
#include <tr1/regexp>
#endif

namespace lyx
{
    LYX_REGEX_TO_LYX(std::tr1);    
}

#else 

#include "boost/regex.hpp"

namespace lyx
{
    LYX_REGEX_TO_LYX(boost);
}

#endif


#endif
