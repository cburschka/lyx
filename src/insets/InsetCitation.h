// -*- C++ -*-
/**
 * \file InsetCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_CITE_H
#define INSET_CITE_H


#include "InsetCommand.h"

#include "BiblioInfo.h"


namespace lyx {


/** Used to insert citations
 */
class InsetCitation : public InsetCommand {
public:
	///
	InsetCitation(InsetCommandParams const &);
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::CITE_CODE; }
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	/// the string that is passed to the TOC
	virtual int textString(Buffer const &, odocstream &,
		OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	void replaceContents(std::string const & from, std::string const & to);

private:
	virtual std::auto_ptr<Inset> doClone() const
	{
		return std::auto_ptr<Inset>(new InsetCitation(params()));
	}

	/// This function does the donkey work of creating the pretty label
	docstring const generateLabel(Buffer const &) const;

	class Cache {
	public:
		///
		Cache() : engine(biblio::ENGINE_BASIC), params("cite") {}
		///
		biblio::CiteEngine engine;
		///
		InsetCommandParams params;
		///
		docstring generated_label;
		///
		docstring screen_label;
	};
	///
	mutable Cache cache;
};


} // namespace lyx

#endif // INSET_CITE_H
