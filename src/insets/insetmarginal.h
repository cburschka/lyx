// -*- C++ -*-
/**
 * \file insetmarginal.h
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


#include "insetfootlike.h"

/** The marginal note inset

*/
class InsetMarginal : public InsetFootlike {
public:
	///
	InsetMarginal(BufferParams const &);
	///
	InsetBase::Code lyxCode() const { return InsetBase::MARGIN_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const & runparams) const;
	///
	std::string const editMessage() const;
protected:
	InsetMarginal(InsetMarginal const &);
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
