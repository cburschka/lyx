// -*- C++ -*-
/**
 * \file support/assert.h
 *
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LASSERT_H
#define LASSERT_H

namespace lyx {

void doAssert(char const * expr, char const * file, long line);

} // namespace lyx

#define LASSERT(expr, escape) \
	if (expr) {} else { lyx::doAssert(#expr, __FILE__, __LINE__); escape; }


#endif // LASSERT
