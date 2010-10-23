// -*- C++ -*-
/**
 * \file bind.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_BIND_H
#define LYX_BIND_H

#include "support/functional.h"

#ifdef LYX_USE_TR1

namespace lyx
{
	using std::tr1::bind;
	using std::tr1::placeholders::_1;
	using std::tr1::placeholders::_2;
	using std::tr1::ref;
}

#else

#include <boost/bind.hpp>

namespace lyx
{
	using boost::bind;
	using boost::ref;
}

#endif


#endif
