// -*- C++ -*-
/**
 * \file math_matrixinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MATRIXINSET_H
#define MATH_MATRIXINSET_H

#include "math_gridinset.h"


// "shortcut" for DelimInset("(",ArrayInset,")")

class MathMatrixInset : public MathGridInset {
public:
	///
	MathMatrixInset(MathGridInset const &);
	///
	MathMatrixInset(std::string const & str);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	/// identifies MatrixInsets
	MathMatrixInset const * asMatrixInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;
};

#endif
