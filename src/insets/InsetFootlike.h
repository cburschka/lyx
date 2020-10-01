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

#include "InsetCollapsible.h"


namespace lyx {

// To have this class is probably a bit overkill... (Lgb)

// The footnote inset
class InsetFootlike : public InsetCollapsible {
public:
	///
	explicit InsetFootlike(Buffer *);
	///
	bool hasSettings() const override { return false; }
private:
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(std::ostream & os) const override;
	///
	bool insetAllowed(InsetCode) const override;
	/** returns false if, when outputting LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool inheritFont() const override { return false; }
};


} // namespace lyx

#endif
