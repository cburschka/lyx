// -*- C++ -*-
/**
 * \file InsetMathCommand.h
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
class InsetMathCommand : public InsetMathNest {
public:
	///
	explicit InsetMathCommand(Buffer * buf, docstring const & name,
		bool needs_math_mode = true);
	///
	marker_type marker(BufferView const *) const override { return NO_MARKER; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	Inset * editXY(Cursor &, int, int) override;
	///
	void write(WriteStream & os) const override;
	//
	// void infoize(odocstream & os) const;
	///
	virtual docstring const screenLabel() const;
	///
	docstring const & commandname() const { return name_; }
	///
	bool isActive() const override { return false; }

private:
	Inset * clone() const override;

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
