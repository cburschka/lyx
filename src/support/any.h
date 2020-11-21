// -*- C++ -*-
/**
 * \file any.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Yuriy Skalko
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_ANY_H
#define LYX_ANY_H

#if __cplusplus >= 201703L
	#include <any>
	namespace lyx { using std::any; }
#else
	#include <boost/any.hpp>
	namespace lyx { using boost::any; }
#endif

#endif // LYX_ANY_H
