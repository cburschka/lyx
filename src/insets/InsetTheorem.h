// -*- C++ -*-
/**
 * \file Inset.heorem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#if 0

#ifndef INSETTHEOREM_H
#define INSETTHEOREM_H

#include "InsetCollapsable.h"


namespace lyx {

/** The theorem inset

*/
class Inset.heorem : public InsetCollapsable {
public:
	///
	Inset.heorem();
	///
	void write(std::ostream & os) const;
	///
	InsetCode lyxCode() const { return THEOREM_CODE; }
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool isMacroScope() const { return true; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	docstring editMessage() const;
private:
	Inset * clone() const;

	///
	mutable unsigned int center_indent_;
};


} // namespace lyx

#endif

#endif
