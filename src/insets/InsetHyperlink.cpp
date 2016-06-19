/**
 * \file InsetHyperlink.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetHyperlink.h"

#include "Buffer.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "texstream.h"

#include "support/docstream.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "frontends/alert.h"

using namespace std;
using namespace lyx::support;

namespace lyx {


InsetHyperlink::InsetHyperlink(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{}


ParamInfo const & InsetHyperlink::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("name", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("target", ParamInfo::LATEX_REQUIRED);
		param_info_.add("type", ParamInfo::LATEX_REQUIRED);
	}
	return param_info_;
}


docstring InsetHyperlink::screenLabel() const
{
	docstring const temp = _("Hyperlink: ");

	docstring url;

	url += getParam("name");
	if (url.empty())
		url += getParam("target");

	// elide if long
	if (url.length() > 30) {
		docstring end = url.substr(url.length() - 17, url.length());
		support::truncateWithEllipsis(url, 13);
		url += end;
	}
	return temp + url;
}

void InsetHyperlink::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_EDIT:
		viewTarget();
		break;

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


bool InsetHyperlink::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_INSET_EDIT:
		flag.setEnabled(getParam("type").empty() || getParam("type") == "file:");
		return true;

	default:
		return InsetCommand::getStatus(cur, cmd, flag);
	}
}


void InsetHyperlink::viewTarget() const
{
	if (getParam("type") == "file:") {
		FileName url = makeAbsPath(to_utf8(getParam("target")), buffer().filePath());
		string const format = formats.getFormatFromFile(url);
		formats.view(buffer(), url, format);
	}
}


void InsetHyperlink::latex(otexstream & os,
			   OutputParams const & runparams) const
{
	docstring url = getParam("target");
	docstring name = getParam("name");
	static docstring const backslash = from_ascii("\\");
	static docstring const braces = from_ascii("{}");
	static char_type const chars_url[2] = {'%', '#'};
	static char_type const chars_name[6] = {
		'&', '_', '$', '%', '#', '^'};

	// For the case there is no name given, the target is set as name.
	// Do this before !url.empty() and !name.empty() to handle characters
	// like the "%" correctly.
	if (name.empty())
		name = url;

	if (!url.empty()) {
		// Replace the "\" character by its ASCII code according to the
		// URL specifications because "\" is not allowed in URLs and by
		// \href. Only do this when the following character is not also
		// a "\", because "\\" is valid code
		for (size_t i = 0, pos;
			(pos = url.find('\\', i)) != string::npos;
			i = pos + 2) {
			if (url[pos + 1] != '\\')
				url.replace(pos, 1, from_ascii("%5C"));
		}

		// The characters in chars_url[] need to be escaped in the url
		// field because otherwise LaTeX will fail when the hyperlink is
		// within an argument of another command, e.g. in a \footnote. It
		// is important that they are escaped as "\#" and not as "\#{}".
		for (int k = 0; k < 2; k++)
			for (size_t i = 0, pos;
				(pos = url.find(chars_url[k], i)) != string::npos;
				i = pos + 2)
				url.replace(pos, 1, backslash + chars_url[k]);
		
		// add "http://" when the type is web (type = empty)
		// and no "://" or "run:" is given
		docstring type = getParam("type");
		if (url.find(from_ascii("://")) == string::npos
			&& url.find(from_ascii("run:")) == string::npos
			&& type.empty())
			url = from_ascii("http://") + url;

	} // end if (!url.empty())

	// The characters in chars_name[] need to be changed to a command when
	// they are in the name field.
	if (!name.empty()) {
		// handle the "\" character, but only when the following character
		// is not also a "\", because "\\" is valid code
		docstring const textbackslash = from_ascii("\\textbackslash{}");
		for (size_t i = 0, pos;
			(pos = name.find('\\', i)) != string::npos;
			i = pos + 2) {
			if (name[pos + 1] != '\\')
				name.replace(pos, 1, textbackslash);
		}
		// The characters in chars_name[] need to be changed to a command
		// when they are in the name field.
		// Therefore the treatment of "\" must be the first thing
		for (int k = 0; k < 6; k++)
			for (size_t i = 0, pos;
				(pos = name.find(chars_name[k], i)) != string::npos;
				i = pos + 2)
				name.replace(pos, 1, backslash + chars_name[k] + braces);

		// replace the tilde by the \sim character as suggested in the
		// LaTeX FAQ for URLs
		docstring const sim = from_ascii("$\\sim$");
		for (size_t i = 0, pos;
			(pos = name.find('~', i)) != string::npos;
			i = pos + 1)
			name.replace(pos, 1, sim);
		pair<docstring, docstring> name_latexed =
			runparams.encoding->latexString(name, runparams.dryrun);
		name = name_latexed.first;
		if (!name_latexed.second.empty() && !runparams.silent) {
			// issue a warning about omitted characters
			// FIXME: should be passed to the error dialog
			frontend::Alert::warning(_("Uncodable characters"),
				bformat(_("The following characters that are used in the href inset are not\n"
					  "representable in the current encoding and therefore have been omitted:\n%1$s."),
					name_latexed.second));
		}
	}  // end if (!name.empty())
	
	if (runparams.moving_arg)
		os << "\\protect";

	// output the ready \href command
	os << "\\href{" << getParam("type") << url << "}{" << name << '}';
}


int InsetHyperlink::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	odocstringstream oss;

	oss << '[' << getParam("target");
	if (getParam("name").empty())
		oss << ']';
	else
		oss << "||" << getParam("name") << ']';

	docstring const str = oss.str();
	os << str;
	return str.size();
}


int InsetHyperlink::docbook(odocstream & os, OutputParams const &) const
{
	os << "<ulink url=\""
	   << subst(getParam("target"), from_ascii("&"), from_ascii("&amp;"))
	   << "\">"
	   << getParam("name")
	   << "</ulink>";
	return 0;
}


docstring InsetHyperlink::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	docstring const & target = 
		html::htmlize(getParam("target"), XHTMLStream::ESCAPE_AND);
	docstring const & name   = getParam("name");
	xs << html::StartTag("a", to_utf8("href=\"" + target + "\""));
	xs << (name.empty() ? target : name);
	xs << html::EndTag("a");
	return docstring();
}


void InsetHyperlink::toString(odocstream & os) const
{
	odocstringstream ods;
	plaintext(ods, OutputParams(0), INT_MAX);
	os << ods.str();
}


void InsetHyperlink::forOutliner(docstring & os, size_t const, bool const) const
{
	docstring const & n = getParam("name");
	if (!n.empty()) {
		os += n;
		return;
	}
	os += getParam("target");
}


docstring InsetHyperlink::toolTip(BufferView const & /*bv*/, int /*x*/, int /*y*/) const
{
	docstring url = getParam("target");
	docstring type = getParam("type");
	docstring guitype = _("www");
	if (type == "mailto:")
		guitype = _("email");
	else if (type == "file:")
		guitype = _("file");
	return bformat(_("Hyperlink (%1$s) to %2$s"), guitype, url);
}


void InsetHyperlink::validate(LaTeXFeatures & features) const
{
	features.require("hyperref");
}


string InsetHyperlink::contextMenuName() const
{
	return "context-hyperlink";
}


} // namespace lyx
