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
#include "Encoding.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetCaption.h"
#include "InsetList.h"
#include "Language.h"
#include "MetricsInfo.h"
#include "output_latex.h"
#include "TextClass.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lassert.h"

#include "frontends/alert.h"
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
	: InsetCollapsable(buf)
{
	status_ = par.status();
}


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
	Counters & cnts = buffer().masterBuffer()->params().documentClass().counters();
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
	docstring uncodable;
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	bool encoding_switched = false;
	Encoding const * const save_enc = runparams.encoding;

	if (!runparams.encoding->hasFixedWidth()) {
		// We need to switch to a singlebyte encoding, since the listings
		// package cannot deal with multiple-byte-encoded glyphs
		Language const * const outer_language =
			(runparams.local_font != 0) ?
				runparams.local_font->language()
				: buffer().params().language;
		// We try if there's a singlebyte encoding for the current
		// language; if not, fall back to latin1.
		Encoding const * const lstenc =
			(outer_language->encoding()->hasFixedWidth()) ?
				outer_language->encoding() 
				: encodings.fromLyXName("iso8859-1");
		pair<bool, int> const c = switchEncoding(os, buffer().params(),
				runparams, *lstenc, true);
		runparams.encoding = lstenc;
		encoding_switched = true;
	}

	while (par != end) {
		pos_type siz = par->size();
		bool captionline = false;
		for (pos_type i = 0; i < siz; ++i) {
			if (i == 0 && par->isInset(i) && i + 1 == siz)
				captionline = true;
			// ignore all struck out text and (caption) insets
			if (par->isDeleted(i) || par->isInset(i))
				continue;
			char_type c = par->getChar(i);
			// we can only output characters covered by the current
			// encoding!
			try {
				if (runparams.encoding->latexChar(c) == docstring(1, c))
					code += c;
				else if (runparams.dryrun) {
					code += "<" + _("LyX Warning: ")
					   + _("uncodable character") + " '";
					code += docstring(1, c);
					code += "'>";
 				} else
					uncodable += c;
			} catch (EncodingException & /* e */) {
 				if (runparams.dryrun) {
					code += "<" + _("LyX Warning: ")
					   + _("uncodable character") + " '";
					code += docstring(1, c);
					code += "'>";
 				} else
					uncodable += c;
			}
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
			docstring delim_error = "<" + _("LyX Warning: ")
				+ _("no more lstline delimiters available") + ">";
			code = subst(code, from_ascii("!"), delim_error);
			delimiter = lstinline_delimiters;
			if (!runparams.dryrun) {
				// FIXME: warning should be passed to the error dialog
				frontend::Alert::warning(_("Running out of delimiters"),
				_("For inline program listings, one character must be reserved\n"
				  "as a delimiter. One of the listings, however, uses all available\n"
				  "characters, so none is left for delimiting purposes.\n"
				  "For the time being, I have replaced '!' by a warning, but you\n"
				  "must investigate!"));
			}
		}
		if (param_string.empty())
			os << "\\lstinline" << *delimiter;
		else
			os << "\\lstinline[" << from_utf8(param_string) << "]" << *delimiter;
                os << code
                   << *delimiter;
	} else {
		OutputParams rp = runparams;
		rp.moving_arg = true;
		docstring const caption = getCaption(rp);
		if (param_string.empty() && caption.empty())
			os << "\n\\begin{lstlisting}\n";
		else {
			os << "\n\\begin{lstlisting}[";
			if (!caption.empty()) {
				os << "caption={" << caption << '}';
				if (!param_string.empty())
					os << ',';
			}
			os << from_utf8(param_string) << "]\n";
		}
		lines += 2;
		os << code << "\n\\end{lstlisting}\n";
		lines += 2;
	}

	if (encoding_switched){
		// Switch back
		pair<bool, int> const c = switchEncoding(os, buffer().params(),
				runparams, *save_enc, true);
		runparams.encoding = save_enc;
	}

	if (!uncodable.empty()) {
		// issue a warning about omitted characters
		// FIXME: should be passed to the error dialog
		frontend::Alert::warning(_("Uncodable characters in listings inset"),
			bformat(_("The following characters in one of the program listings are\n"
				  "not representable in the current encoding and have been omitted:\n%1$s."),
			uncodable));
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
	case LFUN_TAB_INSERT:
		if (cur.selection()) {
			// If there is a selection, a tab is inserted at the
			// beginning of each paragraph.
			cur.recordUndoSelection();
			pit_type const pit_end = cur.selEnd().pit();
			for (pit_type pit = cur.selBegin().pit(); pit <= pit_end; pit++) {
				paragraphs()[pit].insertChar(0, '\t', 
					buffer().params().trackChanges);
				// Update the selection pos to make sure the selection does not
				// change as the inserted tab will increase the logical pos.
				if (cur.anchor_.pit() == pit)
					cur.anchor_.forwardPos();
				if (cur.pit() == pit)
					cur.forwardPos();
			}
			cur.finishUndo();
		} else {
			// Maybe we shouldn't allow tabs within a line, because they
			// are not (yet) aligned as one might do expect.
			cur.recordUndo();
			cur.insert(from_ascii("\t"));
			cur.finishUndo();
		}
		break;
	case LFUN_TAB_DELETE:
		if (cur.selection()) {
			// If there is a selection, a tab (if present) is removed from
			// the beginning of each paragraph.
			cur.recordUndoSelection();
			pit_type const pit_end = cur.selEnd().pit();
			for (pit_type pit = cur.selBegin().pit(); pit <= pit_end; pit++) {
				Paragraph & par = paragraphs()[pit];
				if (par.getChar(0) == '\t') {
					if (cur.pit() == pit)
						cur.posBackward();
					if (cur.anchor_.pit() == pit && cur.anchor_.pos() > 0 )
						cur.anchor_.backwardPos();

					par.eraseChar(0, buffer().params().trackChanges);
				} else 
					// If no tab was present, try to remove up to four spaces.
					for (int n_spaces = 0;
						par.getChar(0) == ' ' && n_spaces < 4; ++n_spaces) {
							if (cur.pit() == pit)
								cur.posBackward();
							if (cur.anchor_.pit() == pit && cur.anchor_.pos() > 0 )
								cur.anchor_.backwardPos();

							par.eraseChar(0, buffer().params().trackChanges);
					}
			}
			cur.finishUndo();
		} else {
			// If there is no selection, try to remove a tab or some spaces 
			// before the position of the cursor.
			Paragraph & par = paragraphs()[cur.pit()];
			pos_type const pos = cur.pos();

			if (pos == 0)
				break;

			char_type const c = par.getChar(pos - 1);
			cur.recordUndo();
			if (c == '\t') {
				cur.posBackward();
				par.eraseChar(cur.pos(), buffer().params().trackChanges);
			} else
				for (int n_spaces = 0; cur.pos() > 0
					&& par.getChar(cur.pos() - 1) == ' ' && n_spaces < 4;
					++n_spaces) {
						cur.posBackward();
						par.eraseChar(cur.pos(), buffer().params().trackChanges);
				}
				cur.finishUndo();
		}
		break;
	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetListings::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		case LFUN_INSET_MODIFY:
		case LFUN_INSET_DIALOG_UPDATE:
			status.setEnabled(true);
			return true;
		case LFUN_CAPTION_INSERT:
			status.setEnabled(!params().isInline());
			return true;
			case LFUN_TAB_INSERT:
			case LFUN_TAB_DELETE:
				status.setEnabled(true);
				return true;
		default:
			return InsetCollapsable::getStatus(cur, cmd, status);
	}
}


docstring const InsetListings::buttonLabel(BufferView const & bv) const
{
	// FIXME UNICODE
	if (decoration() == InsetLayout::CLASSIC)
		return isOpen(bv) ? _("Listing") : getNewLabel(_("Listing"));
	else
		return getNewLabel(_("Listing"));
}


void InsetListings::validate(LaTeXFeatures & features) const
{
	features.require("listings");
	string param_string = params().params();
	if (param_string.find("\\color") != string::npos)
		features.require("color");
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
