// -*- C++ -*-
/**
 * \file insetcite.h
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


#include "insetcommand.h"
#include "bufferparams.h"


/** Used to insert citations
 */
class InsetCitation : public InsetCommand {
public:
	///
	InsetCitation(InsetCommandParams const &);
	///
	lyx::docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetBase::Code lyxCode() const { return InsetBase::CITE_CODE; }
	///
	int plaintext(Buffer const &, lyx::odocstream &, OutputParams const &) const;
	///
	int latex(Buffer const &, lyx::odocstream &,
		  OutputParams const &) const;
	///
	int docbook(Buffer const &, lyx::odocstream &,
		  OutputParams const &) const;
	/// the string that is passed to the TOC
	virtual int textString(Buffer const &, lyx::odocstream &,
		OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	void replaceContents(std::string const & from, std::string const & to);

private:
	virtual std::auto_ptr<InsetBase> doClone() const
	{
		return std::auto_ptr<InsetBase>(new InsetCitation(params()));
	}

	/// This function does the donkey work of creating the pretty label
	lyx::docstring const generateLabel(Buffer const &) const;

	class Cache {
	public:
		///
		Cache() : engine(lyx::biblio::ENGINE_BASIC), params("cite") {}
		///
		lyx::biblio::CiteEngine engine;
		///
		InsetCommandParams params;
		///
		lyx::docstring generated_label;
		///
		lyx::docstring screen_label;
	};
	///
	mutable Cache cache;
};

#endif // INSET_CITE_H
