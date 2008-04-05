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

#include "Buffer.h"
#include "BufferView.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetCaption.h"
#include "InsetList.h"
#include "Language.h"
#include "MetricsInfo.h"
#include "TextClass.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "frontends/Application.h"

#include <boost/regex.hpp>

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

using boost::regex;

char const lstinline_delimiters[] =
	"!*()-=+|;:'\"`,<.>/?QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";

InsetListings::InsetListings(Buffer const & buf, InsetListingsParams const & par)
	: InsetCollapsable(buf, par.status())
{}


InsetListings::~InsetListings()
{
	hideDialogs("listings", this);
}


Inset::DisplayType InsetListings::display() const
{
	return params().isInline() || params().isFloat() ? Inline : AlignLeft;
}


void InsetListings::updateLabels(ParIterator const & it)
{
	Counters & cnts = buffer().params().documentClass().counters();
	string const saveflt = cnts.current_float();

	// Tell to captions what the current float is
	cnts.current_float("listing");

	InsetCollapsable::updateLabels(it);

	//reset afterwards
	cnts.current_float(saveflt);
}


void InsetListings::write(ostream & os) const
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
	InsetCollapsable::write(os);
}


void InsetListings::read(Lexer & lex)
{
	while (lex.isOK()) {
		lex.next();
		string token = lex.getString();
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
	InsetCollapsable::read(lex);
}


docstring InsetListings::editMessage() const
{
	return _("Opened Listing Inset");
}


int InsetListings::latex(odocstream & os, OutputParams const & runparams) const
{
	string param_string = params().params();
	// NOTE: I use {} to quote text, which is an experimental feature
	// of the listings package (see page 25 of the manual)
	int lines = 0;
	bool isInline = params().isInline();
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
		if (par != end && !isInline && !captionline) {
			code += "\n";
			++lines;
		}
	}
	if (isInline) {
		char const * delimiter = lstinline_delimiters;
		for (; delimiter != '\0'; ++delimiter)
			if (!contains(code, *delimiter))
				break;
		// This code piece contains all possible special character? !!!
		// Replace ! with a warning message and use ! as delimiter.
		if (*delimiter == '\0') {
			code = subst(code, from_ascii("!"), from_ascii(" WARNING: no lstline delimiter can be used "));
			delimiter = lstinline_delimiters;
		}
		if (param_string.empty())
			os << "\\lstinline" << *delimiter;
		else
			os << "\\lstinline[" << from_ascii(param_string) << "]" << *delimiter;
                os << code
                   << *delimiter;
	} else {
		OutputParams rp = runparams;
		// FIXME: the line below would fix bug 4182,
		// but real_current_font moved to cursor.
		//rp.local_font = &text_.real_current_font;
		rp.moving_arg = true;
		docstring const caption = getCaption(rp);
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
		os << code << "\n\\end{lstlisting}\n\\endgroup\n";
		lines += 3;
	}

	return lines;
}


docstring InsetListings::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-listings");
}


void InsetListings::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetListings::string2params(to_utf8(cmd.argument()), params());
		break;
	}
	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("listings", params2string(params()));
		break;
	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetListings::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	LYXERR0("CURSOR SIZE: " << cur.depth());
	switch (cmd.action) {
		case LFUN_INSET_DIALOG_UPDATE:
			status.enabled(true);
			return true;
		case LFUN_CAPTION_INSERT:
			status.enabled(!params().isInline());
			return true;
		default:
			return InsetCollapsable::getStatus(cur, cmd, status);
	}
}


void InsetListings::setButtonLabel()
{
	// FIXME UNICODE
	if (decoration() == InsetLayout::Classic)
		setLabel(isOpen() ?  _("Listing") : getNewLabel(_("Listing")));
	else
		setLabel(getNewLabel(_("Listing")));
}


void InsetListings::validate(LaTeXFeatures & features) const
{
	features.require("listings");
	InsetCollapsable::validate(features);
}


bool InsetListings::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("listings", params2string(params()),
		const_cast<InsetListings *>(this));
	return true;
}


docstring InsetListings::getCaption(OutputParams const & runparams) const
{
	if (paragraphs().empty())
		return docstring();

	ParagraphList::const_iterator pit = paragraphs().begin();
	for (; pit != paragraphs().end(); ++pit) {
		InsetList::const_iterator it = pit->insetList().begin();
		for (; it != pit->insetList().end(); ++it) {
			Inset & inset = *it->inset;
			if (inset.lyxCode() == CAPTION_CODE) {
				odocstringstream ods;
				InsetCaption * ins =
					static_cast<InsetCaption *>(it->inset);
				ins->getOptArg(ods, runparams);
				ins->getArgument(ods, runparams);
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


void InsetListings::string2params(string const & in,
				   InsetListingsParams & params)
{
	params = InsetListingsParams();
	if (in.empty())
		return;
	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	// discard "listings", which is only used to determine inset
	lex.next();
	params.read(lex);
}


string InsetListings::params2string(InsetListingsParams const & params)
{
	ostringstream data;
	data << "listings" << ' ';
	params.write(data);
	return data.str();
}


} // namespace lyx
