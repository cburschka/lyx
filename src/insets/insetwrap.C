/**
 * \file insetwrap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetwrap.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "support/lstrings.h"
#include "LaTeXFeatures.h"
#include "debug.h"
#include "buffer.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "lyxlex.h"
#include "FloatList.h"

using std::ostream;
using std::endl;

namespace {

// this should not be hardcoded, but be part of the definition
// of the float (JMarc)
string const caplayout("Caption");
string floatname(string const & type, BufferParams const & bp)
{
	FloatList const & floats = bp.getLyXTextClass().floats();
	FloatList::const_iterator it = floats[type];
	if (it == floats.end())
		return type;

	return _(it->second.name());
}

} // namespace anon


InsetWrap::InsetWrap(BufferParams const & bp, string const & type)
	: InsetCollapsable(bp), width_(50, LyXLength::PCW)
{
	string lab(_("wrap: "));
	lab += floatname(type, bp);
	setLabel(lab);
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	Type_ = type;
	setInsetName(type);
	LyXTextClass const & tclass = bp.getLyXTextClass();
	if (tclass.hasLayout(caplayout))
		inset.paragraph()->layout(tclass[caplayout]);
}


InsetWrap::InsetWrap(InsetWrap const & in, bool same_id)
	: InsetCollapsable(in, same_id), Type_(in.Type_),
	  Placement_(in.Placement_), width_(in.width_)
{}


InsetWrap::~InsetWrap()
{
	hideDialog();
}


void InsetWrap::write(Buffer const * buf, ostream & os) const
{
	os << "Wrap " // getInsetName()
	   << Type_ << '\n';

	if (!Placement_.empty()) {
		os << "placement " << Placement_ << "\n";
	}
	os << "width \"" << width_.asString() << "\"\n";

	InsetCollapsable::write(buf, os);
}


void InsetWrap::read(Buffer const * buf, LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
		if (token == "placement") {
			lex.next();
			Placement_ = lex.getString();
		} else {
			// take countermeasures
			lex.pushToken(token);
		}
	}
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
		if (token == "width") {
			lex.next();
			width_ = LyXLength(lex.getString());
		} else {
			lyxerr << "InsetWrap::Read:: Missing 'width'-tag!"
			       << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
	InsetCollapsable::read(buf, lex);
}


void InsetWrap::validate(LaTeXFeatures & features) const
{
	features.require("floatflt");
	InsetCollapsable::validate(features);
}


Inset * InsetWrap::clone(Buffer const &, bool same_id) const
{
	return new InsetWrap(*const_cast<InsetWrap *>(this), same_id);
}


string const InsetWrap::editMessage() const
{
	return _("Opened Wrap Inset");
}


int InsetWrap::latex(Buffer const * buf,
		      ostream & os, bool fragile, bool fp) const
{
	os << "\\begin{floating" << Type_ << "}";
	if (!Placement_.empty()) {
		os << "[" << Placement_ << "]";
	}
	os  << "{" << width_.asLatexString() << "}%\n";

	int const i = inset.latex(buf, os, fragile, fp);

	os << "\\end{floating" << Type_ << "}%\n";
	return i + 2;
}


int InsetWrap::docbook(Buffer const * buf, ostream & os, bool mixcont) const
{
	os << "<" << Type_ << ">";
	int const i = inset.docbook(buf, os, mixcont);
	os << "</" << Type_ << ">";

	return i;
}


bool InsetWrap::insetAllowed(Inset::Code code) const
{
	switch(code) {
	case FLOAT_CODE:
	case FOOT_CODE:
	case MARGIN_CODE:
                return false;
	default:
		return InsetCollapsable::insetAllowed(code);
	}
}

int InsetWrap::getMaxWidth(BufferView * bv, UpdatableInset const * inset)
	const
{
	if (owner() &&
	    static_cast<UpdatableInset*>(owner())->getMaxWidth(bv, inset) < 0) {
		return -1;
	}
	if (!width_.zero()) {
		int ww1 = latexTextWidth(bv);
		int ww2 = InsetCollapsable::getMaxWidth(bv, inset);
		if (ww2 > 0 && ww2 < ww1) {
			return ww2;
		}
		return ww1;
	}
	// this should not happen!
	return InsetCollapsable::getMaxWidth(bv, inset);
}


int InsetWrap::latexTextWidth(BufferView * bv) const
{
	return width_.inPixels(InsetCollapsable::latexTextWidth(bv));
}


string const & InsetWrap::type() const
{
	return Type_;
}


LyXLength const & InsetWrap::pageWidth() const
{
	return width_;
}


void InsetWrap::pageWidth(LyXLength const & ll)
{
	if (ll != width_) {
		width_ = ll;
		need_update = FULL;
	}
}


void InsetWrap::placement(string const & p)
{
	Placement_ = p;
}


string const & InsetWrap::placement() const
{
	return Placement_;
}


bool InsetWrap::showInsetDialog(BufferView * bv) const
{
	if (!inset.showInsetDialog(bv)) {
		bv->owner()->getDialogs().showWrap(const_cast<InsetWrap *>(this));
	}
	return true;
}


void InsetWrap::addToToc(toc::TocList & toclist, Buffer const * buf) const
{
	// Now find the caption in the float...
	// We now tranverse the paragraphs of
	// the inset...
	Paragraph * tmp = inset.paragraph();
	while (tmp) {
		if (tmp->layout()->name() == caplayout) {
			string const str =
				tostr(toclist[type()].size() + 1)
				+ ". " + tmp->asString(buf, false);
			toc::TocItem const item(tmp, 0 , str);
			toclist[type()].push_back(item);
		}
		tmp = tmp->next();
	}
}
