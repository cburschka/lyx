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
	InsetMarginal(Buffer *);
	///
	InsetCode lyxCode() const { return MARGIN_CODE; }
	///
	docstring layoutName() const { return from_ascii("Marginal"); }
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const & runparams) const;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype) const;
private:
	///
	Inset * clone() const { return new InsetMarginal(*this); }
};


} // namespace lyx

#endif
