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

#include "InsetCommand.h"

namespace lyx {

class BiblioInfo;
class docstring_list;

namespace support {
	class FileName;
} // namespace support

/** Used to insert BibTeX's information
  */
class InsetBibtex : public InsetCommand {
public:
	///
	InsetBibtex(Buffer *, InsetCommandParams const &);

	///
	docstring_list getBibFiles() const;
	///
	bool addDatabase(docstring const &);
	///
	bool delDatabase(docstring const &);
	///
	void write(std::ostream &) const override;

	/// \name Public functions inherited from Inset class
	//@{
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	bool hasSettings() const override { return true; }
	///
	InsetCode lyxCode() const override { return BIBTEX_CODE; }
	///
	RowFlags rowFlags() const override { return Display; }
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	void collectBibKeys(InsetIterator const &, support::FileNameList &) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	std::string contextMenuName() const override;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "bibtex"; }
	///
	static bool isCompatibleCommand(std::string const & s)
		{ return s == "bibtex"; }
	//@}

private:
	///
	void editDatabases(docstring const & db = docstring()) const;
	///
	void parseBibTeXFiles(support::FileNameList &) const;
	///
	bool usingBiblatex() const;
	///
	docstring getRefLabel() const;
	///
	std::map<std::string, std::string> getFileEncodings() const;

	/// \name Private functions inherited from Inset class
	//@{
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	Inset * clone() const override { return new InsetBibtex(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const override;
	//@}
};


} // namespace lyx

#endif // INSET_BIBTEX_H
