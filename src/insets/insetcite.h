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
	std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetCitation(params()));
	}
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::CITE_CODE; }
	///
	int plaintext(Buffer const &, std::ostream &, int linelen) const;
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;

private:
	/// This function does the donkey work of creating the pretty label
	std::string const generateLabel(Buffer const &) const;

	struct Cache {
		///
		Cache() : engine(biblio::ENGINE_BASIC) {}
		///
		biblio::CiteEngine engine;
		///
		InsetCommandParams params;
		///
		std::string generated_label;
		///
		std::string screen_label;
	};
	///
	mutable Cache cache;
};

#endif // INSET_CITE_H
