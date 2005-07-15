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
#include "dimension.h"

class LColor_color;
class UpdatableInset;


/// Insets
class InsetOld : public InsetBase {
public:
	///
	enum {
		///
		TEXT_TO_INSET_OFFSET = 2
	};

	///
	InsetOld();
	///
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;

	///
	void setInsetName(std::string const & s) { name_ = s; }
	///
	std::string const & getInsetName() const { return name_; }
	///
	virtual void setBackgroundColor(LColor_color);
	///
	LColor_color backgroundColor() const;
	/// set x/y drawing position cache
	void setPosCache(PainterInfo const &, int, int) const;

protected:
	///
	InsetOld(InsetOld const & in);
	///
	mutable Dimension dim_;

private:
	InsetOld & operator=(InsetOld const &) const;

	///
	std::string name_;
	/** We store the LColor::color value as an int to get LColor.h out
	 *  of the header file.
	 */
	int background_color_;
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

#endif
