// -*- C++ -*-
/**
 * \file insettheorem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef InsetTheorem_H
#define InsetTheorem_H


#include "insetcollapsable.h"


namespace lyx {

/** The theorem inset

*/
class InsetTheorem : public InsetCollapsable {
public:
	///
	InsetTheorem();
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	Inset::Code lyxCode() const { return Inset::THEOREM_CODE; }
	///
	bool metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	virtual docstring const editMessage() const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;

	///
	mutable unsigned int center_indent_;
};


} // namespace lyx

#endif
