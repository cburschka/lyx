// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_CITE_H
#define INSET_CITE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

/** Used to insert citations
 */
class InsetCitation : public InsetCommand {
public:
	///
	InsetCitation(InsetCommandParams const &, bool same_id = false);
	///
	Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetCitation(params(), same_id);
	}
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::CITE_CODE; }
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	void edit(BufferView * bv, bool front = true);
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int latex(Buffer const *, std::ostream &, bool, bool) const;
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
