/**
 * \file InsetListings.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetListings.h"
#include "InsetCaption.h"

#include "Language.h"
#include "gettext.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "MetricsInfo.h"
#include "Cursor.h"
#include "support/lstrings.h"

#include <boost/regex.hpp>

#include <sstream>

namespace lyx {

using support::token;
using support::contains;
using support::subst;

using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;

using boost::regex;

char const lstinline_delimiters[] = "!*()-=+|;:'\"`,<.>/?QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";

void InsetListings::init()
{
	setButtonLabel();
	Font font(Font::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(Color::none);
	setLabelFont(font);
	text_.current_font.setLanguage(latex_language);
	text_.real_current_font.setLanguage(latex_language);
}


InsetListings::InsetListings(BufferParams const & bp, InsetListingsParams const & par)
	: InsetERT(bp, par.status())
{
	init();
}


InsetListings::InsetListings(InsetListings const & in)
	: InsetERT(in), params_(in.params_)
{
	init();
}


auto_ptr<Inset> InsetListings::doClone() const
{
	return auto_ptr<Inset>(new InsetListings(*this));
}


InsetListings::~InsetListings()
{
	InsetListingsMailer(*this).hideDialog();
}


Inset::DisplayType InsetListings::display() const
{
	return params().isInline() || params().isFloat() ? Inline : AlignLeft;
}


void InsetListings::write(Buffer const & buf, ostream & os) const
{
	os << "listings" << "\n";
	InsetListingsParams const & par = params();
	// parameter string is encoded to be a valid lyx token.
	string opt = par.encodedString();
	if (!opt.empty())
		os << "lstparams \"" << opt << "\"\n";
	if (par.isInline())
		os << "inline true\n";
	else
		os << "inline false\n";
	InsetCollapsable::write(buf, os);
}


void InsetListings::read(Buffer const & buf, Lexer & lex)
{
	while (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "lstparams") {
			lex.next();
			string const value = lex.getString();
			params().fromEncodedString(value);
		} else if (token == "inline") {
			lex.next();
			params().setInline(lex.getBool());
		} else {
			// no special option, push back 'status' etc
			lex.pushToken(token);
			break;
		}
	}
	InsetERT::read(buf, lex);
}


docstring const InsetListings::editMessage() const
{
	return _("Opened Listing Inset");
}


int InsetListings::latex(Buffer const & buf, odocstream & os,
		    OutputParams const & runparams) const
{
	string param_string = params().params();
	// NOTE: I use {} to quote text, which is an experimental feature
	// of the listings package (see page 25 of the manual)
	int lines = 0;
	bool lstinline = params().isInline();
	// get the paragraphs. We can not output them directly to given odocstream
	// because we can not yet determine the delimiter character of \lstinline
	docstring code;
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	while (par != end) {
		pos_type siz = par->size();
		bool captionline = false;
		for (pos_type i = 0; i < siz; ++i) {
			if (i == 0 && par->isInset(i) && i + 1 == siz)
				captionline = true;
			// ignore all struck out text and (caption) insets
			if (par->isDeleted(i) || par->isInset(i))
				continue;
			code += par->getChar(i);
		}
		++par;
		// for the inline case, if there are multiple paragraphs
		// they are simply joined. Otherwise, expect latex errors.
		if (par != end && !lstinline && !captionline) {
			code += "\n";
			++lines;
		}
	}
	// FIXME: use C++ code instead of this thing.
	char const * delimiter = 0;
	if (lstinline) {
		for (delimiter = lstinline_delimiters; delimiter != '\0'; ++delimiter)
			if (!contains(code, *delimiter))
				break;
		// this code piece contains all possible special character? !!!
		// Replace ! with a warning message and use ! as delimiter.
		if (*delimiter == '\0') {
			code = subst(code, from_ascii("!"), from_ascii(" WARNING: no lstline delimiter can be used "));
			delimiter = lstinline_delimiters;
		}
		if (param_string.empty())
			os << "\\lstinline" << *delimiter;
		else
			os << "\\lstinline[" << from_ascii(param_string) << "]" << *delimiter;
	} else {
		OutputParams rp = runparams;
		rp.local_font = &text_.real_current_font;
		rp.moving_arg = true;
		docstring const caption = getCaption(buf, rp);
		runparams.encoding = rp.encoding;
		if (param_string.empty() && caption.empty())
			os << "\n\\begingroup\n\\inputencoding{latin1}\n\\begin{lstlisting}\n";
		else {
			os << "\n\\begingroup\n\\inputencoding{latin1}\n\\begin{lstlisting}[";
			if (!caption.empty()) {
				os << "caption={" << caption << '}';
				if (!param_string.empty())
					os << ',';
			}
			os << from_utf8(param_string) << "]\n";
		}
		lines += 4;
	}
	os << code;
	if (lstinline)
		os << *delimiter;
	else {
		os << "\n\\end{lstlisting}\n\\endgroup\n";
		lines += 3;
	}

	return lines;
}


void InsetListings::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetListingsMailer::string2params(to_utf8(cmd.argument()), params());
		break;
	}
	case LFUN_INSET_DIALOG_UPDATE:
		InsetListingsMailer(*this).updateDialog(&cur.bv());
		break;
	case LFUN_MOUSE_RELEASE: {
		if (cmd.button() == mouse_button::button3 && hitButton(cmd)) {
			InsetListingsMailer(*this).showDialog(&cur.bv());
			break;
		}
		InsetERT::doDispatch(cur, cmd);
		break;
	}
	default:
		InsetERT::doDispatch(cur, cmd);
		break;
	}
}


bool InsetListings::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		case LFUN_INSET_DIALOG_UPDATE:
			status.enabled(true);
			return true;
		case LFUN_CAPTION_INSERT:
			status.enabled(!params().isInline());
			return true;
		default:
			return InsetERT::getStatus(cur, cmd, status);
	}
}


void InsetListings::setButtonLabel()
{
	// FIXME UNICODE
	setLabel(isOpen() ?  _("Listing") : getNewLabel(_("Listing")));
}


bool InsetListings::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Font tmpfont = mi.base.font;
	getDrawFont(mi.base.font);
	mi.base.font.realize(tmpfont);
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetListings::draw(PainterInfo & pi, int x, int y) const
{
	Font tmpfont = pi.base.font;
	getDrawFont(pi.base.font);
	pi.base.font.realize(tmpfont);
	InsetCollapsable::draw(pi, x, y);
	pi.base.font = tmpfont;
}


void InsetListings::validate(LaTeXFeatures & features) const
{
	features.require("listings");
	InsetERT::validate(features);
}


bool InsetListings::showInsetDialog(BufferView * bv) const
{
	InsetListingsMailer(const_cast<InsetListings &>(*this)).showDialog(bv);
	return true;
}


void InsetListings::getDrawFont(Font & font) const
{
	font = Font(Font::ALL_INHERIT, latex_language);
	font.setFamily(Font::TYPEWRITER_FAMILY);
	// FIXME: define Color::listing?
	font.setColor(Color::foreground);
}


docstring InsetListings::getCaption(Buffer const & buf,
		    OutputParams const & runparams) const
{
	if (paragraphs().empty())
		return docstring();

	ParagraphList::const_iterator pit = paragraphs().begin();
	for (; pit != paragraphs().end(); ++pit) {
		InsetList::const_iterator it = pit->insetlist.begin();
		for (; it != pit->insetlist.end(); ++it) {
			Inset & inset = *it->inset;
			if (inset.lyxCode() == Inset::CAPTION_CODE) {
				odocstringstream ods;
				InsetCaption * ins =
					static_cast<InsetCaption *>(it->inset);
				ins->getOptArg(buf, ods, runparams);
				ins->getArgument(buf, ods, runparams);
				// the caption may contain \label{} but the listings
				// package prefer caption={}, label={}
				docstring cap = ods.str();
				if (!contains(to_utf8(cap), "\\label{"))
					return cap;
				// convert from
				//     blah1\label{blah2} blah3
				// to
				//     blah1 blah3},label={blah2
				// to form options
				//     caption={blah1 blah3},label={blah2}
				//
				// NOTE that } is not allowed in blah2.
				regex const reg("(.*)\\\\label\\{(.*?)\\}(.*)");
				string const new_cap("\\1\\3},label={\\2");
				return from_utf8(regex_replace(to_utf8(cap), reg, new_cap));
			}
		}
	}
	return docstring();
}


string const InsetListingsMailer::name_("listings");

InsetListingsMailer::InsetListingsMailer(InsetListings & inset)
	: inset_(inset)
{}


string const InsetListingsMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetListingsMailer::string2params(string const & in,
				   InsetListingsParams & params)
{
	params = InsetListingsParams();
	if (in.empty())
		return;
	istringstream data(in);
	Lexer lex(0, 0);
	lex.setStream(data);
	// discard "listings", which is only used to determine inset
	lex.next();
	params.read(lex);
}


string const
InsetListingsMailer::params2string(InsetListingsParams const & params)
{
	ostringstream data;
	data << name_ << " ";
	params.write(data);
	return data.str();
}


} // namespace lyx
