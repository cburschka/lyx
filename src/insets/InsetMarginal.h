// -*- C++ -*-
/**
 * \file InsetMarginal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETMARGINAL_H
#define INSETMARGINAL_H


#include "InsetFootlike.h"


namespace lyx {

/** The marginal note inset

*/
class InsetMarginal : public InsetFootlike {
public:
	///
	InsetMarginal(BufferParams const &);
	///
	Inset::Code lyxCode() const { return Inset::MARGIN_CODE; }
	///
	docstring name() const { return from_ascii("Marginal"); }
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const & runparams) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const & runparams) const;
	///
	virtual docstring const editMessage() const;
protected:
	InsetMarginal(InsetMarginal const &);
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif
