/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetfloat.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "support/lstrings.h"
#include "FloatList.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "Floating.h"
#include "buffer.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"

using std::ostream;
using std::endl;

// With this inset it will be possible to support the latex package
// float.sty, and I am sure that with this and some additional support
// classes we can support similar functionality in other formats
// (read DocBook).
// By using float.sty we will have the same handling for all floats, both
// for those already in existance (table and figure) and all user created
// ones¹. So suddenly we give the users the possibility of creating new
// kinds of floats on the fly. (and with a uniform look)
//
// API to float.sty:
//   \newfloat{type}{placement}{ext}[within]
//     type      - The "type" of the new class of floats, like program or
//                 algorithm. After the appropriate \newfloat, commands
//                 such as \begin{program} or \end{algorithm*} will be
//                 available.
//     placement - The default placement for the given class of floats.
//                 They are like in standard LaTeX: t, b, p and h for top,
//                 bottom, page, and here, respectively. On top of that
//                 there is a new type, H, which does not really correspond
//                 to a float, since it means: put it "here" and nowhere else.
//                 Note, however that the H specifier is special and, because
//                 of implementation details cannot be used in the second
//                 argument of \newfloat.
//     ext       - The file name extension of an auxiliary file for the list
//                 of figures (or whatever). LaTeX writes the captions to
//                 this file.
//     within    - This (optional) argument determines whether floats of this
//                 class will be numbered within some sectional unit of the
//                 document. For example, if within is equal to chapter, the
//                 floats will be numbered within chapters. 
//   \floatstyle{style}
//     style -  plain, boxed, ruled
//   \floatname{float}{floatname}
//     float     -
//     floatname -
//   \floatplacement{float}{placement}
//     float     -
//     placement -
//   \restylefloat{float}
//     float -
//   \listof{type}{title}
//     title -

// ¹ the algorithm float is defined using the float.sty package. Like this
//   \floatstyle{ruled}
//   \newfloat{algorithm}{htbp}{loa}[<sect>]
//   \floatname{algorithm}{Algorithm}
//
// The intention is that floats should be definable from two places:
//          - layout files
//          - the "gui" (i.e. by the user)
//
// From layout files.
// This should only be done for floats defined in a documentclass and that
// does not need any additional packages. The two most known floats in this
// category is "table" and "figure". Floats defined in layout files are only
// stored in lyx files if the user modifies them.
//
// By the user.
// There should be a gui dialog (and also a collection of lyxfuncs) where
// the user can modify existing floats and/or create new ones.
//
// The individual floats will also have some settable
// variables: wide and placement.
//
// Lgb

InsetFloat::InsetFloat(string const & type)
	: InsetCollapsable(), wide_(false)
{
	string lab(_("float:"));
	lab += type;
	setLabel(lab);
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	floatType_ = type;
	setInsetName(type);
}


InsetFloat::InsetFloat(InsetFloat const & in, bool same_id)
	: InsetCollapsable(in, same_id), floatType_(in.floatType_),
	  floatPlacement_(in.floatPlacement_), wide_(in.wide_)
{}


InsetFloat::~InsetFloat()
{
	hideDialog();
}


void InsetFloat::write(Buffer const * buf, ostream & os) const
{
	os << "Float " // getInsetName()
	   << floatType_ << '\n';

	if (floatPlacement_.empty()) {
		os << "placement "
		   << floatList.getType(floatType_).placement() << "\n";
	} else {
		os << "placement " << floatPlacement_ << "\n";
	}
	if (wide_) {
		os << "wide true\n";
	} else {
		os << "wide false\n";
	}
	
	InsetCollapsable::write(buf, os);
}


void InsetFloat::read(Buffer const * buf, LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
		if (token == "placement") {
			lex.next();
			floatPlacement_ = lex.getString();
		} else {
			lyxerr << "InsetFloat::Read: Missing placement!"
			       << endl;
			// take countermeasures
			lex.pushToken(token);
		}
		lex.next();
		token = lex.getString();
		if (token == "wide") {
			lex.next();
			string const tmptoken = lex.getString();
			if (tmptoken == "true")
				wide(true);
			else
				wide(false);
		} else {
			lyxerr << "InsetFloat::Read:: Missing wide!"
			       << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
	InsetCollapsable::read(buf, lex);
}


void InsetFloat::validate(LaTeXFeatures & features) const
{
	if (contains(placement(), "H")) {
		features.require("floats");
	}
	
	features.useFloat(floatType_);
	InsetCollapsable::validate(features);
}


Inset * InsetFloat::clone(Buffer const &, bool same_id) const
{
	return new InsetFloat(*const_cast<InsetFloat *>(this), same_id);
}


string const InsetFloat::editMessage() const
{
	return _("Opened Float Inset");
}


int InsetFloat::latex(Buffer const * buf,
		      ostream & os, bool fragile, bool fp) const
{
	string const tmptype = (wide_ ? floatType_ + "*" : floatType_);
	// Figure out the float placement to use.
	// From lowest to highest:
	// - float default placement
	// - document wide default placement
	// - specific float placement
	string placement;
	string const buf_placement = buf->params.float_placement;
	string const def_placement = floatList.defaultPlacement(floatType_);
	if (!floatPlacement_.empty()
	    && floatPlacement_ != def_placement) {
		placement = floatPlacement_;
	} else if (!buf_placement.empty()
		   && buf_placement != def_placement) {
		placement = buf_placement;
	}
	
	os << "\\begin{" << tmptype << "}";
	// We only output placement if different from the def_placement.
	if (!placement.empty()) {
		os << "[" << placement << "]";
	}
	
	os << "%\n";
    
	int const i = inset.latex(buf, os, fragile, fp);
	os << "\\end{" << tmptype << "}%\n";
	
	return i + 2;
}


int InsetFloat::docbook(Buffer const * buf, ostream & os) const
{
	os << "<" << floatType_ << ">";
	int const i = inset.docbook(buf, os);
	os << "</" << floatType_ << ">";

	return i;
}


bool InsetFloat::insetAllowed(Inset::Code code) const
{
	if (code == Inset::FLOAT_CODE)
		return false;
	if (inset.getLockingInset() != const_cast<InsetFloat *>(this))
		return inset.insetAllowed(code);
	if ((code == Inset::FOOT_CODE) || (code == Inset::MARGIN_CODE))
		return false;
	return true;
}


bool InsetFloat::showInsetDialog(BufferView * bv) const
{
	if (!inset.showInsetDialog(bv)) {
		bv->owner()->getDialogs()->showFloat(const_cast<InsetFloat *>(this)); 
	}
	return true;
}


void InsetFloat::insetButtonRelease(BufferView * bv, int x, int y, int button)
{
#if 1
	if ((x >= 0)  && (x < button_length) &&
	    (y >= button_top_y) &&  (y <= button_bottom_y) &&
	    (button == 3))
	{
		showInsetDialog(bv);
		return;
	}
#else
	if (button == 3) {
		showInsetDialog(bv);
		return;
	}
#endif
	InsetCollapsable::insetButtonRelease(bv, x, y, button);
}


string const & InsetFloat::type() const 
{
	return floatType_;
}


void InsetFloat::placement(string const & p)
{
	// FIX: Here we should only allow the placement to be set
	// if a valid value.
	floatPlacement_ = p;
}


string const & InsetFloat::placement() const
{
	return floatPlacement_;
}


void InsetFloat::wide(bool w)
{
	wide_ = w;
	if (wide_) {
		string lab(_("float:"));
		lab += floatType_;
		lab += "*";
		setLabel(lab);
	} else {
		string lab(_("float:"));
		lab += floatType_;
		setLabel(lab);
	}
}


bool InsetFloat::wide() const
{
	return wide_;
}
