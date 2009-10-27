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
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "sgml.h"
#include "Text.h"
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


// FIXME do something here.
docstring htmlize(docstring const & str) {
	return str;
}

// FIXME This needs to be protected somehow.
static vector<string> taglist;

bool openTag(odocstream & os, string const & tag, string const & attr)
{
	if (tag.empty())
		return false;
	os << from_ascii("<" + tag + (attr.empty() ? "" : " " + attr) + ">");
	taglist.push_back(tag);
	return true;
}


bool closeTag(odocstream & os, string const & tag)
{
	if (tag.empty())
		return false;
	// FIXME Check for proper nesting
	if (taglist.empty()){
		LYXERR0("Last tag not found when closing `" << tag << "'!");
		return false;
	}
	string const & lasttag = taglist.back();
	if (lasttag != tag)  {
		LYXERR0("Last tag was `" << lasttag << "' when closing `" << tag << "'!");
		return false;
	}
	taglist.pop_back();
	os << from_ascii("</" + tag + ">");
	return true;
}



} // html

namespace {

bool openTag(odocstream & os, Layout const & lay)
{
	return html::openTag(os, lay.htmltag(), lay.htmlattr());
}


bool closeTag(odocstream & os, Layout const & lay)
{
	return html::closeTag(os, lay.htmltag());
}


bool openLabelTag(odocstream & os, Layout const & lay)
{
	return html::openTag(os, lay.htmllabeltag(), lay.htmllabelattr());
}


bool closeLabelTag(odocstream & os, Layout const & lay)
{
	return html::closeTag(os, lay.htmllabeltag());
}


bool openItemTag(odocstream & os, Layout const & lay)
{
	return html::openTag(os, lay.htmlitemtag(), lay.htmlitemattr());
}


bool closeItemTag(odocstream & os, Layout const & lay)
{
	return html::closeTag(os, lay.htmlitemtag());
}

ParagraphList::const_iterator searchParagraphHtml(
	ParagraphList::const_iterator p,
	ParagraphList::const_iterator const & pend)
{
	for (++p; p != pend && p->layout().latextype == LATEX_PARAGRAPH; ++p)
		;

	return p;
}


ParagraphList::const_iterator searchEnvironmentHtml(
		ParagraphList::const_iterator const pstart,
		ParagraphList::const_iterator const & pend)
{
	ParagraphList::const_iterator p = pstart;
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


ParagraphList::const_iterator makeParagraphs(Buffer const & buf,
					    odocstream & os,
					    OutputParams const & runparams,
					    Text const & text,
					    ParagraphList::const_iterator const & pbegin,
					    ParagraphList::const_iterator const & pend)
{
	ParagraphList::const_iterator const begin = text.paragraphs().begin();
	ParagraphList::const_iterator par = pbegin;
	for (; par != pend; ++par) {
		Layout const & lay = par->layout();
		if (!lay.counter.empty())
			buf.params().documentClass().counters().step(lay.counter);
		// FIXME We should see if there's a label to be output and
		// do something with it.
		if (par != pbegin)
			os << '\n';

		// FIXME Should we really allow anything other than 'p' here?
		
		// If we are already in a paragraph, and this is the first one, then we
		// do not want to open the paragraph tag.
		bool const opened = 
			(par == pbegin && runparams.html_in_par) ? false : openTag(os, lay);
		docstring const deferred = par->simpleLyXHTMLOnePar(buf, os, runparams,
				text.outerFont(distance(begin, par)));

		// We want to issue the closing tag if either:
		//   (i)  We opened it, and either html_in_par is false,
		//        or we're not in the last paragraph, anyway.
		//   (ii) We didn't open it and html_in_par is true, 
		//        but we are in the first par, and there is a next par.
		ParagraphList::const_iterator nextpar = par;
		nextpar++;
		bool const needClose = 
			(opened && (!runparams.html_in_par || nextpar != pend))
			|| (!opened && runparams.html_in_par && par == pbegin && nextpar != pend);
		if (needClose) {
			closeTag(os, lay);
			os << '\n';
		}
		if (!deferred.empty())
			os << deferred << '\n';
	}
	return pend;
}


ParagraphList::const_iterator makeBibliography(Buffer const & buf,
				odocstream & os,
				OutputParams const & runparams,
				Text const & text,
				ParagraphList::const_iterator const & pbegin,
				ParagraphList::const_iterator const & pend) 
{
	os << "<h2 class='bibliography'>" 
	   << pbegin->layout().labelstring(false) 
	   << "</h2>\n"
	   << "<div class='bibliography'>\n";
			makeParagraphs(buf, os, runparams, text, pbegin, pend);
	os << "</div>";
	return pend;
}


namespace {
	bool isNormalEnv(Layout const & lay)
	{
		return lay.latextype == LATEX_ENVIRONMENT;
	}
}

ParagraphList::const_iterator makeEnvironmentHtml(Buffer const & buf,
					      odocstream & os,
					      OutputParams const & runparams,
					      Text const & text,
					      ParagraphList::const_iterator const & pbegin,
					      ParagraphList::const_iterator const & pend) 
{
	ParagraphList::const_iterator const begin = text.paragraphs().begin();
	ParagraphList::const_iterator par = pbegin;
	Layout const & bstyle = par->layout();
	depth_type const origdepth = pbegin->params().depth();

	// Open tag for this environment
	bool const main_tag_opened = openTag(os, bstyle);
	os << '\n';

	// we will on occasion need to remember a layout from before.
	Layout const * lastlay = 0;

	while (par != pend) {
		Layout const & style = par->layout();
		// the counter only gets stepped if we're in some kind of list,
		// or if it's the first time through.
		if (!style.counter.empty() && (par == pbegin || !isNormalEnv(style)))
			buf.params().documentClass().counters().step(style.counter);
		ParagraphList::const_iterator send;
		// this will be positive, if we want to skip the initial word
		// (if it's been taken for the label).
		pos_type sep = 0;

		switch (style.latextype) {
		case LATEX_ENVIRONMENT:
		case LATEX_LIST_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			// There are two possiblities in this case. 
			// One is that we are still in the environment in which we 
			// started---which we will be if the depth is the same.
			if (par->params().depth() == origdepth) {
				LASSERT(bstyle == style, /* */);
				if (lastlay != 0) {
					closeItemTag(os, *lastlay);
					lastlay = 0;
				}
				bool item_tag_opened = false;
				bool const labelfirst = style.htmllabelfirst();
				bool madelabel = false;
				if (isNormalEnv(style)) {
					// in this case, we print the label only for the first 
					// paragraph (as in a theorem).
					item_tag_opened = openItemTag(os, style);
					if (par == pbegin && style.htmllabeltag() != "NONE") {
						docstring const lbl = 
								pbegin->expandLabel(style, buf.params(), false);
						if (!lbl.empty()) {
							bool const label_tag_opened = openLabelTag(os, style);
							os << lbl;
							if (label_tag_opened)
								closeLabelTag(os, style);
						}
						os << '\n';
					}
				}	else { // some kind of list
					if (!labelfirst)
						item_tag_opened = openItemTag(os, style);
					if (style.labeltype == LABEL_MANUAL
					    && style.htmllabeltag() != "NONE") {
						madelabel = openLabelTag(os, style);
						sep = par->firstWordLyXHTML(os, runparams);
						if (madelabel)
							closeLabelTag(os, style);
						os << '\n';
					}
					else if (style.labeltype != LABEL_NO_LABEL
					         && style.htmllabeltag() != "NONE") {
						madelabel = openLabelTag(os, style);
						os << par->expandLabel(style, buf.params(), false);
						if (madelabel)
							closeLabelTag(os, style);
						os << '\n';
					}
					if (labelfirst)
						item_tag_opened = openItemTag(os, style);
					else if (madelabel)
						os << "<span class='" << style.name() << "inneritem'>";
				}
				par->simpleLyXHTMLOnePar(buf, os, runparams, 
					text.outerFont(distance(begin, par)), sep);
				if (!isNormalEnv(style) && !labelfirst && madelabel)
					os << "</span>";
				++par;
				if (item_tag_opened) {
					// We may not want to close the tag yet, in particular,
					// if we're not at the end...
					if (par != pend 
				    //  and are doing items...
				     && style.latextype == LATEX_ITEM_ENVIRONMENT
				     // and if the depth has changed...
				     && par->params().depth() != origdepth) {
				     // then we'll save this layout for later, and close it when
				     // we get another item.
						lastlay = &style;
					} else
						closeItemTag(os, style);
					os << '\n';
				}
			}
			// The other possibility is that the depth has increased, in which
			// case we need to recurse.
			else {
				send = searchEnvironmentHtml(par, pend);
				par = makeEnvironmentHtml(buf, os, runparams, text, par, send);
			}
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraphHtml(par, pend);
			par = makeParagraphs(buf, os, runparams, text, par, send);
			break;
		// Shouldn't happen
		case LATEX_BIB_ENVIRONMENT:
			send = par;
			++send;
			par = makeParagraphs(buf, os, runparams, text, par, send);
			break;
		// Shouldn't happen
		case LATEX_COMMAND:
			++par;
			break;
		}
	}

	if (lastlay != 0)
		closeItemTag(os, *lastlay);
	if (main_tag_opened)
		closeTag(os, bstyle);
	os << '\n';
	return pend;
}


void makeCommand(Buffer const & buf,
					  odocstream & os,
					  OutputParams const & runparams,
					  Text const & text,
					  ParagraphList::const_iterator const & pbegin)
{
	Layout const & style = pbegin->layout();
	if (!style.counter.empty())
		buf.params().documentClass().counters().step(style.counter);

	bool const main_tag_opened = openTag(os, style);

	// Label around sectioning number:
	// FIXME Probably need to account for LABEL_MANUAL
	if (style.labeltype != LABEL_NO_LABEL) {
		bool const label_tag_opened = openLabelTag(os, style);
		os << pbegin->expandLabel(style, buf.params(), false);
		if (label_tag_opened)
			closeLabelTag(os, style);
		// Otherwise the label might run together with the text
		os << ' ';
	}

	ParagraphList::const_iterator const begin = text.paragraphs().begin();
	pbegin->simpleLyXHTMLOnePar(buf, os, runparams,
			text.outerFont(distance(begin, pbegin)));
	if (main_tag_opened)
		closeTag(os, style);
	os << '\n';
}

} // end anonymous namespace


void xhtmlParagraphs(Text const & text,
		       Buffer const & buf,
		       odocstream & os,
		       OutputParams const & runparams)
{
	ParagraphList const & paragraphs = text.paragraphs();
	ParagraphList::const_iterator par = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();

	OutputParams ourparams = runparams;
	while (par != pend) {
		Layout const & style = par->layout();
		ParagraphList::const_iterator lastpar = par;
		ParagraphList::const_iterator send;

		switch (style.latextype) {
		case LATEX_COMMAND: {
			// The files with which we are working never have more than
			// one paragraph in a command structure.
			// FIXME 
			// if (ourparams.html_in_par)
			//   fix it so we don't get sections inside standard, e.g.
			// note that we may then need to make runparams not const, so we
			// can communicate that back.
			// FIXME Maybe this fix should be in the routines themselves, in case
			// they are called from elsewhere.
			makeCommand(buf, os, ourparams, text, par);
			++par;
			break;
		}
		case LATEX_ENVIRONMENT:
		case LATEX_LIST_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			// FIXME Same fix here.
			send = searchEnvironmentHtml(par, pend);
			par = makeEnvironmentHtml(buf, os, ourparams, text, par, send);
			break;
		}
		case LATEX_BIB_ENVIRONMENT: {
			// FIXME Same fix here.
			send = searchEnvironmentHtml(par, pend);
			par = makeBibliography(buf, os, ourparams, text, par, send);
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraphHtml(par, pend);
			par = makeParagraphs(buf, os, ourparams, text, par, send);
			break;
		}
		// FIXME??
		// makeEnvironment may process more than one paragraphs and bypass pend
		if (distance(lastpar, par) >= distance(lastpar, pend))
			break;
	}
}


} // namespace lyx
