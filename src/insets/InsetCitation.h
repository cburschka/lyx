// -*- C++ -*-
/**
 * \file InsetCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_CITATION_H
#define INSET_CITATION_H

#include "InsetCommand.h"

namespace lyx {

class CitationStyle;

/////////////////////////////////////////////////////////////////////////
//
// InsetCitation
//
/////////////////////////////////////////////////////////////////////////

/// Used to insert citations
class InsetCitation : public InsetCommand
{
public:
	///
	InsetCitation(Buffer * buf, InsetCommandParams const &);
	///
	~InsetCitation();

	///
	bool addKey(std::string const & key);

	/// \name Public functions inherited from Inset class
	//@{
	///
	bool isLabeled() const override { return true; }
	///
	bool hasSettings() const override { return true; }
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;
	///
	InsetCode lyxCode() const override { return CITE_CODE; }
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void toString(odocstream &) const override;
	///
	void forOutliner(docstring &, size_t const, bool const) const override;
	///
	void updateBuffer(ParIterator const & it, UpdateType, bool const deleted = false) override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	std::string contextMenuName() const override;
	///
	bool forceLTR(OutputParams const &) const override;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "cite"; }
	///
	static bool isCompatibleCommand(std::string const &);
	//@}
	///
	typedef std::vector<std::pair<docstring, docstring>> QualifiedList;
	///
	void redoLabel() { cache.recalculate = true; }
	///
	CitationStyle getCitationStyle(BufferParams const & bp, std::string const & input,
				       std::vector<CitationStyle> const & valid_styles) const;
	///
	QualifiedList getQualifiedLists(docstring const & p) const;
	///
	static bool last_literal;
	/// Check whether citation contains necessary url/file entries
	/// or external search script is available
	bool openCitationPossible() const;
	/// search and open citation source
	void openCitation();

private:
	/// tries to make a pretty label and makes a basic one if not
	docstring generateLabel(bool for_xhtml = false) const;
	/// makes a pretty label
	docstring complexLabel(bool for_xhtml = false) const;
	/// makes a very basic label, in case we can't make a pretty one
	docstring basicLabel(bool for_xhtml = false) const;

	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const override { return new InsetCitation(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const override;
	//@}

	///
	struct Cache {
		Cache() : recalculate(true) {}
		///
		bool recalculate;
		///
		docstring generated_label;
		///
		docstring screen_label;
	};
	///
	mutable Cache cache;
};


} // namespace lyx

#endif // INSET_CITATION_H
