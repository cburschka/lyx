/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998 The LyX Team.
 *
 *======================================================*/

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
// Lgb

InsetFloat::InsetFloat(string const & type)
	: InsetCollapsable()
{
	string lab(_("float:"));
	lab += type;
	setLabel(lab);
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::footnote);
	setLabelFont(font);
	setAutoCollapse(false);
	setInsetName("Float");
	floatType = type;
	setInsetName(type.c_str());
	//floatPlacement = "H";
}


void InsetFloat::Write(Buffer const * buf, ostream & os) const
{
	os << getInsetName()
	   << " " << floatType << '\n';

	if (floatPlacement.empty()) {
		os << "placement "
		   << floatList.getType(floatType).placement << "\n";
	} else {
		os << "placement " << floatPlacement << "\n";
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
			floatPlacement = lex.GetString();
		} else {
			lyxerr << "InsetFloat::Read: Missing placement!"
			       << endl;
		}
	}
	InsetCollapsable::Read(buf, lex);
}


void InsetFloat::Validate(LaTeXFeatures & features) const
{
	features.usedFloats.insert(floatType);
}


Inset * InsetFloat::Clone() const
{
	InsetFloat * result = new InsetFloat(floatType);
	result->inset->init(inset);

	result->collapsed = collapsed;
	return result;
}


char const * InsetFloat::EditMessage() const
{
	return _("Opened Float Inset");
}


int InsetFloat::Latex(Buffer const * buf,
		      ostream & os, bool fragile, bool fp) const
{
	os << "\\begin{" << floatType << "}";
	if (!floatPlacement.empty()
	    && floatPlacement != floatList.defaultPlacement(floatType))
		os << "[" << floatPlacement << "]";
	os << "%\n";
    
	int i = inset->Latex(buf, os, fragile, fp);
	os << "\\end{" << floatType << "}%\n";
	
	return i + 2;
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
	if (x >= 0
	    && x < button_length
	    && y >= button_top_y
	    && y < button_bottom_y
	    && button == 3) {
		// This obviously need to change.
		lyxerr << "InsetFloat: Let's edit this floats parameters!"
		       << endl;
	} else {
		InsetCollapsable::InsetButtonRelease(bv, x, y, button);
	}
}


void InsetFloat::wide(bool w)
{
	wide_ = w;
	if (wide_) {
		string lab(_("float:"));
		lab += floatType;
		lab += "*";
		setLabel(lab);
	} else {
		string lab(_("float:"));
		lab += floatType;
		setLabel(lab);
	}
}


bool InsetFloat::wide() const
{
	return wide_;
}
