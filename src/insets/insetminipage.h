// -*- C++ -*-
/**
 * \file insetminipage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef InsetMinipage_H
#define InsetMinipage_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"
#include "lyxlength.h"

#include <boost/signals/signal0.hpp>

/** The minipage inset

*/
class InsetMinipage : public InsetCollapsable {
public:
	///
	enum Position {
		top,
		center,
		bottom
	};
	///
	enum InnerPosition {
		inner_center,
		inner_top,
		inner_bottom,
		inner_stretch
	};
	///
	InsetMinipage(BufferParams const &);
	///
	InsetMinipage(InsetMinipage const &, bool same_id = false);
	///
	~InsetMinipage();
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	Inset::Code lyxCode() const { return Inset::MINIPAGE_CODE; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const editMessage() const;
	///
	bool insetAllowed(Inset::Code) const;
	///
	Position pos() const;
	///
	void pos(Position);
	///
	InnerPosition innerPos() const;
	///
	void innerPos(InnerPosition);
	///
	LyXLength const & pageHeight() const;
	///
	void pageHeight(LyXLength const &);
	///
	LyXLength const & pageWidth() const;
	///
	void pageWidth(LyXLength const &);
	///
	boost::signal0<void> hideDialog;
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	bool needFullRow() const { return false; }
	/** returns true if, when outputing LaTeX, font changes should
            be closed before generating this inset. This is needed for
            insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	bool showInsetDialog(BufferView *) const;
	///
	int latexTextWidth(BufferView *) const;

private:
	///
	Position pos_;
	///
	InnerPosition inner_pos_;
	///
	LyXLength height_;
	///
	LyXLength width_;
};

#endif
