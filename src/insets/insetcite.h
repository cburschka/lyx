// -*- C++ -*-
/**
 * \file insetcite.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

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
	void edit(BufferView *, int, int, mouse_button::state);
	///
	void edit(BufferView * bv, bool front = true);
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int latex(Buffer const *, std::ostream &, bool, bool) const;
	///
	void validate(LaTeXFeatures &) const;
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
