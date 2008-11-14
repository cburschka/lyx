// -*- C++ -*-
/**
 * \file CommandInset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */


#ifndef COMMAND_INSET_H
#define COMMAND_INSET_H

#include "InsetMathNest.h"

#include "insets/RenderButton.h"


namespace lyx {


/// Inset for things like \name[options]{contents}
class CommandInset : public InsetMathNest {
public:
	///
	explicit CommandInset(docstring const & name, bool needs_math_mode = true);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	Inset * editXY(Cursor &, int, int);
	///
	void write(WriteStream & os) const;
	//
	// void infoize(odocstream & os) const;
	///
	virtual docstring const screenLabel() const;
	///
	docstring const & commandname() const { return name_; }
	///
	bool isActive() const { return false; }

private:
	virtual Inset * clone() const;

	///
	docstring name_;
	///
	bool needs_math_mode_;
	///
	mutable bool set_label_;
	///
	mutable RenderButton button_;
};


} // namespace lyx

#endif
