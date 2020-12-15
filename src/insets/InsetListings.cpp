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
#include "Cursor.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetCaption.h"
#include "InsetLabel.h"
#include "InsetLayout.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "output_latex.h"
#include "output_docbook.h"
#include "output_xhtml.h"
#include "TexRow.h"
#include "texstream.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lassert.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include <regex>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {


InsetListings::InsetListings(Buffer * buf, InsetListingsParams const & par)
	: InsetCaptionable(buf,"listing")
{
	params_.setMinted(buffer().params().use_minted);
	status_ = par.status();
}


InsetListings::~InsetListings()
{
	hideDialogs("listings", this);
}


Inset::RowFlags InsetListings::rowFlags() const
{
	return params().isInline() || params().isFloat() ? Inline : Display | AlignLeft;
}


docstring InsetListings::layoutName() const
{
	if (buffer().params().use_minted)
		return from_ascii("MintedListings");
	else
		return from_ascii("Listings");
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
	InsetCaptionable::write(os);
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
	InsetCaptionable::read(lex);
}


Encoding const * InsetListings::forcedEncoding(Encoding const * inner_enc,
					       Encoding const * outer_enc) const
{
	// The listings package cannot deal with multi-byte-encoded
	// glyphs, except for Xe/LuaTeX (with non-TeX fonts) or pLaTeX.
	// Minted can deal with all encodings.
	if (buffer().params().use_minted
		|| inner_enc->name() == "utf8-plain"
		|| inner_enc->package() == Encoding::japanese
		|| inner_enc->hasFixedWidth())
		return 0;

	// We try if there's a singlebyte encoding for the outer
	// language; if not, fall back to latin1.
	// Power-users can set inputenc to utf8-plain to bypass this workaround
	// and provide alternatives in the user-preamble.
	return (outer_enc->hasFixedWidth()) ?
			outer_enc : encodings.fromLyXName("iso8859-1");
}


void InsetListings::latex(otexstream & os, OutputParams const & runparams) const
{
	string param_string = params().params();
	// NOTE: I use {} to quote text, which is an experimental feature
	// of the listings package (see page 25 of the manual)
	bool const isInline = params().isInline();
	bool const use_minted = buffer().params().use_minted;
	static regex const reg1("(.*)(basicstyle=\\{)([^\\}]*)(\\\\ttfamily)([^\\}]*)(\\})(.*)");
	static regex const reg2("(.*)(basicstyle=\\{)([^\\}]*)(\\\\rmfamily)([^\\}]*)(\\})(.*)");
	static regex const reg3("(.*)(basicstyle=\\{)([^\\}]*)(\\\\sffamily)([^\\}]*)(\\})(.*)");
	static regex const reg4("(.*)(basicstyle=\\{)([^\\}]*)(\\\\(tiny|scriptsize|footnotesize|small|normalsize|large|Large))([^\\}]*)(\\})(.*)");
	static regex const reg5("(.*)(fontfamily=)(tt|sf|rm)(.*)");
	static regex const reg6("(.*)(fontsize=\\{)(\\\\(tiny|scriptsize|footnotesize|small|normalsize|large|Large))(\\})(.*)");
	if (use_minted) {
		// If params have been entered with "listings", and then the user switched to "minted",
		// we have params that need to be translated.
		// FIXME: We should use a backend-abstract syntax in listings params instead!
		// Substitute fontstyle option
		smatch sub;
		if (regex_match(param_string, sub, reg1))
			param_string = sub.str(1) + "fontfamily=tt," + sub.str(2) + sub.str(3)
					+ sub.str(5) + sub.str(6) + sub.str(7);
		if (regex_match(param_string, sub, reg2))
			param_string = sub.str(1) + "fontfamily=rm," + sub.str(2) + sub.str(3)
					+ sub.str(5) + sub.str(6) + sub.str(7);
		if (regex_match(param_string, sub, reg3))
			param_string = sub.str(1) + "fontfamily=sf," + sub.str(2) + sub.str(3)
					+ sub.str(5) + sub.str(6) + sub.str(7);
		// as well as fontsize option
		if (regex_match(param_string, sub, reg4))
			param_string = sub.str(1) + "fontsize={" + sub.str(4) + sub.str(3) + sub.str(7) + sub.str(8);
	} else {
		// And the same vice versa
		// Substitute fontstyle option
		smatch sub;
		string basicstyle;
		if (regex_match(param_string, sub, reg5)) {
			basicstyle = "\\" + sub.str(3) + "family";
			param_string = sub.str(1) + sub.str(4);
		}
		// as well as fontsize option
		if (regex_match(param_string, sub, reg6)) {
			basicstyle += sub.str(3);
			param_string = sub.str(1) + sub.str(6);
		}
		if (!basicstyle.empty())
			param_string = rtrim(param_string, ",") + ",basicstyle={" + basicstyle + "}";
	}
	if (runparams.use_polyglossia && runparams.local_font->isRightToLeft()) {
		// We need to use the *latin switches (#11554)
		smatch sub;
		if (regex_match(param_string, sub, reg1))
			param_string = sub.str(1) + sub.str(2) + sub.str(3) + sub.str(4)
					+ "latin"  + sub.str(5) + sub.str(6) + sub.str(7);
		if (regex_match(param_string, sub, reg2))
			param_string = sub.str(1) + sub.str(2) + sub.str(3) + sub.str(4)
					+ "latin"  + sub.str(5) + sub.str(6) + sub.str(7);
		if (regex_match(param_string, sub, reg3))
			param_string = sub.str(1) + sub.str(2) + sub.str(3) + sub.str(4)
					+ "latin"  + sub.str(5) + sub.str(6) + sub.str(7);
	}
	string minted_language;
	string float_placement;
	bool const isfloat = params().isFloat();
	if (use_minted && (isfloat || contains(param_string, "language="))) {
		// Get float placement and/or language of the code,
		// then remove the relative options.
		vector<string> opts =
			getVectorFromString(param_string, ",", false);
		for (size_t i = 0; i < opts.size(); ++i) {
			if (prefixIs(opts[i], "float")) {
				if (prefixIs(opts[i], "float="))
					float_placement = opts[i].substr(6);
				opts.erase(opts.begin() + int(i--));
			}
			else if (prefixIs(opts[i], "language=")) {
				minted_language = opts[i].substr(9);
				opts.erase(opts.begin() + int(i--));
			}
		}
		param_string = getStringFromVector(opts, ",");
	}
	// Minted needs a language specification
	if (minted_language.empty()) {
		// If a language has been set globally, use that,
		// otherwise use TeX by default
		string const & blp = buffer().params().listings_params;
		size_t start = blp.find("language=");
		if (start != string::npos) {
			start += strlen("language=");
			size_t len = blp.find(",", start);
			if (len != string::npos)
				len -= start;
			minted_language = blp.substr(start, len);
		} else
			minted_language = "TeX";
	}

	// get the paragraphs. We can not output them directly to given odocstream
	// because we can not yet determine the delimiter character of \lstinline
	docstring code;
	docstring uncodable;
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	bool encoding_switched = false;
	Encoding const * const save_enc = runparams.encoding;

	Encoding const * const outer_encoding =
		(runparams.local_font != 0) ?
			runparams.local_font->language()->encoding()
			: buffer().params().language->encoding();
	Encoding const * fixedlstenc = forcedEncoding(runparams.encoding, outer_encoding);
	if (fixedlstenc) {
		// We need to switch to a singlebyte encoding, due to
		// the restrictions of the listings package (see above).
		// This needs to be consistent with
		// LaTeXFeatures::getTClassI18nPreamble().
		// We need to put this into a group in order to prevent encoding leaks
		// (happens with cprotect).
		os << "\\bgroup";
		switchEncoding(os.os(), buffer().params(), runparams, *fixedlstenc, true);
		runparams.encoding = fixedlstenc;
		encoding_switched = true;
	}

	bool const captionfirst = !isfloat && par->isInset(0)
				&& par->getInset(0)->lyxCode() == CAPTION_CODE;

	while (par != end) {
		pos_type const siz = par->size();
		bool captionline = false;
		for (pos_type i = 0; i < siz; ++i) {
			if (i == 0 && par->isInset(i) && i + 1 == siz)
				captionline = true;
			// ignore all struck out text and (caption) insets
			if (par->isDeleted(i)
			    || (par->isInset(i) && par->getInset(i)->lyxCode() == CAPTION_CODE))
				continue;
			if (par->isInset(i)) {
				// Currently, this can only be a quote inset
				// that is output as plain quote here, but
				// we use more generic code anyway.
				otexstringstream ots;
				OutputParams rp = runparams;
				rp.pass_thru = true;
				par->getInset(i)->latex(ots, rp);
				code += ots.str();
				continue;
			}
			char_type c = par->getChar(i);
			// we can only output characters covered by the current
			// encoding!
			try {
				if (runparams.encoding->encodable(c))
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
		if (par != end && !isInline && !captionline)
			code += "\n";
	}
	if (isInline) {
		static const docstring delimiters =
				from_utf8("!*()-=+|;:'\"`,<.>/?QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm");

		size_t pos = delimiters.find_first_not_of(code);

		// This code piece contains all possible special character? !!!
		// Replace ! with a warning message and use ! as delimiter.
		if (pos == string::npos) {
			docstring delim_error = "<" + _("LyX Warning: ")
				+ _("no more lstline delimiters available") + ">";
			code = subst(code, from_ascii("!"), delim_error);
			pos = 0;
			if (!runparams.dryrun && !runparams.silent) {
				// FIXME: warning should be passed to the error dialog
				frontend::Alert::warning(_("Running out of delimiters"),
				_("For inline program listings, one character must be reserved\n"
				  "as a delimiter. One of the listings, however, uses all available\n"
				  "characters, so none is left for delimiting purposes.\n"
				  "For the time being, I have replaced '!' by a warning, but you\n"
				  "must investigate!"));
			}
		}
		docstring const delim(1, delimiters[pos]);
		if (use_minted) {
			os << "\\mintinline";
			if (!param_string.empty())
				os << "[" << from_utf8(param_string) << "]";
			os << "{" << ascii_lowercase(minted_language) << "}";
		} else {
			os << "\\lstinline";
			if (!param_string.empty())
				os << "[" << from_utf8(param_string) << "]";
			else if (pos >= delimiters.find('Q'))
				// We need to terminate the command before
				// the delimiter
				os << " ";
		}
		os << delim << code << delim;
	} else if (use_minted) {
		OutputParams rp = runparams;
		rp.moving_arg = true;
		TexString caption = getCaption(rp);
		if (isfloat) {
			os << breakln << "\\begin{listing}";
			if (!float_placement.empty())
				os << '[' << float_placement << "]";
		} else if (captionfirst && !caption.str.empty()) {
			os << breakln << "\\lyxmintcaption[t]{"
			   << move(caption) << "}\n";
		}
		os << breakln << "\\begin{minted}";
		if (!param_string.empty())
			os << "[" << param_string << "]";
		os << "{" << ascii_lowercase(minted_language) << "}\n"
		   << code << breakln << "\\end{minted}\n";
		if (isfloat) {
			if (!caption.str.empty())
				os << "\\caption{" << move(caption) << "}\n";
			os << "\\end{listing}\n";
		} else if (!captionfirst && !caption.str.empty()) {
			os << breakln << "\\lyxmintcaption[b]{"
			   << move(caption) << "}";
		}
	} else {
		OutputParams rp = runparams;
		rp.moving_arg = true;
		TexString caption = getCaption(rp);
		os << breakln << "\\begin{lstlisting}";
		if (param_string.empty() && caption.str.empty())
			os << "\n";
		else {
			if (!runparams.nice)
				os << safebreakln;
			os << "[";
			if (!caption.str.empty()) {
				os << "caption={" << move(caption) << '}';
				if (!param_string.empty())
					os << ',';
			}
			os << from_utf8(param_string) << "]\n";
		}
		os << code << breakln << "\\end{lstlisting}\n";
	}

	if (encoding_switched){
		// Switch back
		switchEncoding(os.os(), buffer().params(),
			       runparams, *save_enc, true, true);
		os << "\\egroup" << breakln;
		runparams.encoding = save_enc;
	}

	if (!uncodable.empty() && !runparams.silent) {
		// issue a warning about omitted characters
		// FIXME: should be passed to the error dialog
		if (fixedlstenc)
			frontend::Alert::warning(_("Uncodable characters in listings inset"),
				bformat(_("The following characters in one of the program listings are\n"
					  "not representable in the current encoding and have been omitted:\n%1$s.\n"
					  "This is due to a restriction of the listings package, which does\n"
					  "not support your encoding '%2$s'.\n"
					  "Toggling 'Use non-TeX fonts' in Document > Settings...\n"
					  "might help."),
				uncodable, _(runparams.encoding->guiName())));
		else
			frontend::Alert::warning(_("Uncodable characters in listings inset"),
				bformat(_("The following characters in one of the program listings are\n"
					  "not representable in the current encoding and have been omitted:\n%1$s."),
				uncodable));
	}
}


docstring InsetListings::xhtml(XMLStream & os, OutputParams const & rp) const
{
	odocstringstream ods;
	XMLStream out(ods);

	bool const isInline = params().isInline();
	if (isInline)
		out << xml::CompTag("br");
	else {
		out << xml::StartTag("div", "class='float-listings'");
		docstring caption = getCaptionHTML(rp);
		if (!caption.empty())
			out << xml::StartTag("div", "class='listings-caption'")
			    << XMLStream::ESCAPE_NONE
			    << caption << xml::EndTag("div");
	}

	InsetLayout const & il = getLayout();
	string const & tag = il.htmltag();
	string attr = "class ='listings";
	string const lang = params().getParamValue("language");
	if (!lang.empty())
		attr += " " + lang;
	attr += "'";
	out << xml::StartTag(tag, attr);
	OutputParams newrp = rp;
	newrp.html_disable_captions = true;
	// We don't want to convert dashes here. That's the only conversion we
	// do for XHTML, so this is safe.
	newrp.pass_thru = true;
	docstring def = InsetText::insetAsXHTML(out, newrp, InsetText::JustText);
	out << xml::EndTag(tag);

	if (isInline) {
		out << xml::CompTag("br");
		// escaping will already have been done
		os << XMLStream::ESCAPE_NONE << ods.str();
	} else {
		out << xml::EndTag("div");
		// In this case, this needs to be deferred, but we'll put it
		// before anything the text itself deferred.
		def = ods.str() + '\n' + def;
	}
	return def;
}


void InsetListings::docbook(XMLStream & xs, OutputParams const & rp) const
{
	InsetLayout const & il = getLayout();
	bool isInline = params().isInline();

	if (!isInline && !xs.isLastTagCR())
		xs << xml::CR();

	// In case of caption, the code must be wrapped, for instance in a figure. Also detect if there is a label.
	// http://www.sagehill.net/docbookxsl/ProgramListings.html
	// TODO: parts of this code could be merged with InsetFloat and findLabelInParagraph.
	InsetCaption const * caption = getCaptionInset();
	if (caption) {
		InsetLabel const * label = getLabelInset();

		// Ensure that the label will not be output a second time as an anchor.
		OutputParams rpNoLabel = rp;
		if (label)
			rpNoLabel.docbook_anchors_to_ignore.emplace(label->screenLabel());

		// Prepare the right set of attributes, including the label.
		docstring attr = from_ascii("type='listing'");
		if (label)
			attr += " xml:id=\"" + xml::cleanID(label->screenLabel()) + "\"";

		// Finally, generate the wrapper (including the label).
		xs << xml::StartTag("figure", attr);
		xs << xml::CR();
		xs << xml::StartTag("title");
		xs << XMLStream::ESCAPE_NONE << getCaptionDocBook(rpNoLabel);
		xs << xml::EndTag("title");
		xs << xml::CR();
	}

	// Forge the attributes.
	std::string attrs;
	if (!il.docbookattr().empty())
		attrs += " role=\"" + il.docbookattr() + "\"";
	std::string const lang = params().getParamValue("language");
	if (!lang.empty())
		attrs += " language=\"" + lang + "\"";

	// Determine the tag to use. Use the layout-defined value if outside a paragraph.
	std::string tag = il.docbooktag();
	if (isInline)
		tag = "code";

	// Start the listing.
	xs << xml::StartTag(tag, attrs);
	xs.startDivision(false);

	// Deal with the content of the listing.
	OutputParams newrp = rp;
	newrp.pass_thru = true;
	newrp.docbook_make_pars = false;
	newrp.par_begin = 0;
	newrp.par_end = text().paragraphs().size();
	newrp.docbook_in_listing = true;

	docbookParagraphs(text(), buffer(), xs, newrp);

	// Done with the listing.
	xs.endDivision();
	xs << xml::EndTag(tag);
	if (!isInline)
		xs << xml::CR();

	if (caption) {
		xs << xml::EndTag("figure");
		xs << xml::CR();
	}
}


string InsetListings::contextMenuName() const
{
	return "context-listings";
}


void InsetListings::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		cur.recordUndoInset(this);
		InsetListings::string2params(to_utf8(cmd.argument()), params());
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("listings", params2string(params()));
		break;

	default:
		InsetCaptionable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetListings::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
		case LFUN_INSET_MODIFY:
		case LFUN_INSET_DIALOG_UPDATE:
			status.setEnabled(true);
			return true;
		case LFUN_CAPTION_INSERT: {
			// the inset outputs at most one caption
			if (params().isInline() || getCaptionInset()) {
				status.setEnabled(false);
				return true;
			}
		}
		// fall through
		default:
			return InsetCaptionable::getStatus(cur, cmd, status);
	}
}


docstring const InsetListings::buttonLabel(BufferView const & bv) const
{
	// FIXME UNICODE
	docstring const locked = tempfile_ ? docstring(1, 0x1F512) : docstring();
	if (decoration() == InsetDecoration::CLASSIC)
		return locked + (isOpen(bv) ? _("Listing") : getNewLabel(_("Listing")));
	return locked + getNewLabel(_("Listing"));
}


void InsetListings::validate(LaTeXFeatures & features) const
{
	features.useInsetLayout(getLayout());
	string param_string = params().params();
	if (buffer().params().use_minted) {
		features.require("minted");
		OutputParams rp = features.runparams();
		if (!params().isFloat() && !getCaption(rp).str.empty())
			features.require("lyxmintcaption");
		if (features.usePolyglossia() && features.hasRTLLanguage())
			// minted loads color, but color must be loaded before bidi
			// (i.e., polyglossia)
			features.require("color");
	} else {
		features.require("listings");
		if (contains(param_string, "\\color"))
			features.require("color");
	}
	InsetCaptionable::validate(features);
}


bool InsetListings::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("listings", params2string(params()),
		const_cast<InsetListings *>(this));
	return true;
}


TexString InsetListings::getCaption(OutputParams const & runparams) const
{
	InsetCaption const * ins = getCaptionInset();
	if (ins == 0)
		return TexString();

	otexstringstream os;
	ins->getArgs(os, runparams);
	ins->getArgument(os, runparams);

	// TODO: The code below should be moved to support, and then the test
	//       in ../tests should be moved there as well.

	// the caption may contain \label{} but the listings
	// package prefer caption={}, label={}
	TexString cap = os.release();
	if (buffer().params().use_minted
	    || !contains(cap.str, from_ascii("\\label{")))
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
	string const new_cap("$1$3},label={$2");
	// Remove potential \protect'ion of \label.
	docstring capstr = subst(cap.str, from_ascii("\\protect\\label"),
				 from_ascii("\\label"));
	// TexString validity: the substitution preserves the number of newlines.
	// Moreover we assume that $2 does not contain newlines, so that the texrow
	// information remains accurate.
	// Replace '\n' with an improbable character from Private Use Area-A
	// and then return to '\n' after the regex replacement.
	capstr = subst(capstr, char_type('\n'), 0xffffd);
	cap.str = subst(from_utf8(regex_replace(to_utf8(capstr), reg, new_cap)),
			0xffffd, char_type('\n'));
	return cap;
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
