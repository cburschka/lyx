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
#include "FloatList.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "Floating.h"

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
	setAutoCollapse(false);
	floatType_ = type;
	setInsetName(type);
}


void InsetFloat::Write(Buffer const * buf, ostream & os) const
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
	
	InsetCollapsable::Write(buf, os);
}


void InsetFloat::Read(Buffer const * buf, LyXLex & lex)
{
	if (lex.IsOK()) {
		lex.next();
		string token = lex.GetString();
		if (token == "placement") {
			lex.next();
			floatPlacement_ = lex.GetString();
		} else {
			lyxerr << "InsetFloat::Read: Missing placement!"
			       << endl;
		}
		lex.next();
		token = lex.GetString();
		if (token == "wide") {
			lex.next();
			string const tmptoken = lex.GetString();
			if (tmptoken == "true")
				wide(true);
			else
				wide(false);
		} else {
			lyxerr << "InsetFloat::Read:: Missing wide!"
			       << endl;
		}
	}
	InsetCollapsable::Read(buf, lex);
}


void InsetFloat::Validate(LaTeXFeatures & features) const
{
	features.usedFloats.insert(floatType_);
	InsetCollapsable::Validate(features);
}


Inset * InsetFloat::Clone(Buffer const &) const
{
	InsetFloat * result = new InsetFloat(floatType_);
	result->inset.init(&inset);

	result->collapsed = collapsed;
	return result;
}


string const InsetFloat::EditMessage() const
{
	return _("Opened Float Inset");
}


int InsetFloat::Latex(Buffer const * buf,
		      ostream & os, bool fragile, bool fp) const
{
	string const tmptype = (wide_ ? floatType_ + "*" : floatType_);
	
	os << "\\begin{" << tmptype << "}";
	if (!floatPlacement_.empty()
	    && floatPlacement_ != floatList.defaultPlacement(floatType_))
		os << "[" << floatPlacement_ << "]";
	os << "%\n";
    
	int const i = inset.Latex(buf, os, fragile, fp);
	os << "\\end{" << tmptype << "}%\n";
	
	return i + 2;
}


int InsetFloat::DocBook(Buffer const * buf, ostream & os) const
{
	os << "<" << floatType_ << ">";
	int const i = inset.DocBook(buf, os);
	os << "</" << floatType_ << ">";

	return i;
}


bool InsetFloat::InsertInsetAllowed(Inset * in) const
{
	if ((in->LyxCode() == Inset::FOOT_CODE) ||
	    (in->LyxCode() == Inset::MARGIN_CODE)) {
		return false;
	}
	return true;
}


void InsetFloat::InsetButtonRelease(BufferView * bv, int x, int y, int button)
{
	if (x >= top_x
	    && x < button_length
	    && y >= button_top_y
	    && y < button_bottom_y
	    && button == 3) {
		// This obviously need to change.
		lyxerr << "InsetFloat: Let's edit this floats parameters!"
		       << endl;
		//bv->owner()->getDialogs()->showFloat(this);
	} else {
		InsetCollapsable::InsetButtonRelease(bv, x, y, button);
	}
}


string const & InsetFloat::type() const 
{
	return floatType_;
}


void InsetFloat::placement(string const & p)
{
	// Here we should only allow the placement to be set
	// if a valid value.
#ifdef WITH_WARNINGS
#warning FIX!
#endif
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
