// -*- C++ -*-
/**
 * \file insetmarginal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETMARGINAL_H
#define INSETMARGINAL_H


#include "insetfootlike.h"

/** The marginal note inset

*/
class InsetMarginal : public InsetFootlike {
public:
	///
	InsetMarginal(BufferParams const &);
	///
	InsetMarginal(InsetMarginal const &);
	///
	//InsetMarginal(InsetMarginal const &, bool same_id);
	///
	Inset * clone(Buffer const &) const;
	///
	//Inset * clone(Buffer const &, bool same_id) const;
	///
	Inset::Code lyxCode() const { return Inset::MARGIN_CODE; }
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	string const editMessage() const;
};

#endif
