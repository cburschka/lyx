// -*- C++ -*-
/**
 * \file insettoc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_TOC_H
#define INSET_TOC_H

#include "insetcommand.h"


/// Used to insert table of contents and similar lists
class InsetTOC : public InsetCommand {
public:
	///
	explicit InsetTOC(InsetCommandParams const &);
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetBase::Code lyxCode() const;
	///
	bool display() const { return true; }
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
