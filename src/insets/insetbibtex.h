// -*- C++ -*-
/**
 * \file InsetBibtex.h
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
#include "InsetCommand.h"

#include "support/filename.h"


namespace lyx {

/** Used to insert BibTeX's information
  */
class InsetBibtex : public InsetCommand {
public:
	///
	InsetBibtex(InsetCommandParams const &);
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetBase::Code lyxCode() const { return InsetBase::BIBTEX_CODE; }
	///
	bool display() const { return true; }
	///
	int latex(Buffer const &, odocstream &, OutputParams const &) const;
	///
	void fillWithBibKeys(Buffer const & buffer,
		std::vector<std::pair<std::string, docstring> > & keys) const;
	///
	std::vector<support::FileName> const getFiles(Buffer const &) const;
	///
	bool addDatabase(std::string const &);
	///
	bool delDatabase(std::string const &);
	///
	void validate(LaTeXFeatures &) const;
protected:
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
private:
	virtual std::auto_ptr<InsetBase> doClone() const;

};


} // namespace lyx

#endif // INSET_BIBTEX_H
