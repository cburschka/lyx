// -*- C++ -*-
/**
 * \file insetlist.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETLIST_H
#define INSETLIST_H


#include "insetcollapsable.h"

/** The list inset

*/
class InsetList : public InsetCollapsable {
public:
	///
	InsetList();
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	Inset::Code lyxCode() const { return Inset::FOOT_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	string const editMessage() const;
};

#endif
