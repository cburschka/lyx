/**
 * \file insetwrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef InsetWrap_H
#define InsetWrap_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"
#include "lyxlength.h"

#include <boost/signals/signal0.hpp>

class Painter;

/** The wrap inset

*/
class InsetWrap : public InsetCollapsable {
public:
	///
	InsetWrap(BufferParams const &, string const &);
	///
	InsetWrap(InsetWrap const &, bool same_id = false);
	///
	~InsetWrap();
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	Inset::Code lyxCode() const { return Inset::WRAP_CODE; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
	///
	string const editMessage() const;
	///
	bool insetAllowed(Inset::Code) const;
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	string const & type() const;
	///
	LyXLength const & pageWidth() const;
        ///
        void pageWidth(LyXLength const &);
	///
	void placement(string const & p);
	///
	string const & placement() const;
	///
	bool  showInsetDialog(BufferView *) const;
	///
	boost::signal0<void> hideDialog;
	///
	int latexTextWidth(BufferView *) const;
private:
	///
	string Type_;
	///
	string Placement_;
	///
	LyXLength width_;
};

#endif
