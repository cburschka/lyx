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
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	string const editMessage() const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;

	///
	mutable unsigned int center_indent_;
};

#endif
