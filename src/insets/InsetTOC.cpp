/**
 * \file InsetTOC.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetTOC.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Font.h"
#include "FuncRequest.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"

#include <ostream>

using namespace std;

namespace lyx {

namespace {
string cmd2type(string const & cmd)
{
	if (cmd == "lstlistoflistings")
		return "listing";
	return cmd;
}
}


InsetTOC::InsetTOC(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{}


ParamInfo const & InsetTOC::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("type", ParamInfo::LATEX_REQUIRED);
	}
	return param_info_;
}


bool InsetTOC::isCompatibleCommand(string const & cmd)
{
	return cmd == defaultCommand() || cmd == "lstlistoflistings";
}


docstring InsetTOC::screenLabel() const
{
	if (getCmdName() == "tableofcontents")
		return buffer().B_("Table of Contents");
	if (getCmdName() == "lstlistoflistings")
		return buffer().B_("List of Listings");
	return _("Unknown TOC type");
}


void InsetTOC::doDispatch(Cursor & cur, FuncRequest & cmd) {
	switch (cmd.action()) {
	case LFUN_MOUSE_RELEASE:
		if (!cur.selection() && cmd.button() == mouse_button::button1) {
			cur.bv().showDialog("toc", params2string(params()));
			cur.dispatched();
		}
		break;
	
	default:
		InsetCommand::doDispatch(cur, cmd);
	}
}


docstring InsetTOC::layoutName() const
{
	if (getCmdName() == "lstlistoflistings")
		return from_ascii("TOC:Listings");
	return from_ascii("TOC");
}


void InsetTOC::validate(LaTeXFeatures & features) const
{
	InsetCommand::validate(features);
	features.useInsetLayout(getLayout());
	if (getCmdName() == "lstlistoflistings")
		features.require("listings");
}


int InsetTOC::plaintext(odocstringstream & os,
        OutputParams const &, size_t max_length) const
{
	os << screenLabel() << "\n\n";
	buffer().tocBackend().writePlaintextTocList(cmd2type(getCmdName()), os, max_length);
	return PLAINTEXT_NEWLINE;
}


int InsetTOC::docbook(odocstream & os, OutputParams const &) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc></toc>";
	return 0;
}


void InsetTOC::makeTOCEntry(XHTMLStream & xs, 
		Paragraph const & par, OutputParams const & op) const
{
	string const attr = "href='#" + par.magicLabel() + "' class='tocentry'";
	xs << html::StartTag("a", attr);

	// First the label, if there is one
	docstring const & label = par.params().labelString();
	if (!label.empty())
		xs << label << " ";
	// Now the content of the TOC entry, taken from the paragraph itself
	OutputParams ours = op;
	ours.for_toc = true;
	Font const dummy;
	par.simpleLyXHTMLOnePar(buffer(), xs, ours, dummy);

	xs << html::EndTag("a") << html::CR();
}


void InsetTOC::makeTOCWithDepth(XHTMLStream & xs, 
		Toc toc, OutputParams const & op) const
{
	Toc::const_iterator it = toc.begin();
	Toc::const_iterator const en = toc.end();
	int lastdepth = 0;
	for (; it != en; ++it) {
		// do not output entries that are not actually included in the output,
		// e.g., stuff in non-active branches or notes or whatever.
		if (!it->isOutput())
			continue;

		// First, we need to manage increases and decreases of depth
		// If there's no depth to deal with, we artifically set it to 1.
		int const depth = it->depth();
		
		// Ignore stuff above the tocdepth
		if (depth > buffer().params().tocdepth)
			continue;
		
		if (depth > lastdepth) {
			xs << html::CR();
			// open as many tags as we need to open to get to this level
			// this includes the tag for the current level
			for (int i = lastdepth + 1; i <= depth; ++i) {
				stringstream attr;
				attr << "class='lyxtoc-" << i << "'";
				xs << html::StartTag("div", attr.str()) << html::CR();
			}
			lastdepth = depth;
		}
		else if (depth < lastdepth) {
			// close as many as we have to close to get back to this level
			// this includes closing the last tag at this level
			for (int i = lastdepth; i >= depth; --i) 
				xs << html::EndTag("div") << html::CR();
			// now open our tag
			stringstream attr;
			attr << "class='lyxtoc-" << depth << "'";
			xs << html::StartTag("div", attr.str()) << html::CR();
			lastdepth = depth;
		} else {
			// no change of level, so close and open
			xs << html::EndTag("div") << html::CR();
			stringstream attr;
			attr << "class='lyxtoc-" << depth << "'";
			xs << html::StartTag("div", attr.str()) << html::CR();
		}
		
		// Now output TOC info for this entry
		Paragraph const & par = it->dit().innerParagraph();
		makeTOCEntry(xs, par, op);
	}
	for (int i = lastdepth; i > 0; --i) 
		xs << html::EndTag("div") << html::CR();
}


void InsetTOC::makeTOCNoDepth(XHTMLStream & xs, 
		Toc toc, const OutputParams & op) const
{
	Toc::const_iterator it = toc.begin();
	Toc::const_iterator const en = toc.end();
	for (; it != en; ++it) {
		// do not output entries that are not actually included in the output,
		// e.g., stuff in non-active branches or notes or whatever.
		if (!it->isOutput())
			continue;

		xs << html::StartTag("div", "class='lyxtoc-flat'") << html::CR();

		Paragraph const & par = it->dit().innerParagraph();
		makeTOCEntry(xs, par, op);
		
		xs << html::EndTag("div");
	}
}


docstring InsetTOC::xhtml(XHTMLStream &, OutputParams const & op) const
{
	string const & command = getCmdName();
	if (command != "tableofcontents" && command != "lstlistoflistings") {
		LYXERR0("TOC type " << command << " not yet implemented.");
		LASSERT(false, return docstring());
	}

	Toc const & toc = buffer().masterBuffer()->tocBackend().toc(cmd2type(command));
	if (toc.empty())
		return docstring();

	// we'll use our own stream, because we are going to defer everything.
	// that's how we deal with the fact that we're probably inside a standard
	// paragraph, and we don't want to be.
	odocstringstream ods;
	XHTMLStream xs(ods);

	xs << html::StartTag("div", "class='toc'");

	// Title of TOC
	InsetLayout const & il = getLayout();
	string const & tag = il.htmltag();
	docstring title = screenLabel();
	Layout const & lay = buffer().params().documentClass().htmlTOCLayout();
	string const & tocclass = lay.defaultCSSClass();
	string const tocattr = "class='tochead " + tocclass + "'";
	xs << html::StartTag(tag, tocattr)
		 << title
		 << html::EndTag(tag);

	// with lists of listings, at least, there is no depth
	// to worry about. so the code can be simpler.
	bool const use_depth = (command == "tableofcontents");

	// Output of TOC
	if (use_depth)
		makeTOCWithDepth(xs, toc, op);
	else
		makeTOCNoDepth(xs, toc, op);

	xs << html::EndTag("div") << html::CR();
	return ods.str();
}


} // namespace lyx
