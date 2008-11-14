// -*- C++ -*-
/**
 * \file ReplaceData.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_REPLACE_H
#define MATH_REPLACE_H


#include "MathData.h"


namespace lyx {

class ReplaceData {
public:
	///
	MathData from;
	///
	MathData to;
};


} // namespace lyx

#endif
