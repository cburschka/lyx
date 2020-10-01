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
	explicit InsetMarginal(Buffer *);
	///
	InsetCode lyxCode() const override { return MARGIN_CODE; }
	///
	docstring layoutName() const override { return from_ascii("Marginal"); }
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const & runparams) const override;
	/// Is the content of this inset part of the immediate (visible) text sequence?
	bool isPartOfTextSequence() const override { return false; }
private:
	///
	Inset * clone() const override { return new InsetMarginal(*this); }
};


} // namespace lyx

#endif
