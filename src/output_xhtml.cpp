/**
 * \file output_xhtml.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 * 
 * This code is based upon output_docbook.cpp
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "output_xhtml.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Layout.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "sgml.h"
#include "TextClass.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/lstrings.h"

#include <boost/next_prior.hpp>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace html {

docstring escapeChar(char_type c)
{
	docstring str;
	switch (c) {
	case ' ':
		str += " ";
		break;
	case '&':
		str += "&amp;";
		break;
	case '<':
		str += "&lt;";
		break;
	case '>':
		str += "&gt;";
		break;
	default:
		str += c;
		break;
	}
	return str;
}
} // html

namespace {

static vector<string> taglist;

void openTag(odocstream & os, Layout const & lay)
{
	string const & tag = lay.htmltag();
	if (tag.empty())
		return;
	// Now do some checks on nesting of tags.
	if (tag == "p")
		if (find(taglist.begin(), taglist.end(), "p") != taglist.end())
			return;
	// FIXME More? Better?
	string attr = lay.htmlattr();
	if (!attr.empty())
		attr = ' ' + attr;
	os << from_ascii("<" + tag + attr + ">");
	taglist.push_back(tag);
}


void closeTag(odocstream & os, Layout const & lay)
{
	string const & tag = lay.htmltag();
	if (tag.empty() || taglist.empty())
		// Here we will just assume this does not need to be done.
		return;
	if (find(taglist.begin(), taglist.end(), tag) == taglist.end())
		return;
	os << from_ascii("</" + tag + ">");
	// Check for proper nesting
	string const & lasttag = taglist.back();
	if (lasttag != tag)  {
		LYXERR0("Last tag was `" << lasttag << "' when closing `" << tag << "'!");
		return;
	}
	taglist.pop_back();
}


void openLabelTag(odocstream & os, Layout const & lay)
{
	string const & tag = lay.htmllabel();
	if (tag.empty())
		return;
	string attr = lay.htmllabelattr();
	if (!attr.empty())
		attr = ' ' + attr;
	os << from_ascii("<" + tag + attr + ">");
}


void closeLabelTag(odocstream & os, Layout const & lay)
{
	string const & tag = lay.htmllabel();
	if (tag.empty())
		return;
	os << from_ascii("</" + tag + ">");
}


void openItemTag(odocstream & os, Layout const & lay)
{
	string const & tag = lay.htmlitem();
	if (tag.empty())
		return;
	string attr = lay.htmlitemattr();
	if (!attr.empty())
		attr = ' ' + attr;
	os << from_ascii("<" + tag + attr + ">");
}


void closeItemTag(odocstream & os, Layout const & lay)
{
	string const & tag = lay.htmlitem();
	if (tag.empty())
		return;
	os << from_ascii("</" + tag + ">");
}


ParagraphList::const_iterator searchParagraph(
	ParagraphList::const_iterator p,
  ParagraphList::const_iterator const & pend)
{
	for (++p; p != pend && p->layout().latextype == LATEX_PARAGRAPH; ++p)
		;

	return p;
}


ParagraphList::const_iterator searchEnvironment(
		ParagraphList::const_iterator p,
		ParagraphList::const_iterator const & pend)
{
	Layout const & bstyle = p->layout();
	size_t const depth = p->params().depth();
	for (++p; p != pend; ++p) {
		Layout const & style = p->layout();
		// It shouldn't happen that e.g. a section command occurs inside
		// a quotation environment, at a higher depth, but as of 6/2009,
		// it can happen. We pretend that it's just at lowest depth.
		if (style.latextype == LATEX_COMMAND)
			return p;
		// If depth is down, we're done
		if (p->params().depth() < depth)
			return p;
		// If depth is up, we're not done
		if (p->params().depth() > depth)
			continue;
		// Now we know we are at the same depth
		if (style.latextype == LATEX_PARAGRAPH
		    || style.latexname() != bstyle.latexname())
			return p;

	}
	return pend;
}


ParagraphList::const_iterator makeParagraph(Buffer const & buf,
					    odocstream & os,
					    OutputParams const & runparams,
					    ParagraphList const & paragraphs,
					    ParagraphList::const_iterator const & pbegin,
					    ParagraphList::const_iterator const & pend)
{
	ParagraphList::const_iterator par = pbegin;
	for (; par != pend; ++par) {
		Layout const & lay = par->layout();
		if (par != pbegin)
			os << '\n';
		openTag(os, lay);
		par->simpleLyXHTMLOnePar(buf, os, runparams, 
				outerFont(distance(paragraphs.begin(), par), paragraphs));
		closeTag(os, lay);
		os << '\n';
	}
	return pend;
}

ParagraphList::const_iterator makeEnvironment(Buffer const & buf,
					      odocstream & os,
					      OutputParams const & runparams,
					      ParagraphList const & paragraphs,
					      ParagraphList::const_iterator const & pbegin,
					      ParagraphList::const_iterator const & pend) {
	
	ParagraphList::const_iterator par = pbegin;
	Layout const & bstyle = par->layout();
	depth_type const origdepth = pbegin->params().depth();

	// Open tag for this environment
	openTag(os, bstyle);
	os << '\n';

	// we will on occasion need to remember a layout from before.
	Layout const * lastlay = 0;

	while (par != pend) {
		Layout const & style = par->layout();
		ParagraphList::const_iterator send;
		// this will be positive, if we want to skip the initial word
		// (if it's been taken for the label).
		pos_type sep = 0;

		switch (style.latextype) {
		case LATEX_ENVIRONMENT:
		// case LATEX_LIST_ENVIRONMENT??
		case LATEX_ITEM_ENVIRONMENT: {
			// There are two possiblities in this case. 
			// One is that we are still in the environment in which we 
			// started---which we will be if the depth is the same.
			if (par->params().depth() == origdepth) {
				if (lastlay != 0) {
					closeItemTag(os, *lastlay);
					lastlay = 0;
				}
				Layout const & cstyle = par->layout();
				if (cstyle.labeltype == LABEL_MANUAL) {
					openLabelTag(os, cstyle);
					sep = par->firstWordLyXHTML(os, runparams);
					closeLabelTag(os, cstyle);
					os << '\n';
				} else if (style.latextype == LATEX_ENVIRONMENT 
				           && style.labeltype != LABEL_NO_LABEL) {
					openLabelTag(os, cstyle);
					if (!style.counter.empty())
						buf.params().documentClass().counters().step(cstyle.counter);
					os << pbegin->expandLabel(style, buf.params(), false);
					closeLabelTag(os, cstyle);
					os << '\n';
				}

				openItemTag(os, cstyle);
				par->simpleLyXHTMLOnePar(buf, os, runparams, 
					outerFont(distance(paragraphs.begin(), par), paragraphs), sep);
				++par;
				// We may not want to close the tag yet, in particular,
				// if we're not at the end and are doing items...
				if (par != pend && style.latextype == LATEX_ITEM_ENVIRONMENT
				    // and if the depth has changed,
				    && par->params().depth() != origdepth) {
						// then we'll save this layout for later, and close it when
						// we get another item.
						lastlay = &cstyle;
				} else {
					closeItemTag(os, cstyle); // FIXME
					os << '\n';
				}
			} 
			// The other possibility is that the depth has increased, in which
			// case we need to recurse.
			else {
				send = searchEnvironment(par, pend);
				par = makeEnvironment(buf, os, runparams, paragraphs, par, send);
			}
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraph(par, pend);
			par = makeParagraph(buf, os, runparams, paragraphs, par, send);
			break;
		// FIXME
		case LATEX_BIB_ENVIRONMENT:
		case LATEX_COMMAND:
			break;
		}
	}

	if (lastlay != 0)
		closeItemTag(os, *lastlay);
	closeTag(os, bstyle);
	os << '\n';
	return pend;
}


void makeCommand(Buffer const & buf,
					  odocstream & os,
					  OutputParams const & runparams,
					  ParagraphList const & paragraphs,
					  ParagraphList::const_iterator const & pbegin)
{
	Layout const & style = pbegin->layout();

	openTag(os, style);

	// Label around sectioning number:
	if (style.labeltype != LABEL_NO_LABEL) {
		openLabelTag(os, style);
		if (!style.counter.empty())
			buf.params().documentClass().counters().step(style.counter);
		os << pbegin->expandLabel(style, buf.params(), false);
		closeLabelTag(os, style);
		// Otherwise the label might run together with the text
		os << ' ';
	}

	pbegin->simpleLyXHTMLOnePar(buf, os, runparams,
			outerFont(distance(paragraphs.begin(), pbegin), paragraphs));
	closeTag(os, style);
}

} // end anonymous namespace


void xhtmlParagraphs(ParagraphList const & paragraphs,
		       Buffer const & buf,
		       odocstream & os,
		       OutputParams const & runparams)
{
	ParagraphList::const_iterator par = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();

	while (par != pend) {
		Layout const & style = par->layout();
		ParagraphList::const_iterator lastpar = par;
		ParagraphList::const_iterator send;

		switch (style.latextype) {
		case LATEX_COMMAND: {
			// The files with which we are working never have more than
			// one paragraph in a command structure.
			makeCommand(buf, os, runparams, paragraphs, par);
			++par;
			break;
		}
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			send = searchEnvironment(par, pend);
			par = makeEnvironment(buf, os, runparams, paragraphs, par,send);
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraph(par, pend);
			par = makeParagraph(buf, os, runparams, paragraphs, par,send);
			break;
		default:
			break;
		}
		os << '\n';
		// makeEnvironment may process more than one paragraphs and bypass pend
		if (distance(lastpar, par) >= distance(lastpar, pend))
			break;
	}
}


} // namespace lyx
