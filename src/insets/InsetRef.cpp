/**
 * \file InsetRef.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "InsetRef.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "FuncStatus.h"
#include "InsetLabel.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "LyX.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "xml.h"
#include "texstream.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

using namespace lyx::support;
using namespace std;

namespace lyx {


InsetRef::InsetRef(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p), broken_(false), active_(true)
{}


InsetRef::InsetRef(InsetRef const & ir)
	: InsetCommand(ir), broken_(false), active_(true)
{}


bool InsetRef::isCompatibleCommand(string const & s) {
	//FIXME This is likely not the best way to handle this.
	//But this stuff is hardcoded elsewhere already.
	return s == "ref"
		|| s == "pageref"
		|| s == "vref"
		|| s == "vpageref"
		|| s == "formatted"
		|| s == "prettyref" // for InsetMathRef FIXME
		|| s == "eqref"
		|| s == "nameref"
		|| s == "labelonly";
}


ParamInfo const & InsetRef::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("name", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("reference", ParamInfo::LATEX_REQUIRED,
				ParamInfo::HANDLING_ESCAPE);
		param_info_.add("plural", ParamInfo::LYX_INTERNAL);
		param_info_.add("caps", ParamInfo::LYX_INTERNAL);
		param_info_.add("noprefix", ParamInfo::LYX_INTERNAL);
	}
	return param_info_;
}


docstring InsetRef::layoutName() const
{
	return from_ascii("Ref");
}


void InsetRef::changeTarget(docstring const & new_label)
{
	// With change tracking, we insert a new ref
	// and delete the old one
	if (buffer().masterParams().track_changes) {
		InsetCommandParams icp(REF_CODE, "ref");
		icp["reference"] = new_label;
		string const data = InsetCommand::params2string(icp);
		lyx::dispatch(FuncRequest(LFUN_INSET_INSERT, data));
		lyx::dispatch(FuncRequest(LFUN_CHAR_DELETE_FORWARD));
	} else
		setParam("reference", new_label);
}



void InsetRef::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	string const inset = cmd.getArg(0);
	string const arg   = cmd.getArg(1);
	string pstring;
	if (cmd.action() == LFUN_INSET_MODIFY && inset == "ref") {
		if (arg == "toggle-plural")
			pstring = "plural";
		else if (arg == "toggle-caps")
			pstring = "caps";
		else if (arg == "toggle-noprefix")
			pstring = "noprefix";
		else if (arg == "changetarget") {
			string const oldtarget = cmd.getArg(2);
			string const newtarget = cmd.getArg(3);
			if (!oldtarget.empty() && !newtarget.empty()
			    && getParam("reference") == from_utf8(oldtarget))
				changeTarget(from_utf8(newtarget));
			cur.forceBufferUpdate();
			return;
		}
	}
	// otherwise not for us
	if (pstring.empty())
		return InsetCommand::doDispatch(cur, cmd);

	bool const isSet = (getParam(pstring) == "true");
	setParam(pstring, from_ascii(isSet ? "false"  : "true"));
}


bool InsetRef::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	if (cmd.action() != LFUN_INSET_MODIFY)
		return InsetCommand::getStatus(cur, cmd, status);
	if (cmd.getArg(0) != "ref")
		return InsetCommand::getStatus(cur, cmd, status);

	string const arg = cmd.getArg(1);
	string pstring;
	if (arg == "changetarget")
		return true;
	if (arg == "toggle-plural")
		pstring = "plural";
	else if (arg == "toggle-caps")
		pstring = "caps";
	if (!pstring.empty()) {
		status.setEnabled(buffer().params().use_refstyle &&
			params().getCmdName() == "formatted");
		bool const isSet = (getParam(pstring) == "true");
		status.setOnOff(isSet);
		return true;
	}
	if (arg == "toggle-noprefix") {
		status.setEnabled(params().getCmdName() == "labelonly");
		bool const isSet = (getParam("noprefix") == "true");
		status.setOnOff(isSet);
		return true;
	}
	// otherwise not for us
	return InsetCommand::getStatus(cur, cmd, status);
}


namespace {

void capitalize(docstring & s) {
	char_type t = uppercase(s[0]);
	s[0] = t;
}

} // namespace


// the ref argument is the label name we are referencing.
// we expect ref to be in the form: pfx:suffix.
//
// if it isn't, then we can't produce a formatted reference,
// so we return "\ref" and put ref into label.
//
// for refstyle, we return "\pfxcmd", and put suffix into
// label and pfx into prefix. this is because refstyle expects
// the command: \pfxcmd{suffix}.
//
// for prettyref, we return "\prettyref" and put ref into label
// and pfx into prefix. this is because prettyref uses the whole
// label, thus: \prettyref{pfx:suffix}.
//
docstring InsetRef::getFormattedCmd(docstring const & ref,
	docstring & label, docstring & prefix, docstring const & caps) const
{
	static docstring const defcmd = from_ascii("\\ref");
	static docstring const prtcmd = from_ascii("\\prettyref");

	label = split(ref, prefix, ':');

	// we have to have xxx:xxxxx...
	if (label.empty()) {
		LYXERR0("Label `" << ref << "' contains no `:' separator.");
		label = ref;
		prefix = from_ascii("");
		return defcmd;
	}

	if (prefix.empty()) {
		// we have ":xxxx"
		label = ref;
		return defcmd;
	}

	if (!buffer().params().use_refstyle) {
		// \prettyref uses the whole label
		label = ref;
		return prtcmd;
	}

	// make sure the prefix is legal for a latex command
	int const len = prefix.size();
	for (int i = 0; i < len; i++) {
		char_type const c = prefix[i];
		if (!isAlphaASCII(c)) {
			LYXERR0("Prefix `" << prefix << "' is invalid for LaTeX.");
			// restore the label
			label = ref;
			return defcmd;
		}
	}
	if (caps == "true") {
		capitalize(prefix);
	}
	return from_ascii("\\") + prefix + from_ascii("ref");
}


docstring InsetRef::getEscapedLabel(OutputParams const & rp) const
{
	InsetCommandParams const & p = params();
	ParamInfo const & pi = p.info();
	ParamInfo::ParamData const & pd = pi["reference"];
	return p.prepareCommand(rp, getParam("reference"), pd.handling());
}


void InsetRef::latex(otexstream & os, OutputParams const & rp) const
{
	string const & cmd = getCmdName();
	docstring const & data = getEscapedLabel(rp);

	if (rp.inulemcmd > 0)
		os << "\\mbox{";

	if (cmd == "eqref" && buffer().params().use_refstyle) {
		// we advertise this as printing "(n)", so we'll do that, at least
		// for refstyle, since refstlye's own \eqref prints, by default,
		// "equation n". if one wants \eqref, one can get it by using a
		// formatted label in this case.
		os << '(' << from_ascii("\\ref{") << data << from_ascii("})");
	}
	else if (cmd == "formatted") {
		docstring label;
		docstring prefix;
		docstring const fcmd =
			getFormattedCmd(data, label, prefix, getParam("caps"));
		os << fcmd;
		if (buffer().params().use_refstyle && getParam("plural") == "true")
		    os << "[s]";
		os << '{' << label << '}';
	}
	else if (cmd == "labelonly") {
		docstring const & ref = getParam("reference");
		if (getParam("noprefix") != "true")
			os << ref;
		else {
			docstring prefix;
			docstring suffix = split(ref, prefix, ':');
			if (suffix.empty()) {
		    LYXERR0("Label `" << ref << "' contains no `:' separator.");
				os << ref;
			} else {
				os << suffix;
			}
		}
	}
	else {
		// We don't want to output p_["name"], since that is only used
		// in docbook. So we construct new params, without it, and use that.
		InsetCommandParams p(REF_CODE, cmd);
		docstring const ref = getParam("reference");
		p["reference"] = ref;
		os << p.getCommand(rp);
	}

	if (rp.inulemcmd > 0)
		os << "}";
}


int InsetRef::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	docstring const str = getParam("reference");
	os << '[' << str << ']';
	return 2 + str.size();
}


void InsetRef::docbook(XMLStream & xs, OutputParams const &) const
{
	docstring const & ref = getParam("reference");
	InsetLabel const * il = buffer().insetLabel(ref, true);
	string const & cmd = params().getCmdName();
	docstring linkend = xml::cleanID(ref);

	// A name is provided, LyX will provide it. This is supposed to be a very rare case.
	// Link with linkend, as is it within the document (not outside, in which case xlink:href is better suited).
	docstring const & name = getParam("name");
	if (!name.empty()) {
		docstring attr = from_utf8("linkend=\"") + linkend + from_utf8("\"");

		xs << xml::StartTag("link", to_utf8(attr));
		xs << name;
		xs << xml::EndTag("link");
		return;
	}

	// The DocBook processor will generate the name when required.
	docstring display_before;
	docstring display_after;
	docstring role;

	if (il && !il->counterValue().empty()) {
		// Try to construct a label from the InsetLabel we reference.
		if (cmd == "vref" || cmd == "pageref" || cmd == "vpageref" || cmd == "nameref" || cmd == "formatted") {
			// "ref on page #", "on page #", etc. The DocBook processor deals with generating the right text,
			// including in the right language.
			role = from_ascii(cmd);

			if (cmd == "formatted") {
				// A formatted reference may have many parameters. Generate all of them as roles, the only
				// way arbitrary parameters can be passed into DocBook.
				if (buffer().params().use_refstyle && getParam("caps") == "true")
					role += " refstyle-caps";
				if (buffer().params().use_refstyle && getParam("plural") == "true")
					role += " refstyle-plural";
			}
		} else if (cmd == "eqref") {
			display_before = from_ascii("(");
			display_after = from_ascii(")");
		}
		// TODO: what about labelonly? I don't get how this is supposed to work...
	}

	// No name, ask DocBook to generate one.
	docstring attr = from_utf8("linkend=\"") + ref + from_utf8("\"");
	if (!role.empty())
		attr += " role=\"" + role + "\"";
	xs << display_before;
	xs << xml::CompTag("xref", to_utf8(attr));
	xs << display_after;
}


docstring InsetRef::xhtml(XMLStream & xs, OutputParams const & op) const
{
	docstring const & ref = getParam("reference");
	InsetLabel const * il = buffer().insetLabel(ref, true);
	string const & cmd = params().getCmdName();
	docstring display_string;

	if (il && !il->counterValue().empty()) {
		// Try to construct a label from the InsetLabel we reference.
		docstring const & value = il->counterValue();
		if (cmd == "ref")
			display_string = value;
		else if (cmd == "vref")
			// normally, would be "ref on page #", but we have no pages
			display_string = value;
		else if (cmd == "pageref" || cmd == "vpageref")
			// normally would be "on page #", but we have no pages.
			display_string = translateIfPossible(from_ascii("elsewhere"),
			        op.local_font->language()->lang());
		else if (cmd == "eqref")
			display_string = '(' + value + ')';
		else if (cmd == "formatted") {
			display_string = il->prettyCounter();
			if (buffer().params().use_refstyle && getParam("caps") == "true")
				capitalize(display_string);
			// it is hard to see what to do about plurals...
		}
		else if (cmd == "nameref")
			// FIXME We don't really have the ability to handle these
			// properly in XHTML output yet (bug #8599).
			// It might not be that hard to do. We have the InsetLabel,
			// and we can presumably find its paragraph using the TOC.
			// But the label might be referencing a section, yet not be
			// in that section. So this is not trivial.
			display_string = il->prettyCounter();
	} else
			display_string = ref;

	// FIXME What we'd really like to do is to be able to output some
	// appropriate sort of text here. But to do that, we need to associate
	// some sort of counter with the label, and we don't have that yet.
	docstring const attr = "href=\"#" + xml::cleanAttr(ref) + '"';
	xs << xml::StartTag("a", to_utf8(attr));
	xs << display_string;
	xs << xml::EndTag("a");
	return docstring();
}


void InsetRef::toString(odocstream & os) const
{
	odocstringstream ods;
	plaintext(ods, OutputParams(0));
	os << ods.str();
}


void InsetRef::forOutliner(docstring & os, size_t const, bool const) const
{
	// There's no need for details in the TOC, and a long label
	// will just get in the way.
	os += '#';
}


void InsetRef::updateBuffer(ParIterator const & it, UpdateType, bool const /*deleted*/)
{
	docstring const & ref = getParam("reference");

	// Check if this one is active (i.e., neither deleted with change-tracking
	// nor in an inset that does not produce output, such as notes or inactive branches)
	Paragraph const & para = it.paragraph();
	active_ = !para.isDeleted(it.pos()) && para.inInset().producesOutput();
	// If not, check whether we are in a deleted/non-outputting inset
	if (active_) {
		for (size_type sl = 0 ; sl < it.depth() ; ++sl) {
			Paragraph const & outer_par = it[sl].paragraph();
			if (outer_par.isDeleted(it[sl].pos())
			    || !outer_par.inInset().producesOutput()) {
				active_ = false;
				break;
			}
		}
	}

	// register this inset into the buffer reference cache.
	buffer().addReference(ref, this, it);

	docstring label;
	string const & cmd = getCmdName();
	for (int i = 0; !types[i].latex_name.empty(); ++i) {
		if (cmd == types[i].latex_name) {
			label = _(types[i].short_gui_name);
			break;
		}
	}

	if (cmd != "labelonly")
		label += ref;
	else {
		if (getParam("noprefix") != "true")
			label += ref;
		else {
			docstring prefix;
			docstring suffix = split(ref, prefix, ':');
			if (suffix.empty()) {
				label += ref;
			} else {
				label += suffix;
			}
		}
	}

	if (!buffer().params().isLatex() && !getParam("name").empty()) {
		label += "||";
		label += getParam("name");
	}

	unsigned int const maxLabelChars = 24;
	if (label.size() > maxLabelChars) {
		tooltip_ = label;
		support::truncateWithEllipsis(label, maxLabelChars);
	} else
		tooltip_ = from_ascii("");

	screen_label_ = label;
	broken_ = false;
	setBroken(broken_);
}


docstring InsetRef::screenLabel() const
{
	return (broken_ ? _("BROKEN: ") : docstring()) + screen_label_;
}


void InsetRef::addToToc(DocIterator const & cpit, bool output_active,
			UpdateType, TocBackend & backend) const
{
	active_ = output_active;
	docstring const & label = getParam("reference");
	if (buffer().insetLabel(label)) {
		broken_ = !buffer().activeLabel(label);
		setBroken(broken_);
		if (broken_ && output_active) {
			shared_ptr<Toc> toc2 = backend.toc("brokenrefs");
			toc2->push_back(TocItem(cpit, 0, screenLabel(), output_active));
		}
		// This InsetRef has already been taken care of in InsetLabel::addToToc().
		return;
	}

	// It seems that this reference does not point to any valid label.
	broken_ = true;
	setBroken(broken_);
	shared_ptr<Toc> toc = backend.toc("label");
	toc->push_back(TocItem(cpit, 0, screenLabel(), output_active));
	shared_ptr<Toc> toc2 = backend.toc("brokenrefs");
	toc2->push_back(TocItem(cpit, 0, screenLabel(), output_active));
}


void InsetRef::validate(LaTeXFeatures & features) const
{
	string const cmd = getCmdName();
	if (cmd == "vref" || cmd == "vpageref")
		features.require("varioref");
	else if (cmd == "formatted") {
		docstring const data = getEscapedLabel(features.runparams());
		docstring label;
		docstring prefix;
		docstring const fcmd =
			getFormattedCmd(data, label, prefix, getParam("caps"));
		if (buffer().params().use_refstyle) {
			features.require("refstyle");
			if (prefix == "cha")
				features.addPreambleSnippet(from_ascii("\\let\\charef=\\chapref"));
			else if (!prefix.empty()) {
				docstring lcmd = "\\AtBeginDocument{\\providecommand" +
						fcmd + "[1]{\\ref{" + prefix + ":#1}}}";
				features.addPreambleSnippet(lcmd);
			}
		} else {
			features.require("prettyref");
			// prettyref uses "cha" for chapters, so we provide a kind of
			// translation.
			if (prefix == "chap")
				features.addPreambleSnippet(from_ascii("\\let\\pr@chap=\\pr@cha"));
		}
	} else if (cmd == "eqref" && !buffer().params().use_refstyle)
		// with refstyle, we simply output "(\ref{label})"
		features.require("amsmath");
	else if (cmd == "nameref")
		features.require("nameref");
}

bool InsetRef::forceLTR(OutputParams const & rp) const
{
	// We force LTR for references. However,
	// * Namerefs are output in the scripts direction
	//   at least with fontspec/bidi and luabidi, though (see #11518).
	// * Parentheses are automatically swapped with XeTeX/bidi 
	//   [not with LuaTeX/luabidi] (see #11626).
	// FIXME: Re-Audit all other RTL cases.
	if (rp.useBidiPackage())
		return false;
	return (getCmdName() != "nameref" || !buffer().masterParams().useNonTeXFonts);
}


InsetRef::type_info const InsetRef::types[] = {
	{ "ref",       N_("Standard"),              N_("Ref: ")},
	{ "eqref",     N_("Equation"),              N_("EqRef: ")},
	{ "pageref",   N_("Page Number"),           N_("Page: ")},
	{ "vpageref",  N_("Textual Page Number"),   N_("TextPage: ")},
	{ "vref",      N_("Standard+Textual Page"), N_("Ref+Text: ")},
	{ "nameref",   N_("Reference to Name"),     N_("NameRef: ")},
	{ "formatted", N_("Formatted"),             N_("Format: ")},
	{ "labelonly", N_("Label Only"),            N_("Label: ")},
	{ "", "", "" }
};


docstring InsetRef::getTOCString() const
{
	docstring const & label = getParam("reference");
	if (buffer().insetLabel(label))
		broken_ = !buffer().activeLabel(label) && active_;
	else 
		broken_ = active_;
	return tooltip_.empty() ? screenLabel() : tooltip_;
}

} // namespace lyx
