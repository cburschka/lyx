// -*- C++ -*-
/**
 * \file insetcite.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_CITE_H
#define INSET_CITE_H


#include "insetcommand.h"

/** Used to insert citations
 */
class InsetCitation : public InsetCommand {
public:
	///
	InsetCitation(InsetCommandParams const &);
	///
	~InsetCitation();
	///
	std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetCitation(params()));
	}
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::CITE_CODE; }
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	dispatch_result localDispatch(FuncRequest const & cmd);
	///
	void validate(LaTeXFeatures &) const;
	/** Invoked by BufferView::Pimpl::dispatch when a new citation key
	    is inserted. Tells us that the buffer is no longer being loaded
	    and that the cache of BibTeX keys should be reloaded in the future.
	*/
	void setLoadingBuffer(Buffer const * buffer, bool state) const;
private:
	struct Cache {
		///
		enum Style {
			///
			BASIC,
			///
			NATBIB_AY,
			///
			NATBIB_NUM
		};
		///
		Cache() : style(BASIC) {}
		///
		Style style;
		///
		InsetCommandParams params;
		///
		string generated_label;
		///
		string screen_label;
	};

	/// This function does the donkey work of creating the pretty label
	string const generateLabel(Buffer const *) const;
	///
	Cache::Style getStyle(Buffer const * buffer) const;

	///
	mutable Cache cache;
};

#endif // INSET_CITE_H
