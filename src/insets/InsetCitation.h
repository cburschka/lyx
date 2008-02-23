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
#include "InsetCode.h"

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
	InsetCode lyxCode() const { return CITE_CODE; }
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
	void textString(Buffer const &, odocstream &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	//FIXME This is the locus of the design problem we have.
	//It really ought to do what default_cite_command() does,
	//but to do that it needs to know what CiteEngine we are
	//using.
	///
	static std::string defaultCommand() { return "cite"; };
	///
	static bool isCompatibleCommand(std::string const & cmd);
private:
	///
	virtual Inset * clone() const
		{ return new InsetCitation(params()); }
	/// we'll eventually want to be able to get info on this from the 
	/// various CiteEngines
	static ParamInfo param_info_;
	/// This function does the donkey work of creating the pretty label
	docstring const generateLabel(Buffer const &) const;

	///
	class Cache {
	public:
		///
		Cache() : engine(biblio::ENGINE_BASIC), params(CITE_CODE) {}
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
