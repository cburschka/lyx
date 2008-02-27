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

#include <map>
#include "InsetCommand.h"
#include "BiblioInfo.h"

namespace lyx {

class EmbeddedFileList;


/** Used to insert BibTeX's information
  */
class InsetBibtex : public InsetCommand {
public:
	///
	InsetBibtex(InsetCommandParams const &);
	///
	docstring screenLabel() const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetCode lyxCode() const { return BIBTEX_CODE; }
	///
	DisplayType display() const { return AlignCenter; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	virtual void fillWithBibKeys(BiblioInfo &, InsetIterator const &) const;
	///
	EmbeddedFileList const getFiles(Buffer const &) const;
	///
	bool addDatabase(std::string const &);
	///
	bool delDatabase(std::string const &);
	///
	void validate(LaTeXFeatures &) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "bibtex"; };
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "bibtex"; }
	///
	void registerEmbeddedFiles(Buffer const &, EmbeddedFileList &) const;
	///
	void updateEmbeddedFile(Buffer const & buf, EmbeddedFile const & file);
	
protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	///
	virtual Inset * clone() const;

};


} // namespace lyx

#endif // INSET_BIBTEX_H
