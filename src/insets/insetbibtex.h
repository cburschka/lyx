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
	~InsetBibtex();
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// small wrapper for the time being
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	///
	string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::BIBTEX_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	void fillWithBibKeys(Buffer const & buffer,
		std::vector<std::pair<string,string> > & keys) const;
	///
	std::vector<string> const getFiles(Buffer const &) const;
	///
	bool addDatabase(string const &);
	///
	bool delDatabase(string const &);
private:
	///
	mutable unsigned int center_indent_;
};

#endif // INSET_BIBTEX_H
