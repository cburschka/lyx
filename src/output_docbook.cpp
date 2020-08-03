/**
 * \file output_docbook.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "Font.h"
#include "InsetList.h"
#include "Layout.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "xml.h"
#include "Text.h"
#include "TextClass.h"

#include "insets/InsetBibtex.h"
#include "insets/InsetBibitem.h"
#include "insets/InsetLabel.h"
#include "insets/InsetNote.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include "support/regex.h"

#include <stack>
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

std::string fontToDocBookTag(xml::FontTypes type)
{
	switch (type) {
	case xml::FontTypes::FT_EMPH:
	case xml::FontTypes::FT_BOLD:
		return "emphasis";
	case xml::FontTypes::FT_NOUN:
		return "person";
	case xml::FontTypes::FT_UBAR:
	case xml::FontTypes::FT_WAVE:
	case xml::FontTypes::FT_DBAR:
	case xml::FontTypes::FT_SOUT:
	case xml::FontTypes::FT_XOUT:
	case xml::FontTypes::FT_ITALIC:
	case xml::FontTypes::FT_UPRIGHT:
	case xml::FontTypes::FT_SLANTED:
	case xml::FontTypes::FT_SMALLCAPS:
	case xml::FontTypes::FT_ROMAN:
	case xml::FontTypes::FT_SANS:
		return "emphasis";
	case xml::FontTypes::FT_TYPE:
		return "code";
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
		return "emphasis";
	default:
		return "";
	}
}


string fontToRole(xml::FontTypes type)
{
	// Specific fonts are achieved with roles. The only common ones are "" for basic emphasis,
	// and "bold"/"strong" for bold. With some specific options, other roles are copied into
	// HTML output (via the DocBook XSLT sheets); otherwise, if not recognised, they are just ignored.
	// Hence, it is not a problem to have many roles by default here.
	// See https://www.sourceware.org/ml/docbook/2003-05/msg00269.html
	switch (type) {
	case xml::FontTypes::FT_ITALIC:
	case xml::FontTypes::FT_EMPH:
		return "";
	case xml::FontTypes::FT_BOLD:
		return "bold";
	case xml::FontTypes::FT_NOUN: // Outputs a <person>
	case xml::FontTypes::FT_TYPE: // Outputs a <code>
		return "";
	case xml::FontTypes::FT_UBAR:
		return "underline";

	// All other roles are non-standard for DocBook.

	case xml::FontTypes::FT_WAVE:
		return "wave";
	case xml::FontTypes::FT_DBAR:
		return "dbar";
	case xml::FontTypes::FT_SOUT:
		return "sout";
	case xml::FontTypes::FT_XOUT:
		return "xout";
	case xml::FontTypes::FT_UPRIGHT:
		return "upright";
	case xml::FontTypes::FT_SLANTED:
		return "slanted";
	case xml::FontTypes::FT_SMALLCAPS:
		return "smallcaps";
	case xml::FontTypes::FT_ROMAN:
		return "roman";
	case xml::FontTypes::FT_SANS:
		return "sans";
	case xml::FontTypes::FT_SIZE_TINY:
		return "tiny";
	case xml::FontTypes::FT_SIZE_SCRIPT:
		return "size_script";
	case xml::FontTypes::FT_SIZE_FOOTNOTE:
		return "size_footnote";
	case xml::FontTypes::FT_SIZE_SMALL:
		return "size_small";
	case xml::FontTypes::FT_SIZE_NORMAL:
		return "size_normal";
	case xml::FontTypes::FT_SIZE_LARGE:
		return "size_large";
	case xml::FontTypes::FT_SIZE_LARGER:
		return "size_larger";
	case xml::FontTypes::FT_SIZE_LARGEST:
		return "size_largest";
	case xml::FontTypes::FT_SIZE_HUGE:
		return "size_huge";
	case xml::FontTypes::FT_SIZE_HUGER:
		return "size_huger";
	case xml::FontTypes::FT_SIZE_INCREASE:
		return "size_increase";
	case xml::FontTypes::FT_SIZE_DECREASE:
		return "size_decrease";
	default:
		return "";
	}
}

string fontToAttribute(xml::FontTypes type) {
	// If there is a role (i.e. nonstandard use of a tag), output the attribute. Otherwise, the sheer tag is sufficient
	// for the font.
	string role = fontToRole(type);
	if (!role.empty()) {
		return "role='" + role + "'";
	} else {
		return "";
	}
}

} // end anonymous namespace


xml::FontTag docbookStartFontTag(xml::FontTypes type)
{
	return xml::FontTag(from_utf8(fontToDocBookTag(type)), from_utf8(fontToAttribute(type)), type);
}


xml::EndFontTag docbookEndFontTag(xml::FontTypes type)
{
	return xml::EndFontTag(from_utf8(fontToDocBookTag(type)), type);
}


namespace {

// convenience functions

void openParTag(XMLStream & xs, const Paragraph * par, const Paragraph * prevpar)
{
	Layout const & lay = par->layout();

	if (par == prevpar)
		prevpar = nullptr;

	// When should the wrapper be opened here? Only if the previous paragraph has the SAME wrapper tag
	// (usually, they won't have the same layout) and the CURRENT one allows merging.
	// The main use case is author information in several paragraphs: if the name of the author is the
	// first paragraph of an author, then merging with the previous tag does not make sense. Say the
	// next paragraph is the affiliation, then it should be output in the same <author> tag (different
	// layout, same wrapper tag).
	bool openWrapper = lay.docbookwrappertag() != "NONE";
	if (prevpar != nullptr) {
		Layout const & prevlay = prevpar->layout();
		if (prevlay.docbookwrappertag() != "NONE") {
			openWrapper = prevlay.docbookwrappertag() == lay.docbookwrappertag()
					&& !lay.docbookwrappermergewithprevious();
		}
	}

	// Main logic.
	if (openWrapper)
		xs << xml::StartTag(lay.docbookwrappertag(), lay.docbookwrapperattr());

	string tag = lay.docbooktag();
	if (tag == "Plain Layout")
		tag = "para";

	if (!xs.isTagOpen(xml::ParTag(tag, lay.docbookattr()), 1)) // Don't nest a paragraph directly in a paragraph.
		xs << xml::ParTag(tag, lay.docbookattr());

	if (lay.docbookitemtag() != "NONE")
		xs << xml::StartTag(lay.docbookitemtag(), lay.docbookitemattr());
}


void closeTag(XMLStream & xs, Paragraph const * par, Paragraph const * nextpar)
{
	Layout const & lay = par->layout();

	if (par == nextpar)
		nextpar = nullptr;

	// See comment in openParTag.
	bool closeWrapper = lay.docbookwrappertag() != "NONE";
	if (nextpar != nullptr) {
		Layout const & nextlay = nextpar->layout();
		if (nextlay.docbookwrappertag() != "NONE") {
			closeWrapper = nextlay.docbookwrappertag() == lay.docbookwrappertag()
					&& !nextlay.docbookwrappermergewithprevious();
		}
	}

	// Main logic.
	if (lay.docbookitemtag() != "NONE")
		xs << xml::EndTag(lay.docbookitemtag());

	string tag = lay.docbooktag();
	if (tag == "Plain Layout")
		tag = "para";

	xs << xml::EndTag(tag);
	if (closeWrapper)
		xs << xml::EndTag(lay.docbookwrappertag());
}


void openLabelTag(XMLStream & xs, Layout const & lay) // Mostly for definition lists.
{
	xs << xml::StartTag(lay.docbookitemlabeltag(), lay.docbookitemlabelattr());
}


void closeLabelTag(XMLStream & xs, Layout const & lay)
{
	xs << xml::EndTag(lay.docbookitemlabeltag());
	xs << xml::CR();
}


void openItemTag(XMLStream & xs, Layout const & lay)
{
	xs << xml::StartTag(lay.docbookitemtag(), lay.docbookitemattr());
}


// Return true when new elements are output in a paragraph, false otherwise.
bool openInnerItemTag(XMLStream & xs, Layout const & lay)
{
	if (lay.docbookiteminnertag() != "NONE") {
		xs << xml::CR();
		xs << xml::ParTag(lay.docbookiteminnertag(), lay.docbookiteminnerattr());

		if (lay.docbookiteminnertag() == "para") {
			return true;
		}
	}
	return false;
}


void closeInnerItemTag(XMLStream & xs, Layout const & lay)
{
	if (lay.docbookiteminnertag()!= "NONE") {
		xs << xml::EndTag(lay.docbookiteminnertag());
		xs << xml::CR();
	}
}


inline void closeItemTag(XMLStream & xs, Layout const & lay)
{
	xs << xml::EndTag(lay.docbookitemtag());
	xs << xml::CR();
}

// end of convenience functions

ParagraphList::const_iterator findLast(
		ParagraphList::const_iterator p,
		ParagraphList::const_iterator const & pend,
		LatexType type) {
	for (++p; p != pend && p->layout().latextype == type; ++p);

	return p;
}

ParagraphList::const_iterator findLastBibliographyParagraph(
		ParagraphList::const_iterator p,
		ParagraphList::const_iterator const & pend) {
	for (++p; p != pend && p->layout().latextype == LATEX_BIB_ENVIRONMENT; ++p);

	return p;
}


ParagraphList::const_iterator findEndOfEnvironment(
		ParagraphList::const_iterator const & pstart,
		ParagraphList::const_iterator const & pend)
{
	ParagraphList::const_iterator p = pstart;
	size_t const depth = p->params().depth();

	for (++p; p != pend; ++p) {
		Layout const &style = p->layout();
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
		if (style.latextype == LATEX_PARAGRAPH || style != p->layout())
			return p;
	}

	return pend;
}


ParagraphList::const_iterator makeParagraphBibliography(
		Buffer const &buf,
		XMLStream &xs,
		OutputParams const &runparams,
		Text const &text,
		ParagraphList::const_iterator const & pbegin,
		ParagraphList::const_iterator const & pend)
{
	auto const begin = text.paragraphs().begin();
	auto const end = text.paragraphs().end();

	// Find the paragraph *before* pbegin.
	ParagraphList::const_iterator pbegin_before = begin;
	if (pbegin != begin) {
		ParagraphList::const_iterator pbegin_before_next = begin;
		++pbegin_before_next;

		while (pbegin_before_next != pbegin) {
			++pbegin_before;
			++pbegin_before_next;
		}
	}

	ParagraphList::const_iterator par = pbegin;

	// If this is the first paragraph in a bibliography, open the bibliography tag.
	if (pbegin != begin && pbegin_before->layout().latextype != LATEX_BIB_ENVIRONMENT) {
		xs << xml::StartTag("bibliography");
		xs << xml::CR();
	}

	// Generate the required paragraphs, but only if they are .
	for (; par != pend; ++par) {
		// Start the precooked bibliography entry. This is very much like opening a paragraph tag.
		// Don't forget the citation ID!
		docstring attr;
		for (auto i = 0; i < par->size(); ++i) {
			Inset const *ip = par->getInset(0);
			if (ip != nullptr && ip->lyxCode() == BIBITEM_CODE) {
				const auto * bibitem = dynamic_cast<const InsetBibitem*>(par->getInset(i));
				attr = from_utf8("xml:id='") + bibitem->getParam("key") + from_utf8("'");
				break;
			}
		}
		xs << xml::StartTag(from_utf8("bibliomixed"), attr);

		// Generate the entry.
		par->simpleDocBookOnePar(buf, xs, runparams, text.outerFont(distance(begin, par)), true, true, 0);

		// End the precooked bibliography entry.
		xs << xml::EndTag("bibliomixed");
		xs << xml::CR();
	}

	// If this is the last paragraph in a bibliography, close the bibliography tag.
	if (par == end || par->layout().latextype != LATEX_BIB_ENVIRONMENT) {
		xs << xml::EndTag("bibliography");
		xs << xml::CR();
	}

	return pend;
}


ParagraphList::const_iterator makeParagraphs(
		Buffer const &buf,
		XMLStream &xs,
		OutputParams const &runparams,
		Text const &text,
		ParagraphList::const_iterator const & pbegin,
		ParagraphList::const_iterator const & pend)
{
	auto const begin = text.paragraphs().begin();
	auto const end = text.paragraphs().end();
	ParagraphList::const_iterator par = pbegin;
	ParagraphList::const_iterator prevpar = pbegin;

	for (; par != pend; prevpar = par, ++par) {
		// We want to open the paragraph tag if:
		//   (i) the current layout permits multiple paragraphs
		//  (ii) we are either not already inside a paragraph (HTMLIsBlock) OR
		//	   we are, but this is not the first paragraph
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
		Inset const *specinset = par->size() == 1 ? par->getInset(0) : nullptr;
		if (specinset && !specinset->getLayout().htmlisblock()) { // TODO: Convert htmlisblock to a DocBook parameter?
			Layout const &style = par->layout();
			FontInfo const first_font = style.labeltype == LABEL_MANUAL ?
										style.labelfont : style.font;
			FontInfo const our_font =
					par->getFont(buf.masterBuffer()->params(), 0,
								 text.outerFont(distance(begin, par))).fontInfo();

			if (first_font == our_font)
				special_case = true;
		}

		// Plain layouts must be ignored.
		if (!special_case && buf.params().documentClass().isPlainLayout(par->layout()) && !runparams.docbook_force_pars)
			special_case = true;
		// TODO: Could get rid of this with a DocBook equivalent to htmlisblock?
		if (!special_case && par->size() == 1 && par->getInset(0)) {
			Inset const * firstInset = par->getInset(0);

			// Floats cannot be in paragraphs.
			special_case = to_utf8(firstInset->layoutName()).substr(0, 6) == "Float:";

			// Bibliographies cannot be in paragraphs.
			if (!special_case && firstInset->asInsetCommand())
				special_case = firstInset->asInsetCommand()->params().getCmdName() == "bibtex";

			// Equations do not deserve their own paragraph (DocBook allows them outside paragraphs).
			if (!special_case && firstInset->asInsetMath())
				special_case = true;

			// ERTs are in comments, not paragraphs.
			if (!special_case && firstInset->lyxCode() == lyx::ERT_CODE)
				special_case = true;

			// Listings should not get into their own paragraph.
			if (!special_case && firstInset->lyxCode() == lyx::LISTINGS_CODE)
				special_case = true;
		}

		bool const open_par = runparams.docbook_make_pars
							  && (!runparams.docbook_in_par || par != pbegin)
							  && !special_case;

		// We want to issue the closing tag if either:
		//   (i)  We opened it, and either docbook_in_par is false,
		//		or we're not in the last paragraph, anyway.
		//   (ii) We didn't open it and docbook_in_par is true,
		//		but we are in the first par, and there is a next par.
		auto nextpar = par;
		++nextpar;
		bool const close_par =
				((open_par && (!runparams.docbook_in_par || nextpar != pend))
				|| (!open_par && runparams.docbook_in_par && par == pbegin && nextpar != pend));

		// Determine if this paragraph has some real content. Things like new pages are not caught
		// by Paragraph::empty(), even though they do not generate anything useful in DocBook.
		odocstringstream os2;
		XMLStream xs2(os2);
		par->simpleDocBookOnePar(buf, xs2, runparams, text.outerFont(distance(begin, par)), open_par, close_par, 0);

		docstring cleaned = os2.str();
		static const lyx::regex reg("[ \\r\\n]*");
		cleaned = from_utf8(lyx::regex_replace(to_utf8(cleaned), reg, string("")));

		if (!cleaned.empty()) {
			if (open_par)
				openParTag(xs, &*par, &*prevpar);

			xs << XMLStream::ESCAPE_NONE << os2.str();

			if (close_par) {
				closeTag(xs, &*par, (nextpar != end) ? &*nextpar : nullptr);
				xs << xml::CR();
			}
		}
	}
	return pend;
}


bool isNormalEnv(Layout const &lay)
{
	return lay.latextype == LATEX_ENVIRONMENT
		   || lay.latextype == LATEX_BIB_ENVIRONMENT;
}


ParagraphList::const_iterator makeEnvironment(
		Buffer const &buf,
		XMLStream &xs,
		OutputParams const &runparams,
		Text const &text,
		ParagraphList::const_iterator const & pbegin,
		ParagraphList::const_iterator const & pend)
{
	auto const begin = text.paragraphs().begin();
	auto const end = text.paragraphs().end();
	ParagraphList::const_iterator par = pbegin;
	depth_type const origdepth = pbegin->params().depth();

	// Output the opening tag for this environment.
	{
		// Find the previous paragraph.
		auto prevpar = begin;
		if (prevpar != par) {
			auto prevpar_next = prevpar;
			++prevpar_next;

			while (prevpar_next != par) {
				++prevpar_next;
				++prevpar;
			}
		}

		// Open tag for this environment.
		openParTag(xs, &*par, &*prevpar);
		xs << xml::CR();
	}

	// we will on occasion need to remember a layout from before.
	Layout const *lastlay = nullptr;
	auto prevpar = par;

	while (par != pend) {
		Layout const & style = par->layout();
		ParagraphList::const_iterator send;

		auto parnext = par;
		++parnext;

		// Actual content of this paragraph.
		prevpar = par;
		switch (style.latextype) {
		case LATEX_ENVIRONMENT:
		case LATEX_LIST_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			// There are two possibilities in this case.
			// One is that we are still in the environment in which we
			// started---which we will be if the depth is the same.
			if (par->params().depth() == origdepth) {
				LATTEST(par->layout() == style);
				if (lastlay != nullptr) {
					closeItemTag(xs, *lastlay);
					if (lastlay->docbookitemwrappertag() != "NONE") {
						xs << xml::EndTag(lastlay->docbookitemwrappertag());
						xs << xml::CR();
					}
					lastlay = nullptr;
				}

				// this will be positive if we want to skip the
				// initial word (if it's been taken for the label).
				pos_type sep = 0;

				// Open a wrapper tag if needed.
				if (style.docbookitemwrappertag() != "NONE") {
					xs << xml::StartTag(style.docbookitemwrappertag(), style.docbookitemwrapperattr());
					xs << xml::CR();
				}

				// label output
				if (style.labeltype != LABEL_NO_LABEL &&
						style.docbookitemlabeltag() != "NONE") {

					if (isNormalEnv(style)) {
						// in this case, we print the label only for the first
						// paragraph (as in a theorem or an abstract).
						if (par == pbegin) {
							docstring const lbl = pbegin->params().labelString();
							if (!lbl.empty()) {
								openLabelTag(xs, style);
								xs << lbl;
								closeLabelTag(xs, style);
							} else {
								// No new line after closeLabelTag.
								xs << xml::CR();
							}
						}
					} else { // some kind of list
						if (style.labeltype == LABEL_MANUAL) {
							// Only variablelist gets here.

							openLabelTag(xs, style);
							sep = par->firstWordDocBook(xs, runparams);
							closeLabelTag(xs, style);
						} else {
							openLabelTag(xs, style);
							xs << par->params().labelString();
							closeLabelTag(xs, style);
						}
					}
				} // end label output

				// Start generating the item.
				bool wasInParagraph = runparams.docbook_in_par;
				openItemTag(xs, style);
				bool getsIntoParagraph = openInnerItemTag(xs, style);
				OutputParams rp = runparams;
				rp.docbook_in_par = wasInParagraph | getsIntoParagraph;

				// Maybe the item is completely empty, i.e. if the first word ends at the end of the current paragraph
				// AND if the next paragraph doesn't have the same depth (if there is such a paragraph).
				// Common case: there is only the first word on the line, but there is a nested list instead
				// of more text.
				bool emptyItem = false;
				if (sep == par->size()) {
					auto next_par = par;
					++next_par;
					if (next_par == text.paragraphs().end()) // There is no next paragraph.
						emptyItem = true;
					else // There is a next paragraph: check depth.
						emptyItem = par->params().depth() >= next_par->params().depth();
				}

				if (emptyItem) {
					// Avoid having an empty item, this is not valid DocBook. A single character is enough to force
					// generation of a full <para>.
					xs << ' ';
				} else {
					// Generate the rest of the paragraph, if need be.
					par->simpleDocBookOnePar(buf, xs, rp, text.outerFont(distance(begin, par)), true, true, sep);
				}

				++par;
				if (getsIntoParagraph)
					closeInnerItemTag(xs, style);

				// We may not want to close the tag yet, in particular:
				// If we're not at the end of the item...
				if (par != pend
					//  and are doing items...
					&& !isNormalEnv(style)
					// and if the depth has changed...
					&& par->params().depth() != origdepth) {
					// then we'll save this layout for later, and close it when
					// we get another item.
					lastlay = &style;
				} else {
					closeItemTag(xs, style);

					// Eventually, close the item wrapper.
					if (style.docbookitemwrappertag() != "NONE") {
						xs << xml::EndTag(style.docbookitemwrappertag());
						xs << xml::CR();
					}
				}
			}
			// The other possibility is that the depth has increased.
			else {
				send = findEndOfEnvironment(par, pend);
				par = makeEnvironment(buf, xs, runparams, text, par, send);
			}
			break;
		}
		case LATEX_PARAGRAPH:
//			send = findLast(par, pend, LATEX_PARAGRAPH);
			par = makeParagraphs(buf, xs, runparams, text, par, parnext);
			break;
		case LATEX_BIB_ENVIRONMENT:
//			send = findLast(par, pend, LATEX_BIB_ENVIRONMENT);
			makeParagraphBibliography(buf, xs, runparams, text, par, parnext);
			break;
		case LATEX_COMMAND:
			par = parnext;
			break;
		}
	}

	if (lastlay != nullptr) {
		closeItemTag(xs, *lastlay);
		if (lastlay->docbookitemwrappertag() != "NONE") {
			xs << xml::EndTag(lastlay->docbookitemwrappertag());
			xs << xml::CR();
		}
	}
//	auto nextpar = par;
//	++nextpar;
	closeTag(xs, &*prevpar, &*par);
//	closeTag(xs, &*par, (nextpar != end) ? &*nextpar : nullptr);
	xs << xml::CR();
	return pend;
}


void makeCommand(
		Buffer const & buf,
		XMLStream & xs,
		OutputParams const & runparams,
		Text const & text,
		ParagraphList::const_iterator const & pbegin)
{
	// No need for labels, as they are handled by DocBook tags.
	auto const begin = text.paragraphs().begin();
	auto const end = text.paragraphs().end();
	auto nextpar = pbegin;
	++nextpar;

	// Find the previous paragraph.
	auto prevpar = begin;
	if (prevpar != pbegin) {
		auto prevpar_next = prevpar;
		++prevpar_next;

		while (prevpar_next != pbegin) {
			++prevpar_next;
			++prevpar;
		}
	}

	// Generate this command.
	openParTag(xs, &*pbegin, &*prevpar);

	pbegin->simpleDocBookOnePar(buf, xs, runparams,
								text.outerFont(distance(begin, pbegin)));

	closeTag(xs, &*pbegin, (nextpar != end) ? &*nextpar : nullptr);
	xs << xml::CR();
}

pair<ParagraphList::const_iterator, ParagraphList::const_iterator> makeAny(
		Text const &text,
		Buffer const &buf,
		XMLStream &xs,
		OutputParams const &ourparams,
		ParagraphList::const_iterator par,
		ParagraphList::const_iterator send,
		ParagraphList::const_iterator pend)
{
	switch (par->layout().latextype) {
	case LATEX_COMMAND: {
		// The files with which we are working never have more than
		// one paragraph in a command structure.
		// FIXME
		// if (ourparams.docbook_in_par)
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
	case LATEX_ITEM_ENVIRONMENT:
		// FIXME Same fix here.
		send = findEndOfEnvironment(par, pend);
		par = makeEnvironment(buf, xs, ourparams, text, par, send);
		break;
	case LATEX_PARAGRAPH:
		send = findLast(par, pend, LATEX_PARAGRAPH);
		par = makeParagraphs(buf, xs, ourparams, text, par, send);
		break;
	case LATEX_BIB_ENVIRONMENT:
		send = findLast(par, pend, LATEX_BIB_ENVIRONMENT);
		par = makeParagraphBibliography(buf, xs, ourparams, text, par, send);
		break;
	}

	return make_pair(par, send);
}

ParagraphList::const_iterator makeAnySimple(
		Text const &text,
		Buffer const &buf,
		XMLStream &xs,
		OutputParams const &ourparams,
		ParagraphList::const_iterator par)
{
	auto parnext = par;
	++parnext;

	switch (par->layout().latextype) {
	case LATEX_COMMAND: {
		// The files with which we are working never have more than
		// one paragraph in a command structure.
		// FIXME
		// if (ourparams.docbook_in_par)
		//   fix it so we don't get sections inside standard, e.g.
		// note that we may then need to make runparams not const, so we
		// can communicate that back.
		// FIXME Maybe this fix should be in the routines themselves, in case
		// they are called from elsewhere.
		makeCommand(buf, xs, ourparams, text, par);
		return parnext;
	}
	case LATEX_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
	case LATEX_ITEM_ENVIRONMENT:
		// FIXME Same fix here.
		return makeEnvironment(buf, xs, ourparams, text, par, parnext);
	case LATEX_PARAGRAPH:
		return makeParagraphs(buf, xs, ourparams, text, par, parnext);
	case LATEX_BIB_ENVIRONMENT:
		return makeParagraphBibliography(buf, xs, ourparams, text, par, parnext);
	}

	// This should never happen. Return the next paragraph to avoid an infinite loop.
	return parnext;
}

} // end anonymous namespace


using DocBookDocumentSectioning = tuple<bool, pit_type>;


struct DocBookInfoTag
{
	const set<pit_type> shouldBeInInfo;
	const set<pit_type> mustBeInInfo;
	const set<pit_type> abstract;
	pit_type bpit;
	pit_type epit;

	DocBookInfoTag(const set<pit_type> & shouldBeInInfo, const set<pit_type> & mustBeInInfo,
				   const set<pit_type> & abstract, pit_type bpit, pit_type epit) :
				   shouldBeInInfo(shouldBeInInfo), mustBeInInfo(mustBeInInfo), abstract(abstract),
				   bpit(bpit), epit(epit) {}
};


DocBookDocumentSectioning hasDocumentSectioning(ParagraphList const &paragraphs, pit_type bpit, pit_type const epit) {
	bool documentHasSections = false;

	while (bpit < epit) {
		Layout const &style = paragraphs[bpit].layout();
		documentHasSections |= style.category() == from_utf8("Sectioning");

		if (documentHasSections) {
			break;
		}
		bpit += 1;
	}
	// Paragraphs before the first section: [ runparams.par_begin ; eppit )

	return make_tuple(documentHasSections, bpit);
}


bool hasOnlyNotes(Paragraph const & par)
{
	for (int i = 0; i < par.size(); ++i)
		if (!par.isInset(i) || !dynamic_cast<InsetNote *>(par.insetList().get(i)))
			return false;
	return true;
}


DocBookInfoTag getParagraphsWithInfo(ParagraphList const &paragraphs, pit_type bpit, pit_type const epit) {
	set<pit_type> shouldBeInInfo;
	set<pit_type> mustBeInInfo;
	set<pit_type> abstract;

	// Find the first non empty paragraph by mutating bpit.
	while (bpit < epit) {
		Paragraph const &par = paragraphs[bpit];
		if (par.empty() || hasOnlyNotes(par))
			bpit += 1;
		else
			break;
	}

	// Find the last info-like paragraph.
	pit_type cpit = bpit;
	bool hasAbstractLayout = false;
	while (cpit < epit) {
		// Skip paragraphs only containing one note.
		Paragraph const & par = paragraphs[cpit];
		if (hasOnlyNotes(par)) {
			cpit += 1;
			continue;
		}

		if (par.layout().docbookabstract())
			hasAbstractLayout = true;

		// Based on layout information, store this paragraph in one set: should be in <info>, must be.
		Layout const &style = par.layout();

		if (style.docbookininfo() == "always") {
			mustBeInInfo.emplace(cpit);
		} else if (style.docbookininfo() == "maybe") {
			shouldBeInInfo.emplace(cpit);
		} else {
			// Hypothesis: the <info> parts should be grouped together near the beginning bpit.
			// There may be notes in between, but nothing else.
			break;
		}
		cpit += 1;
	}
	// Now, cpit points to the last paragraph that has things that could go in <info>.
	// bpit is the beginning of the <info> part.

	// Go once again through the list of paragraphs to find the abstract. If there is an abstract
	// layout, only consider it. Otherwise, an abstract is just a sequence of paragraphs with text.
	if (hasAbstractLayout) {
		pit_type pit = bpit;
		while (pit < cpit) { // Don't overshoot the <info> part.
			if (paragraphs[pit].layout().docbookabstract())
				abstract.emplace(pit);
			pit++;
		}
	} else {
		pit_type lastAbstract = epit + 1; // A nonsensical value.
		docstring lastAbstractLayout;

		pit_type pit = bpit;
		while (pit < cpit) { // Don't overshoot the <info> part.
			const Paragraph & par = paragraphs.at(pit);
			if (!par.insetList().empty()) {
				for (const auto &i : par.insetList()) {
					if (i.inset->getText(0) != nullptr) {
						if (lastAbstract == epit + 1) {
							// First paragraph that matches the heuristic definition of abstract.
							lastAbstract = pit;
							lastAbstractLayout = par.layout().name();
						} else if (pit > lastAbstract + 1 || par.layout().name() != lastAbstractLayout) {
							// This is either too far from the last abstract paragraph or doesn't
							// have the right layout name, BUT there has already been an abstract
							// in this document: done with detecting the abstract.
							goto done; // Easier to get out of two nested loops.
						}

						abstract.emplace(pit);
						break;
					}
				}
			}
			pit++;
		}
	}

	done:
	return DocBookInfoTag(shouldBeInInfo, mustBeInInfo, abstract, bpit, cpit);
}


pit_type generateDocBookParagraphWithoutSectioning(
		Text const & text,
		Buffer const & buf,
		XMLStream & xs,
		OutputParams const & runparams,
		ParagraphList const & paragraphs,
		DocBookInfoTag const & info)
{
	auto bpit = info.bpit;
	auto par = paragraphs.iterator_at(bpit);
	auto lastStartedPar = par;
	ParagraphList::const_iterator send;
	auto const pend =
			(info.epit == (int) paragraphs.size()) ?
			paragraphs.end() : paragraphs.iterator_at(info.epit);

	while (bpit < info.epit) {
		if (info.abstract.find(bpit) != info.abstract.end()) {
			bpit += 1;
			continue;
		}

		tie(par, send) = makeAny(text, buf, xs, runparams, par, send, pend);
		bpit += distance(lastStartedPar, par);
		lastStartedPar = par;
	}

	return bpit;
}


void outputDocBookInfo(
		Text const & text,
		Buffer const & buf,
		XMLStream & xs,
		OutputParams const & runparams,
		ParagraphList const & paragraphs,
		DocBookInfoTag const & info)
{
	// Perform an additional check on the abstract. Sometimes, there are many paragraphs that should go
	// into the abstract, but none generates actual content. Thus, first generate to a temporary stream,
	// then only create the <abstract> tag if these paragraphs generate some content.
	// This check must be performed *before* a decision on whether or not to output <info> is made.
	bool hasAbstract = !info.abstract.empty();
	docstring abstract;
	if (hasAbstract) {
		odocstringstream os2;
		{
			XMLStream xs2(os2);
			auto bpit = *std::min_element(info.abstract.begin(), info.abstract.end());
			auto epit = 1 + *std::max_element(info.abstract.begin(), info.abstract.end());
			// info.abstract is inclusive, epit is exclusive, hence +1 for looping.

			while (bpit < epit) {
				makeAnySimple(text, buf, xs2, runparams, paragraphs.iterator_at(bpit));
				xs2 << XMLStream::ESCAPE_NONE << from_ascii("<!-- " + to_string(bpit) + " -->");
				bpit += 1;
			}
		}

		// Actually output the abstract if there is something to do. Don't count line feeds or spaces in this,
		// even though they must be properly output if there is some abstract.
		abstract = os2.str();
		static const lyx::regex reg("[ \\r\\n]*");
		docstring abstractContent = from_utf8(lyx::regex_replace(to_utf8(abstract), reg, string("")));

		// Nothing? Then there is no abstract!
		if (abstractContent.empty())
			hasAbstract = false;
	}

	// The abstract must go in <info>.
	bool needInfo = !info.mustBeInInfo.empty() || hasAbstract;

	// Start the <info> tag if required.
	if (needInfo) {
		xs.startDivision(false);
		xs << xml::StartTag("info");
		xs << xml::CR();
	}

	// Output the elements that should go in <info>, before and after the abstract.
	xs << XMLStream::ESCAPE_NONE << "<!-- shouldBeInInfo -->";
	for (auto pit : info.shouldBeInInfo) // Typically, the title: these elements are so important and ubiquitous
		// that mandating a wrapper like <info> would repel users.
		makeAnySimple(text, buf, xs, runparams, paragraphs.iterator_at(pit));
	xs << XMLStream::ESCAPE_NONE << "<!-- mustBeInInfo -->";
	for (auto pit : info.mustBeInInfo)
		if (info.abstract.find(pit) == info.abstract.end()) // The abstract must be in info, but is dealt with after.
			makeAnySimple(text, buf, xs, runparams, paragraphs.iterator_at(pit));
	xs << XMLStream::ESCAPE_NONE << "<!-- /info -->";

	if (hasAbstract) {
//		string tag = paragraphs[*info.abstract.begin()].layout().docbookforceabstracttag();
//		if (tag == "NONE")
//			tag = "abstract";
//
//		xs << xml::StartTag(tag);
//		xs << xml::CR();
		xs << XMLStream::ESCAPE_NONE << "<!-- abs -->";
		xs << XMLStream::ESCAPE_NONE << abstract;
		xs << XMLStream::ESCAPE_NONE << "<!-- /abs -->";
//		xs << xml::EndTag(tag);
//		xs << xml::CR();
	}

	// End the <info> tag if it was started.
	if (needInfo) {
		xs << xml::EndTag("info");
		xs << xml::CR();
		xs.endDivision();
	}
}


void docbookFirstParagraphs(
		Text const &text,
		Buffer const &buf,
		XMLStream &xs,
		OutputParams const &runparams,
		pit_type epit)
{
	// Handle the beginning of the document, supposing it has sections.
	// Major role: output the first <info> tag.

	ParagraphList const &paragraphs = text.paragraphs();
	pit_type bpit = runparams.par_begin;
	DocBookInfoTag info = getParagraphsWithInfo(paragraphs, bpit, epit);
	outputDocBookInfo(text, buf, xs, runparams, paragraphs, info);
}


void docbookSimpleAllParagraphs(
		Text const & text,
		Buffer const & buf,
		XMLStream & xs,
		OutputParams const & runparams)
{
	// Handle the given text, supposing it has no sections (i.e. a "simple" text). The input may vary in length
	// between a single paragraph to a whole document.

	// First, the <info> tag.
	ParagraphList const &paragraphs = text.paragraphs();
	pit_type bpit = runparams.par_begin;
	pit_type const epit = runparams.par_end;
	DocBookInfoTag info = getParagraphsWithInfo(paragraphs, bpit, epit);
	outputDocBookInfo(text, buf, xs, runparams, paragraphs, info);

	// Then, the content. It starts where the <info> ends.
	bpit = info.epit;
	while (bpit < epit) {
		auto par = paragraphs.iterator_at(bpit);
		if (!hasOnlyNotes(*par))
			makeAnySimple(text, buf, xs, runparams, par);
		bpit += 1;
	}
}


void docbookParagraphs(Text const &text,
					   Buffer const &buf,
					   XMLStream &xs,
					   OutputParams const &runparams) {
	ParagraphList const &paragraphs = text.paragraphs();
	if (runparams.par_begin == runparams.par_end) {
		runparams.par_begin = 0;
		runparams.par_end = paragraphs.size();
	}
	pit_type bpit = runparams.par_begin;
	pit_type const epit = runparams.par_end;
	LASSERT(bpit < epit,
			{
				xs << XMLStream::ESCAPE_NONE << "<!-- DocBook output error! -->\n";
				return;
			});

	ParagraphList::const_iterator const pend =
			(epit == (int) paragraphs.size()) ?
			paragraphs.end() : paragraphs.iterator_at(epit);
	std::stack<std::pair<int, string>> headerLevels; // Used to determine when to open/close sections: store the depth
	// of the section and the tag that was used to open it.

	// Detect whether the document contains sections. If there are no sections, there can be no automatically
	// discovered abstract.
	bool documentHasSections;
	pit_type eppit;
	tie(documentHasSections, eppit) = hasDocumentSectioning(paragraphs, bpit, epit);

	if (documentHasSections) {
		docbookFirstParagraphs(text, buf, xs, runparams, eppit);
		bpit = eppit;
	} else {
		docbookSimpleAllParagraphs(text, buf, xs, runparams);
		return;
	}

	bool currentlyInAppendix = false;

	while (bpit < epit) {
		OutputParams ourparams = runparams;

		auto par = paragraphs.iterator_at(bpit);
		if (par->params().startOfAppendix())
			currentlyInAppendix = true;
		Layout const &style = par->layout();
		ParagraphList::const_iterator const lastStartedPar = par;
		ParagraphList::const_iterator send;

		if (hasOnlyNotes(*par)) {
			++par;
			bpit += distance(lastStartedPar, par);
			continue;
		}

		// Think about adding <section> and/or </section>s.
		const bool isLayoutSectioning = style.category() == from_utf8("Sectioning");
		if (isLayoutSectioning) {
			int level = style.toclevel;

			// Need to close a previous section if it has the same level or a higher one (close <section> if opening a <h2>
			// after a <h2>, <h3>, <h4>, <h5> or <h6>). More examples:
			//   - current: h2; back: h1; do not close any <section>
			//   - current: h1; back: h2; close two <section> (first the <h2>, then the <h1>, so a new <h1> can come)
			while (!headerLevels.empty() && level <= headerLevels.top().first) {
				int stackLevel = headerLevels.top().first;
				docstring stackTag = from_utf8("</" + headerLevels.top().second + ">");
				headerLevels.pop();

				// Output the tag only if it corresponds to a legit section.
				if (stackLevel != Layout::NOT_IN_TOC)
					xs << XMLStream::ESCAPE_NONE << stackTag << xml::CR();
			}

			// Open the new section: first push it onto the stack, then output it in DocBook.
			string sectionTag = (currentlyInAppendix && style.docbooksectiontag() == "chapter") ?
								"appendix" : style.docbooksectiontag();
			headerLevels.push(std::make_pair(level, sectionTag));

			// Some sectioning-like elements should not be output (such as FrontMatter).
			if (level != Layout::NOT_IN_TOC) {
				// Look for a label in the title, i.e. a InsetLabel as a child.
				docstring id = docstring();
				for (pos_type i = 0; i < par->size(); ++i) {
					Inset const *inset = par->getInset(i);
					if (inset) {
						if (auto label = dynamic_cast<InsetLabel const *>(inset)) {
							// Generate the attributes for the section if need be.
							id += "xml:id=\"" + xml::cleanID(label->screenLabel()) + "\"";

							// Don't output the ID as a DocBook <anchor>.
							ourparams.docbook_anchors_to_ignore.emplace(label->screenLabel());

							// Cannot have multiple IDs per tag.
							break;
						}
					}
				}

				// Write the open tag for this section.
				docstring tag = from_utf8("<" + sectionTag);
				if (!id.empty())
					tag += from_utf8(" ") + id;
				tag += from_utf8(">");
				xs << XMLStream::ESCAPE_NONE << tag;
				xs << xml::CR();
			}
		}

		// Close all sections before the bibliography.
		// TODO: Only close all when the bibliography is at the end of the document? Or force to output the bibliography at the end of the document? Or don't care (as allowed by DocBook)?
		auto insetsLength = distance(par->insetList().begin(), par->insetList().end());
		if (insetsLength > 0) {
			Inset const *firstInset = par->getInset(0);
			if (firstInset && dynamic_cast<InsetBibtex const *>(firstInset)) {
				while (!headerLevels.empty()) {
					int level = headerLevels.top().first;
					docstring tag = from_utf8("</" + headerLevels.top().second + ">");
					headerLevels.pop();

					// Output the tag only if it corresponds to a legit section.
					if (level != Layout::NOT_IN_TOC) {
						xs << XMLStream::ESCAPE_NONE << tag;
						xs << xml::CR();
					}
				}
			}
		}

		// Generate this paragraph.
		tie(par, send) = makeAny(text, buf, xs, ourparams, par, send, pend);
		bpit += distance(lastStartedPar, par);
	}

	// If need be, close <section>s, but only at the end of the document (otherwise, dealt with at the beginning
	// of the loop).
	while (!headerLevels.empty() && headerLevels.top().first > Layout::NOT_IN_TOC) {
		docstring tag = from_utf8("</" + headerLevels.top().second + ">");
		headerLevels.pop();
		xs << XMLStream::ESCAPE_NONE << tag;
		xs << xml::CR();
	}
}

} // namespace lyx