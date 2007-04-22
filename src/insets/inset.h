// -*- C++ -*-
/**
 * \file inset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETOLD_H
#define INSETOLD_H

#include "insetbase.h"


namespace lyx {

/// Insets
class InsetOld : public InsetBase {
public:
	///
	enum {
		///
		TEXT_TO_INSET_OFFSET = 4
	};

	///
	InsetOld();

	///
	void setInsetName(docstring const & s) { name_ = s; }
	///
	virtual docstring const & getInsetName() const { return name_; }
	/// set x/y drawing position cache
	void setPosCache(PainterInfo const &, int, int) const;

protected:
	///
	InsetOld(InsetOld const & in);

private:
	InsetOld & operator=(InsetOld const &) const;

	///
	docstring name_;
};


/** \c InsetBase_code is a wrapper for InsetBase::Code.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose inset.h.
 */
class InsetBase_code {
	InsetBase::Code val_;
public:
	InsetBase_code(InsetBase::Code val) : val_(val) {}
	operator InsetBase::Code() const { return val_; }
};


} // namespace lyx

#endif
