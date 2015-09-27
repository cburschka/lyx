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
	bool isLabeled() const { return true; }
	///
	bool hasSettings() const { return true; }
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	InsetCode lyxCode() const { return CITE_CODE; }
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void toString(odocstream &) const;
	///
	void forOutliner(docstring &, size_t) const;
	///
	void validate(LaTeXFeatures &) const {}
	///
	void updateBuffer(ParIterator const & it, UpdateType);
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype) const;
	///
	std::string contextMenuName() const;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "cite"; }
	///
	static bool isCompatibleCommand(std::string const & cmd);
	//@}

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
	Inset * clone() const { return new InsetCitation(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const;
	//@}

	/// we'll eventually want to be able to get info on this from the
	/// various CiteEngines
	static ParamInfo param_info_;

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
