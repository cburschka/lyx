#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_boxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"

#include "math_cursor.h"
#include "commandtags.h"
#include "formulabase.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"
#include "frontends/Dialogs.h"
#include "lyxfunc.h"
#include "gettext.h"
#include "LaTeXFeatures.h"


ButtonInset::ButtonInset()
	: MathNestInset(2)
{}


void ButtonInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "textnormal");
	if (editing()) {
		MathNestInset::metrics(mi);
		width_   = xcell(0).width() + xcell(1).width() + 4;
		ascent_  = max(xcell(0).ascent(), xcell(1).ascent());
		descent_ = max(xcell(0).descent(), xcell(1).descent());
	} else {
		string s = screenLabel();
		mathed_string_dim(mi.base.font,
				 s, ascent_, descent_, width_);
		width_ += 10;
	}
}


void ButtonInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
	if (editing()) {
		xcell(0).draw(pi, x, y);
		xcell(1).draw(pi, x + xcell(0).width() + 2, y);
		mathed_draw_framebox(pi, x, y, this);
	} else {
		pi.pain.buttonText(x + 2, y, screenLabel(),
			pi.base.font);
	}
}


////////////////////////////////

CommandInset::CommandInset(string const & data)
{
	lock_ = true;

	string::size_type idx0 = data.find("|++|");
	name_ = data.substr(0, idx0);
	if (idx0 == string::npos)
		return;
	idx0 += 4;
	string::size_type idx1 = data.find("|++|", idx0);
	cell(0) = asArray(data.substr(idx0, idx1 - idx0));
	if (idx1 == string::npos)
		return;
	cell(1) = asArray(data.substr(idx1 + 4));
}


MathInset * CommandInset::clone() const
{
	return new CommandInset(*this);
}


void CommandInset::write(WriteStream & os) const
{
	os << "\\" << name_;
	if (cell(1).size())
		os << "[" << cell(1) << "]";
	os << "{" << cell(0) << "}";
}


string CommandInset::screenLabel() const
{
	return name_;
}

////////////////////////////////

RefInset::RefInset()
	: CommandInset("ref")
{}


RefInset::RefInset(string const & data)
	: CommandInset(data)
{}


MathInset * RefInset::clone() const
{
	return new RefInset(*this);
}


void RefInset::infoize(std::ostream & os) const
{
	os << "Ref: " << cell(0);
}


int RefInset::dispatch(string const & cmd, idx_type, pos_type) 
{
	if (cmd == "mouse 3") {
		cerr << "trying to goto ref" << cell(0) << "\n";
		mathcursor->formula()->view()->owner()->getLyXFunc()->
			dispatch(LFUN_REF_GOTO, asString(cell(0)));
		return 1; // dispatched
	}
	
	if (cmd == "mouse 1") {
		cerr << "trying to open ref" << cell(0) << "\n";
		// Eventually trigger dialog with button 3 not 1
//		mathcursor->formula()->view()->owner()->getDialogs()
//			->showRef(this);
		return 1; // dispatched
	}

	return 0; // undispatched
}


string RefInset::screenLabel() const
{
	string str;
	for (int i = 0; !types[i].latex_name.empty(); ++i)
		if (name_ == types[i].latex_name) {
			str = _(types[i].short_gui_name);
			break;
		}
	str += asString(cell(0));

	//if (/* !isLatex && */ !cell(0).empty()) {
	//	str += "||";
	//	str += asString(cell(1));
	//}
	return str;
}


void RefInset::validate(LaTeXFeatures & features) const
{
	if (name_ == "vref" || name_ == "vpageref")
		features.require("varioref");
	else if (name_ == "prettyref")
		features.require("prettyref");
}


int RefInset::ascii(std::ostream & os, int) const
{
	os << "[" << asString(cell(0)) << "]";
	return 0;
}


int RefInset::linuxdoc(std::ostream & os) const
{
	os << "<ref id=\"" << asString(cell(0))
	   << "\" name=\"" << asString(cell(1)) << "\" >";
	return 0;
}


int RefInset::docbook(std::ostream & os, bool) const
{
	if (cell(1).empty()) {
		os << "<xref linkend=\"" << asString(cell(0)) << "\">";
	} else {
		os << "<link linkend=\"" << asString(cell(0))
		   << "\">" << asString(cell(1)) << "</link>";
	}

	return 0;
}

RefInset::type_info RefInset::types[] = {
	{ "ref",	N_("Standard"),			N_("Ref: ")},
	{ "pageref",	N_("Page Number"),		N_("Page: ")},
	{ "vpageref",	N_("Textual Page Number"),	N_("TextPage: ")},
	{ "vref",	N_("Standard+Textual Page"),	N_("Ref+Text: ")},
	{ "prettyref",	N_("PrettyRef"),		N_("PrettyRef: ")},
	{ "", "", "" }
};

///////////////////////////////////


MathBoxInset::MathBoxInset(string const & name)
	: MathGridInset(1, 1), name_(name)
{}


MathInset * MathBoxInset::clone() const
{
	return new MathBoxInset(*this);
}


void MathBoxInset::write(WriteStream & os) const
{
	os << "\\" << name_ << "{" << cell(0) << "}";
}


void MathBoxInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	//text_->write(buffer(), os);
	os << "] ";
}


void MathBoxInset::rebreak()
{
	//lyxerr << "trying to rebreak...\n";
}


void MathBoxInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "textnormal");
	MathGridInset::metrics(mi);
}


void MathBoxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
	MathGridInset::draw(pi, x, y);
	mathed_draw_framebox(pi, x, y, this);
}
