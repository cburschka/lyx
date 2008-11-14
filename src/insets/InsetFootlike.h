// -*- C++ -*-
/**
 * \file InsetFootlike.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETFOOTLIKE_H
#define INSETFOOTLIKE_H

#include "InsetCollapsable.h"


namespace lyx {

// To have this class is probably a bit overkill... (Lgb)

// The footnote inset
class InsetFootlike : public InsetCollapsable {
public:
	///
	InsetFootlike(Buffer const &);
private:
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream & os) const;
	///
	bool insetAllowed(InsetCode) const;
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
};


} // namespace lyx

#endif
