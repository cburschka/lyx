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
#include "EmbeddedFiles.h"
#include "InsetCommand.h"

#include <map>

namespace lyx {


/** Used to insert BibTeX's information
  */
class InsetBibtex : public InsetCommand {
public:
	///
	InsetBibtex(InsetCommandParams const &);
	///
	virtual void setBuffer(Buffer & buffer);
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
	void fillWithBibKeys(BiblioInfo &, InsetIterator const &) const;
	///
	EmbeddedFileList const & getBibFiles() const;
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
	/// create bibfiles_ from params bibfiles and embed
	/**
		\param bibfiles comma separated bib files
		\param embed comma separated embed status
	*/
	void createBibFiles(docstring const & bibfiles, docstring const & embed) const;
	/// update bibfiles and embed from bibfiles_
	void updateParam();
private:
	///
	void registerEmbeddedFiles(EmbeddedFileList &) const;
	///
	void updateEmbeddedFile(EmbeddedFile const & file);
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetBibtex(*this); }
	/// embedded bib files
	mutable EmbeddedFileList bibfiles_;
};


} // namespace lyx

#endif // INSET_BIBTEX_H
