// -*- C++ -*-
/**
 * \file insetbibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_BIBTEX_H
#define INSET_BIBTEX_H


#include <vector>
#include "insetcommand.h"

/** Used to insert BibTeX's information
  */
class InsetBibtex : public InsetCommand {
public:
	///
	InsetBibtex(InsetCommandParams const &);
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::BIBTEX_CODE; }
	///
	bool display() const { return true; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	void fillWithBibKeys(Buffer const & buffer,
		std::vector<std::pair<std::string,std::string> > & keys) const;
	///
	std::vector<std::string> const getFiles(Buffer const &) const;
	///
	bool addDatabase(std::string const &);
	///
	bool delDatabase(std::string const &);
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(LCursor & cur, FuncRequest const & cmd);
};

#endif // INSET_BIBTEX_H
