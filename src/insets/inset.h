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
	InsetOld(InsetOld const & in);
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
	/// returns the actual scroll-value
	virtual int scroll(bool recursive = true) const;

	///
	bool forceDefaultParagraphs(InsetBase const * inset) const;
protected:
	///
	mutable int scx;
	///
	mutable Dimension dim_;

private:
	///
	std::string name_;
	/** We store the LColor::color value as an int to get LColor.h out
	 *  of the header file.
	 */
	int background_color_;
};


/** \c InsetOld_code is a wrapper for InsetOld::Code.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose inset.h.
 */
class InsetOld_code {
	InsetOld::Code val_;
public:
	InsetOld_code(InsetOld::Code val) : val_(val) {}
	operator InsetOld::Code() const { return val_; }
};

#endif
