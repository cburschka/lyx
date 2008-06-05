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

#include "BiblioInfo.h"
#include "InsetCommand.h"

#include "support/FileNameList.h"

#include <map>

namespace lyx {

/** Used to insert BibTeX's information
  */
class InsetBibtex : public InsetCommand {
public:
	///
	InsetBibtex(Buffer const &, InsetCommandParams const &);
	///
	virtual ~InsetBibtex();
	///
	docstring screenLabel() const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetCode lyxCode() const { return BIBTEX_CODE; }
	///
	DisplayType display() const { return AlignCenter; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	void fillWithBibKeys(BiblioInfo &, InsetIterator const &) const;
	///
	support::FileNameList getBibFiles() const;
	///
	bool addDatabase(docstring const &);
	///
	bool delDatabase(docstring const &);
	///
	void validate(LaTeXFeatures &) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "bibtex"; };
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "bibtex"; }
	/// look up the path to the file using TeX
	static support::FileName 
		getBibTeXPath(docstring const & filename, Buffer const & buf);
	///
	docstring contextMenu(BufferView const &, int, int) const;
private:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;
	///
	void editDatabases() const;
	///
	Inset * clone() const { return new InsetBibtex(*this); }
};


} // namespace lyx

#endif // INSET_BIBTEX_H
