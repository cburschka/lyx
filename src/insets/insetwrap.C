/**
 * \file insetwrap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetwrap.h"

#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "Floating.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "lyxlex.h"
#include "paragraph.h"

#include "support/tostr.h"

#include "support/std_sstream.h"

using std::endl;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


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
	: InsetCollapsable(bp)
{
	string lab(_("wrap: "));
	lab += floatname(type, bp);
	setLabel(lab);
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	params_.type = type;
	params_.width = LyXLength(50, LyXLength::PCW);
	setInsetName(type);
	LyXTextClass const & tclass = bp.getLyXTextClass();
	if (tclass.hasLayout(caplayout))
		inset.paragraphs.begin()->layout(tclass[caplayout]);
}


InsetWrap::~InsetWrap()
{
	InsetWrapMailer(*this).hideDialog();
}


dispatch_result InsetWrap::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetWrapParams params;
		InsetWrapMailer::string2params(cmd.argument, params);

		params_.placement = params.placement;
		params_.width     = params.width;

		cmd.view()->updateInset(this);
		return DISPATCHED;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetWrapMailer(*this).updateDialog(cmd.view());
		return DISPATCHED;

	default:
		return InsetCollapsable::localDispatch(cmd);
	}
}


void InsetWrapParams::write(ostream & os) const
{
	os << "Wrap " << type << '\n';

	if (!placement.empty())
		os << "placement " << placement << "\n";

	os << "width \"" << width.asString() << "\"\n";
}


void InsetWrapParams::read(LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string token = lex.getString();
		if (token == "placement") {
			lex.next();
			placement = lex.getString();
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
			width = LyXLength(lex.getString());
		} else {
			lyxerr << "InsetWrap::Read:: Missing 'width'-tag!"
			       << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
}


void InsetWrap::write(Buffer const & buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetWrap::read(Buffer const & buf, LyXLex & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
}


void InsetWrap::validate(LaTeXFeatures & features) const
{
	features.require("floatflt");
	InsetCollapsable::validate(features);
}


auto_ptr<InsetBase> InsetWrap::clone() const
{
	return auto_ptr<InsetBase>(new InsetWrap(*this));
}


string const InsetWrap::editMessage() const
{
	return _("Opened Wrap Inset");
}


int InsetWrap::latex(Buffer const & buf, ostream & os,
		     LatexRunParams const & runparams) const
{
	os << "\\begin{floating" << params_.type << '}';
	if (!params_.placement.empty()) {
		os << '[' << params_.placement << ']';
	}
	os  << '{' << params_.width.asLatexString() << "}%\n";

	int const i = inset.latex(buf, os, runparams);

	os << "\\end{floating" << params_.type << "}%\n";
	return i + 2;
}


int InsetWrap::docbook(Buffer const & buf, ostream & os, bool mixcont) const
{
	os << '<' << params_.type << '>';
	int const i = inset.docbook(buf, os, mixcont);
	os << "</" << params_.type << '>';

	return i;
}


bool InsetWrap::insetAllowed(InsetOld::Code code) const
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


int InsetWrap::latexTextWidth(BufferView * bv) const
{
	return params_.width.inPixels(InsetCollapsable::latexTextWidth(bv));
}


bool InsetWrap::showInsetDialog(BufferView * bv) const
{
	if (!inset.showInsetDialog(bv)) {
		InsetWrap * tmp = const_cast<InsetWrap *>(this);
		InsetWrapMailer(*tmp).showDialog(bv);
	}
	return true;
}


void InsetWrap::addToToc(lyx::toc::TocList & toclist, Buffer const & buf) const
{
	// Now find the caption in the float...
	ParagraphList::iterator tmp = inset.paragraphs.begin();
	ParagraphList::iterator end = inset.paragraphs.end();

	for (; tmp != end; ++tmp) {
		if (tmp->layout()->name() == caplayout) {
			string const name = floatname(params_.type, buf.params());
			string const str =
				tostr(toclist[name].size() + 1)
				+ ". " + tmp->asString(buf, false);
			lyx::toc::TocItem const item(tmp->id(), 0 , str);
			toclist[name].push_back(item);
		}
	}
}


string const InsetWrapMailer::name_("wrap");

InsetWrapMailer::InsetWrapMailer(InsetWrap & inset)
	: inset_(inset)
{}


string const InsetWrapMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetWrapMailer::string2params(string const & in,
				    InsetWrapParams & params)
{
	params = InsetWrapParams();

	if (in.empty())
		return;

	istringstream data(STRCONV(in));
	LyXLex lex(0,0);
	lex.setStream(data);

	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != name_)
			return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "Wrap" || !lex.eatLine())
			return;
	}

	if (lex.isOK()) {
		params.read(lex);
	}
}


string const InsetWrapMailer::params2string(InsetWrapParams const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);
	return STRCONV(data.str());
}
