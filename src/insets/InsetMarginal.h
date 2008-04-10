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
	InsetMarginal(Buffer const &);
	///
	InsetCode lyxCode() const { return MARGIN_CODE; }
	///
	docstring name() const { return from_ascii("Marginal"); }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const & runparams) const;
	///
	int docbook(odocstream &, OutputParams const & runparams) const;
	///
	docstring editMessage() const;
	///
	void addToToc(ParConstIterator const &) const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
private:
	///
	Inset * clone() const { return new InsetMarginal(*this); }
};


} // namespace lyx

#endif
