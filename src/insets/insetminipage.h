// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998 The LyX Team.
 *
 *======================================================
 */

#ifndef InsetMinipage_H
#define InsetMinipage_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"
#include "vspace.h"
#include <sigc++/signal_system.h>

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
	InsetMinipage();
	///
	~InsetMinipage();
	///
	void Write(Buffer const * buf, std::ostream & os) const;
	///
	void Read(Buffer const * buf, LyXLex & lex);
	///
	Inset * Clone(Buffer const &) const;
	///
	Inset::Code LyxCode() const { return Inset::MINIPAGE_CODE; }
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const EditMessage() const;
	///
	bool InsertInsetAllowed(Inset * inset) const;
	///
	Position pos() const;
	///
	void pos(Position);
	///
	InnerPosition innerPos() const;
	///
	void innerPos(InnerPosition);
	///
	LyXLength const & height() const;
	///
	void height(LyXLength const &);
	///
	string const & width() const;
	///
	void width(string const &);
	///
	int widthp() const;
	///
	void widthp(int);
	///
	void widthp(string const &);
	///
	SigC::Signal0<void> hideDialog;
	///
	void InsetButtonRelease(BufferView * bv, int x, int y, int button);

private:
	///
	Position pos_;
	///
	InnerPosition inner_pos_;
	///
	LyXLength height_;
	///
	string width_;
	///
	int widthp_;
};

#endif
