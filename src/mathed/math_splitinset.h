// -*- C++ -*-
/**
 * \file math_splitinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SPLITINSET_H
#define MATH_SPLITINSET_H

#include "math_gridinset.h"


class MathSplitInset : public MathGridInset {
public:
	///
	explicit MathSplitInset(std::string const & name);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void write(WriteStream & os) const;
	///
	int defaultColSpace(col_type) { return 0; }
	///
	char defaultColAlign(col_type);
private:
	///
	std::string name_;
};

#endif
