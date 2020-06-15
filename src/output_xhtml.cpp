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
#include "Font.h"
#include "Layout.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "xml.h"
#include "Text.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <vector>

// Uncomment to activate debugging code.
// #define XHTML_DEBUG

using namespace std;
using namespace lyx::support;

namespace lyx {


docstring fontToHtmlTag(xml::FontTypes type)
{
    switch(type) {
        case xml::FontTypes::FT_EMPH:
            return from_utf8("em");
        case xml::FontTypes::FT_BOLD:
            return from_utf8("b");
        case xml::FontTypes::FT_NOUN:
            return from_utf8("dfn");
        case xml::FontTypes::FT_UBAR:
        case xml::FontTypes::FT_WAVE:
        case xml::FontTypes::FT_DBAR:
            return from_utf8("u");
        case xml::FontTypes::FT_SOUT:
        case xml::FontTypes::FT_XOUT:
            return from_utf8("del");
        case xml::FontTypes::FT_ITALIC:
            return from_utf8("i");
        case xml::FontTypes::FT_UPRIGHT:
        case xml::FontTypes::FT_SLANTED:
        case xml::FontTypes::FT_SMALLCAPS:
        case xml::FontTypes::FT_ROMAN:
        case xml::FontTypes::FT_SANS:
        case xml::FontTypes::FT_TYPE:
        case xml::FontTypes::FT_SIZE_TINY:
        case xml::FontTypes::FT_SIZE_SCRIPT:
        case xml::FontTypes::FT_SIZE_FOOTNOTE:
        case xml::FontTypes::FT_SIZE_SMALL:
        case xml::FontTypes::FT_SIZE_NORMAL:
        case xml::FontTypes::FT_SIZE_LARGE:
        case xml::FontTypes::FT_SIZE_LARGER:
        case xml::FontTypes::FT_SIZE_LARGEST:
        case xml::FontTypes::FT_SIZE_HUGE:
        case xml::FontTypes::FT_SIZE_HUGER:
        case xml::FontTypes::FT_SIZE_INCREASE:
        case xml::FontTypes::FT_SIZE_DECREASE:
            return from_utf8("span");
    }
    // kill warning
    return docstring();
}


docstring fontToHtmlAttribute(xml::FontTypes type)
{
	switch(type) {
	case xml::FontTypes::FT_EMPH:
	case xml::FontTypes::FT_BOLD:
		return from_ascii("");
	case xml::FontTypes::FT_NOUN:
		return from_ascii("class='lyxnoun'");
	case xml::FontTypes::FT_UBAR:
		return from_ascii("");
	case xml::FontTypes::FT_DBAR:
		return from_ascii("class='dline'");
	case xml::FontTypes::FT_XOUT:
	case xml::FontTypes::FT_SOUT:
		return from_ascii("class='strikeout'");
	case xml::FontTypes::FT_WAVE:
		return from_ascii("class='wline'");
	case xml::FontTypes::FT_ITALIC:
		return from_ascii("");
	case xml::FontTypes::FT_UPRIGHT:
		return from_ascii("style='font-style:normal;'");
	case xml::FontTypes::FT_SLANTED:
		return from_ascii("style='font-style:oblique;'");
	case xml::FontTypes::FT_SMALLCAPS:
		return from_ascii("style='font-variant:small-caps;'");
	case xml::FontTypes::FT_ROMAN:
		return from_ascii("style='font-family:serif;'");
	case xml::FontTypes::FT_SANS:
		return from_ascii("style='font-family:sans-serif;'");
	case xml::FontTypes::FT_TYPE:
		return from_ascii("style='font-family:monospace;'");
	case xml::FontTypes::FT_SIZE_TINY:
	case xml::FontTypes::FT_SIZE_SCRIPT:
	case xml::FontTypes::FT_SIZE_FOOTNOTE:
		return from_ascii("style='font-size:x-small;'");
	case xml::FontTypes::FT_SIZE_SMALL:
		return from_ascii("style='font-size:small;'");
	case xml::FontTypes::FT_SIZE_NORMAL:
		return from_ascii("style='font-size:normal;'");
	case xml::FontTypes::FT_SIZE_LARGE:
		return from_ascii("style='font-size:large;'");
	case xml::FontTypes::FT_SIZE_LARGER:
	case xml::FontTypes::FT_SIZE_LARGEST:
		return from_ascii("style='font-size:x-large;'");
	case xml::FontTypes::FT_SIZE_HUGE:
	case xml::FontTypes::FT_SIZE_HUGER:
		return from_ascii("style='font-size:xx-large;'");
	case xml::FontTypes::FT_SIZE_INCREASE:
		return from_ascii("style='font-size:larger;'");
	case xml::FontTypes::FT_SIZE_DECREASE:
		return from_ascii("style='font-size:smaller;'");
	}
	// kill warning
	return from_ascii("");
}


xml::FontTag xhtmlStartFontTag(xml::FontTypes type)
{
	return xml::FontTag(fontToHtmlTag(type), fontToHtmlAttribute(type), type);
}


xml::EndFontTag xhtmlEndFontTag(xml::FontTypes type)
{
	return xml::EndFontTag(fontToHtmlTag(type), type);
}

namespace {

// convenience functions

inline void openParTag(XMLStream & xs, Layout const & lay,
                       std::string parlabel)
{
	xs << xml::ParTag(lay.htmltag(), lay.htmlattr(), parlabel);
}


void openParTag(XMLStream & xs, Layout const & lay,
                ParagraphParameters const & params,
                std::string parlabel)
{
	// FIXME Are there other things we should handle here?
	string const align = alignmentToCSS(params.align());
	if (align.empty()) {
		openParTag(xs, lay, parlabel);
		return;
	}
	string attrs = lay.htmlattr() + " style='text-align: " + align + ";'";
	xs << xml::ParTag(lay.htmltag(), attrs, parlabel);
}


inline void closeTag(XMLStream & xs, Layout const & lay)
{
	xs << xml::EndTag(lay.htmltag());
}


inline void openLabelTag(XMLStream & xs, Layout const & lay)
{
	xs << xml::StartTag(lay.htmllabeltag(), lay.htmllabelattr());
}


inline void closeLabelTag(XMLStream & xs, Layout const & lay)
{
	xs << xml::EndTag(lay.htmllabeltag());
}


inline void openItemTag(XMLStream & xs, Layout const & lay)
{
	xs << xml::StartTag(lay.htmlitemtag(), lay.htmlitemattr(), true);
}


void openItemTag(XMLStream & xs, Layout const & lay,
             ParagraphParameters const & params)
{
	// FIXME Are there other things we should handle here?
	string const align = alignmentToCSS(params.align());
	if (align.empty()) {
		openItemTag(xs, lay);
		return;
	}
	string attrs = lay.htmlattr() + " style='text-align: " + align + ";'";
	xs << xml::StartTag(lay.htmlitemtag(), attrs);
}


inline void closeItemTag(XMLStream & xs, Layout const & lay)
{
	xs << xml::EndTag(lay.htmlitemtag());
}

// end of convenience functions

ParagraphList::const_iterator findLastParagraph(
	ParagraphList::const_iterator p,
	ParagraphList::const_iterator const & pend)
{
	for (++p; p != pend && p->layout().latextype == LATEX_PARAGRAPH; ++p)
		;

	return p;
}


ParagraphList::const_iterator findEndOfEnvironment(
		ParagraphList::const_iterator const & pstart,
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

		// FIXME I am not sure about the first check.
		// Surely we *could* have different layouts that count as
		// LATEX_PARAGRAPH, right?
		if (style.latextype == LATEX_PARAGRAPH || style != bstyle)
			return p;
	}
	return pend;
}


ParagraphList::const_iterator makeParagraphs(Buffer const & buf,
					    XMLStream & xs,
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
			buf.masterBuffer()->params().
			    documentClass().counters().step(lay.counter, OutputUpdate);

		// FIXME We should see if there's a label to be output and
		// do something with it.
		if (par != pbegin)
			xs << xml::CR();

		// We want to open the paragraph tag if:
		//   (i) the current layout permits multiple paragraphs
		//  (ii) we are either not already inside a paragraph (HTMLIsBlock) OR
		//       we are, but this is not the first paragraph
		//
		// But there is also a special case, and we first see whether we are in it.
		// We do not want to open the paragraph tag if this paragraph contains
		// only one item, and that item is "inline", i.e., not HTMLIsBlock (such
		// as a branch). On the other hand, if that single item has a font change
		// applied to it, then we still do need to open the paragraph.
		//
		// Obviously, this is very fragile. The main reason we need to do this is
		// because of branches, e.g., a branch that contains an entire new section.
		// We do not really want to wrap that whole thing in a <div>...</div>.
		bool special_case = false;
		Inset const * specinset = par->size() == 1 ? par->getInset(0) : nullptr;
		if (specinset && !specinset->getLayout().htmlisblock()) {
			Layout const & style = par->layout();
			FontInfo const first_font = style.labeltype == LABEL_MANUAL ?
						style.labelfont : style.font;
			FontInfo const our_font =
				par->getFont(buf.masterBuffer()->params(), 0,
			               text.outerFont(distance(begin, par))).fontInfo();
			if (first_font == our_font)
				special_case = true;
		}

		bool const open_par = runparams.html_make_pars
			&& (!runparams.html_in_par || par != pbegin)
			&& !special_case;

		// We want to issue the closing tag if either:
		//   (i)  We opened it, and either html_in_par is false,
		//        or we're not in the last paragraph, anyway.
		//   (ii) We didn't open it and html_in_par is true,
		//        but we are in the first par, and there is a next par.
		ParagraphList::const_iterator nextpar = par;
		++nextpar;
		bool const close_par =
			(open_par && (!runparams.html_in_par || nextpar != pend))
			|| (!open_par && runparams.html_in_par && par == pbegin && nextpar != pend);

		if (open_par) {
			// We do not issue the paragraph id if we are doing
			// this for the TOC (or some similar purpose)
			openParTag(xs, lay, par->params(),
			           runparams.for_toc ? "" : par->magicLabel());
		}

		docstring const deferred = par->simpleLyXHTMLOnePar(buf, xs,
			runparams, text.outerFont(distance(begin, par)),
			open_par, close_par);

		if (close_par) {
			closeTag(xs, lay);
			xs << xml::CR();
		}

		if (!deferred.empty()) {
			xs << XMLStream::ESCAPE_NONE << deferred << xml::CR();
		}
	}
	return pend;
}


ParagraphList::const_iterator makeBibliography(Buffer const & buf,
				XMLStream & xs,
				OutputParams const & runparams,
				Text const & text,
				ParagraphList::const_iterator const & pbegin,
				ParagraphList::const_iterator const & pend)
{
	// FIXME XHTML
	// Use TextClass::htmlTOCLayout() to figure out how we should look.
	xs << xml::StartTag("h2", "class='bibliography'")
	   << pbegin->layout().labelstring(false)
	   << xml::EndTag("h2")
	   << xml::CR()
	   << xml::StartTag("div", "class='bibliography'")
	   << xml::CR();
	makeParagraphs(buf, xs, runparams, text, pbegin, pend);
	xs << xml::EndTag("div");
	return pend;
}


bool isNormalEnv(Layout const & lay)
{
	return lay.latextype == LATEX_ENVIRONMENT
	    || lay.latextype == LATEX_BIB_ENVIRONMENT;
}


ParagraphList::const_iterator makeEnvironment(Buffer const & buf,
					      XMLStream & xs,
					      OutputParams const & runparams,
					      Text const & text,
					      ParagraphList::const_iterator const & pbegin,
					      ParagraphList::const_iterator const & pend)
{
	ParagraphList::const_iterator const begin = text.paragraphs().begin();
	ParagraphList::const_iterator par = pbegin;
	Layout const & bstyle = par->layout();
	depth_type const origdepth = pbegin->params().depth();

	// open tag for this environment
	openParTag(xs, bstyle, pbegin->magicLabel());
	xs << xml::CR();

	// we will on occasion need to remember a layout from before.
	Layout const * lastlay = nullptr;

	while (par != pend) {
		Layout const & style = par->layout();
		// the counter only gets stepped if we're in some kind of list,
		// or if it's the first time through.
		// note that enum, etc, are handled automatically.
		// FIXME There may be a bug here about user defined enumeration
		// types. If so, then we'll need to take the counter and add "i",
		// "ii", etc, as with enum.
		Counters & cnts = buf.masterBuffer()->params().documentClass().counters();
		docstring const & cntr = style.counter;
		if (!style.counter.empty()
		    && (par == pbegin || !isNormalEnv(style))
				&& cnts.hasCounter(cntr)
		)
			cnts.step(cntr, OutputUpdate);
		ParagraphList::const_iterator send;

		switch (style.latextype) {
		case LATEX_ENVIRONMENT:
		case LATEX_LIST_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			// There are two possibilities in this case.
			// One is that we are still in the environment in which we
			// started---which we will be if the depth is the same.
			if (par->params().depth() == origdepth) {
				LATTEST(bstyle == style);
				if (lastlay != nullptr) {
					closeItemTag(xs, *lastlay);
					lastlay = nullptr;
				}

				// this will be positive, if we want to skip the
				// initial word (if it's been taken for the label).
				pos_type sep = 0;
				bool const labelfirst = style.htmllabelfirst();
				if (!labelfirst)
					openItemTag(xs, style, par->params());

				// label output
				if (style.labeltype != LABEL_NO_LABEL &&
				    style.htmllabeltag() != "NONE") {
					if (isNormalEnv(style)) {
						// in this case, we print the label only for the first
						// paragraph (as in a theorem).
						if (par == pbegin) {
							docstring const lbl =
									pbegin->params().labelString();
							if (!lbl.empty()) {
								openLabelTag(xs, style);
								xs << lbl;
								closeLabelTag(xs, style);
							}
							xs << xml::CR();
						}
					} else { // some kind of list
						if (style.labeltype == LABEL_MANUAL) {
							openLabelTag(xs, style);
							sep = par->firstWordLyXHTML(xs, runparams);
							closeLabelTag(xs, style);
							xs << xml::CR();
						}
						else {
							openLabelTag(xs, style);
							xs << par->params().labelString();
							closeLabelTag(xs, style);
							xs << xml::CR();
						}
					}
				} // end label output

				if (labelfirst)
					openItemTag(xs, style, par->params());

				docstring deferred = par->simpleLyXHTMLOnePar(buf, xs, runparams,
					text.outerFont(distance(begin, par)), true, true, sep);
				xs << XMLStream::ESCAPE_NONE << deferred;
				++par;

				// We may not want to close the tag yet, in particular:
				// If we're not at the end...
				if (par != pend
					//  and are doing items...
					 && !isNormalEnv(style)
					 // and if the depth has changed...
					 && par->params().depth() != origdepth) {
					 // then we'll save this layout for later, and close it when
					 // we get another item.
					lastlay = &style;
				} else
					closeItemTag(xs, style);
				xs << xml::CR();
			}
			// The other possibility is that the depth has increased, in which
			// case we need to recurse.
			else {
				send = findEndOfEnvironment(par, pend);
				par = makeEnvironment(buf, xs, runparams, text, par, send);
			}
			break;
		}
		case LATEX_PARAGRAPH:
			send = findLastParagraph(par, pend);
			par = makeParagraphs(buf, xs, runparams, text, par, send);
			break;
		// Shouldn't happen
		case LATEX_BIB_ENVIRONMENT:
			send = par;
			++send;
			par = makeParagraphs(buf, xs, runparams, text, par, send);
			break;
		// Shouldn't happen
		case LATEX_COMMAND:
			++par;
			break;
		}
	}

	if (lastlay != nullptr)
		closeItemTag(xs, *lastlay);
	closeTag(xs, bstyle);
	xs << xml::CR();
	return pend;
}


void makeCommand(Buffer const & buf,
		 XMLStream & xs,
		 OutputParams const & runparams,
		 Text const & text,
		 ParagraphList::const_iterator const & pbegin)
{
	Layout const & style = pbegin->layout();
	if (!style.counter.empty())
		buf.masterBuffer()->params().
		    documentClass().counters().step(style.counter, OutputUpdate);

	bool const make_parid = !runparams.for_toc && runparams.html_make_pars;

	openParTag(xs, style, pbegin->params(),
	           make_parid ? pbegin->magicLabel() : "");

	// Label around sectioning number:
	// FIXME Probably need to account for LABEL_MANUAL
	// FIXME Probably also need now to account for labels ABOVE and CENTERED.
	if (style.labeltype != LABEL_NO_LABEL) {
		openLabelTag(xs, style);
		xs << pbegin->params().labelString();
		closeLabelTag(xs, style);
		// Otherwise the label might run together with the text
		xs << from_ascii(" ");
	}

	ParagraphList::const_iterator const begin = text.paragraphs().begin();
	pbegin->simpleLyXHTMLOnePar(buf, xs, runparams,
			text.outerFont(distance(begin, pbegin)));
	closeTag(xs, style);
	xs << xml::CR();
}

} // end anonymous namespace


void xhtmlParagraphs(Text const & text,
		       Buffer const & buf,
		       XMLStream & xs,
		       OutputParams const & runparams)
{
	ParagraphList const & paragraphs = text.paragraphs();
	if (runparams.par_begin == runparams.par_end) {
		runparams.par_begin = 0;
		runparams.par_end = paragraphs.size();
	}
	pit_type bpit = runparams.par_begin;
	pit_type const epit = runparams.par_end;
	LASSERT(bpit < epit,
		{ xs << XMLStream::ESCAPE_NONE << "<!-- XHTML output error! -->\n"; return; });

	OutputParams ourparams = runparams;
	ParagraphList::const_iterator const pend =
		(epit == (int) paragraphs.size()) ?
			paragraphs.end() : paragraphs.iterator_at(epit);
	while (bpit < epit) {
		ParagraphList::const_iterator par = paragraphs.iterator_at(bpit);
		if (par->params().startOfAppendix()) {
			// We want to reset the counter corresponding to toplevel sectioning
			Layout const & lay =
				buf.masterBuffer()->params().documentClass().getTOCLayout();
			docstring const cnt = lay.counter;
			if (!cnt.empty()) {
				Counters & cnts =
					buf.masterBuffer()->params().documentClass().counters();
				cnts.reset(cnt);
			}
		}
		Layout const & style = par->layout();
		ParagraphList::const_iterator const lastpar = par;
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
			makeCommand(buf, xs, ourparams, text, par);
			++par;
			break;
		}
		case LATEX_ENVIRONMENT:
		case LATEX_LIST_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			// FIXME Same fix here.
			send = findEndOfEnvironment(par, pend);
			par = makeEnvironment(buf, xs, ourparams, text, par, send);
			break;
		}
		case LATEX_BIB_ENVIRONMENT: {
			// FIXME Same fix here.
			send = findEndOfEnvironment(par, pend);
			par = makeBibliography(buf, xs, ourparams, text, par, send);
			break;
		}
		case LATEX_PARAGRAPH:
			send = findLastParagraph(par, pend);
			par = makeParagraphs(buf, xs, ourparams, text, par, send);
			break;
		}
		bpit += distance(lastpar, par);
	}
}


string alignmentToCSS(LyXAlignment align)
{
	switch (align) {
	case LYX_ALIGN_BLOCK:
		// we are NOT going to use text-align: justify!!
	case LYX_ALIGN_LEFT:
		return "left";
	case LYX_ALIGN_RIGHT:
		return "right";
	case LYX_ALIGN_CENTER:
		return "center";
	default:
		break;
	}
	return "";
}

} // namespace lyx
