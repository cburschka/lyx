// -*- C++ -*-
/**
 * \file insetfoot.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETFOOT_H
#define INSETFOOT_H

#include "insetfootlike.h"

/** The footnote inset

*/
class InsetFoot : public InsetFootlike {
public:
	///
	InsetFoot(BufferParams const &);
	///
	InsetOld::Code lyxCode() const { return InsetOld::FOOT_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const & runparams) const;
	///
	std::string const editMessage() const;
protected:
	InsetFoot(InsetFoot const &);
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
