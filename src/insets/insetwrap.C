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
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "Floating.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "lyxlex.h"
#include "outputparams.h"
#include "paragraph.h"
#include "pariterator.h"

#include "support/convert.h"

#include <sstream>

using std::string;
using std::endl;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


namespace {

string floatname(string const & type, BufferParams const & bp)
{
	FloatList const & floats = bp.getLyXTextClass().floats();
	FloatList::const_iterator it = floats[type];
	return (it == floats.end()) ? type : _(it->second.name());
}

} // namespace anon


InsetWrap::InsetWrap(BufferParams const & bp, string const & type)
	: InsetCollapsable(bp)
{
	setLabel(_("wrap: ") + floatname(type, bp));
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	params_.type = type;
	params_.width = LyXLength(50, LyXLength::PCW);
	setInsetName(type);
}


InsetWrap::~InsetWrap()
{
	InsetWrapMailer(*this).hideDialog();
}


void InsetWrap::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetWrapParams params;
		InsetWrapMailer::string2params(cmd.argument, params);
		params_.placement = params.placement;
		params_.width     = params.width;
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetWrapMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE: {
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
			InsetWrapMailer(*this).showDialog(&cur.bv());
			break;
		}
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}

	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetWrap::getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.enabled(true);
		return true;

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
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
	string token;
	lex >> token;
	if (token == "placement")
		lex >> placement;
	else {
		// take countermeasures
		lex.pushToken(token);
	}
	if (!lex)
		return;
	lex >> token;
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


auto_ptr<InsetBase> InsetWrap::doClone() const
{
	return auto_ptr<InsetBase>(new InsetWrap(*this));
}


string const InsetWrap::editMessage() const
{
	return _("Opened Wrap Inset");
}


int InsetWrap::latex(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	os << "\\begin{floating" << params_.type << '}';
	if (!params_.placement.empty())
		os << '[' << params_.placement << ']';
	os << '{' << params_.width.asLatexString() << "}%\n";
	int const i = InsetText::latex(buf, os, runparams);
	os << "\\end{floating" << params_.type << "}%\n";
	return i + 2;
}


int InsetWrap::docbook(Buffer const & buf, ostream & os,
		       OutputParams const & runparams) const
{
	os << '<' << params_.type << '>';
	int const i = InsetText::docbook(buf, os, runparams);
	os << "</" << params_.type << '>';
	return i;
}


bool InsetWrap::insetAllowed(InsetBase::Code code) const
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


bool InsetWrap::showInsetDialog(BufferView * bv) const
{
	if (!InsetText::showInsetDialog(bv))
		InsetWrapMailer(const_cast<InsetWrap &>(*this)).showDialog(bv);
	return true;
}


void InsetWrap::addToToc(lyx::toc::TocList & toclist, Buffer const & buf) const
{
	ParConstIterator pit = par_const_iterator_begin(*this);
	ParConstIterator end = par_const_iterator_end(*this);

	// Find a caption layout in one of the (child inset's) pars
	for (; pit != end; ++pit) {
		if (pit->layout()->labeltype == LABEL_SENSITIVE) {
			string const name = floatname(params_.type, buf.params());
			string const str =
				convert<string>(toclist[name].size() + 1)
				+ ". " + pit->asString(buf, false);
			lyx::toc::TocItem const item(pit->id(), 0 , str);
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


void InsetWrapMailer::string2params(string const & in, InsetWrapParams & params)
{
	params = InsetWrapParams();
	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetWrapMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by LyXText::readInset
	string id;
	lex >> id;
	if (!lex || id != "Wrap")
		return print_mailer_error("InsetBoxMailer", in, 2, "Wrap");

	// We have to read the type here!
	lex >> params.type;
	params.read(lex);
}


string const InsetWrapMailer::params2string(InsetWrapParams const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);
	return data.str();
}
