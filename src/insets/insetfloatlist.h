// -*- C++ -*-
/**
 * \file insetfloatlist.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FLOATLIST_H
#define INSET_FLOATLIST_H


#include "insetcommand.h"

/** Used to insert table of contents
 */
class InsetFloatList : public InsetCommand {
public:
	///
	InsetFloatList();
	///
	InsetFloatList(std::string const & type);
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const;
	///
	bool display() const { return true; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, LyXLex &);
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const { return 0; }
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const { return 0; }
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const & runparams) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const
	{
		return std::auto_ptr<InsetBase>(new InsetFloatList(getCmdName()));
	}
};

#endif
