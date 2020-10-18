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
#include "output_docbook.h"
#include "Paragraph.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "xml.h"
#include "Text.h"
#include "TextClass.h"

#include "insets/InsetBibtex.h"
#include "insets/InsetBibitem.h"
#include "insets/InsetLabel.h"
#include "mathed/InsetMath.h"
#include "insets/InsetNote.h"

#include "support/lassert.h"
#include "support/textutils.h"

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
		return "personname";
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


// Convenience functions to open and close tags. First, very low-level ones to ensure a consistent new-line behaviour.
// Block style:
//	  Content before
//	  <blocktag>
//	    Contents of the block.
//	  </blocktag>
//	  Content after
// Paragraph style:
//	  Content before
//	    <paratag>Contents of the paragraph.</paratag>
//	  Content after
// Inline style:
//    Content before<inlinetag>Contents of the paragraph.</inlinetag>Content after

void openInlineTag(XMLStream & xs, const std::string & tag, const std::string & attr)
{
	xs << xml::StartTag(tag, attr);
}


void closeInlineTag(XMLStream & xs, const std::string & tag)
{
	xs << xml::EndTag(tag);
}


void openParTag(XMLStream & xs, const std::string & tag, const std::string & attr)
{
	if (!xs.isLastTagCR())
		xs << xml::CR();
	xs << xml::StartTag(tag, attr);
}


void closeParTag(XMLStream & xs, const std::string & tag)
{
	xs << xml::EndTag(tag);
	xs << xml::CR();
}


void openBlockTag(XMLStream & xs, const std::string & tag, const std::string & attr)
{
	if (!xs.isLastTagCR())
		xs << xml::CR();
	xs << xml::StartTag(tag, attr);
	xs << xml::CR();
}


void closeBlockTag(XMLStream & xs, const std::string & tag)
{
	if (!xs.isLastTagCR())
		xs << xml::CR();
	xs << xml::EndTag(tag);
	xs << xml::CR();
}


void openTag(XMLStream & xs, const std::string & tag, const std::string & attr, const std::string & tagtype)
{
	if (tag.empty() || tag == "NONE") // Common check to be performed elsewhere, if it was not here.
		return;

	if (tag == "para" || tagtype == "paragraph") // Special case for <para>: always considered as a paragraph.
		openParTag(xs, tag, attr);
	else if (tagtype == "block")
		openBlockTag(xs, tag, attr);
	else if (tagtype == "inline")
		openInlineTag(xs, tag, attr);
	else
		xs.writeError("Unrecognised tag type '" + tagtype + "' for '" + tag + " " + attr + "'");
}


void closeTag(XMLStream & xs, const std::string & tag, const std::string & tagtype)
{
	if (tag.empty() || tag == "NONE")
		return;

	if (tag == "para" || tagtype == "paragraph") // Special case for <para>: always considered as a paragraph.
		closeParTag(xs, tag);
	else if (tagtype == "block")
		closeBlockTag(xs, tag);
	else if (tagtype == "inline")
		closeInlineTag(xs, tag);
	else
		xs.writeError("Unrecognised tag type '" + tagtype + "' for '" + tag + "'");
}


void compTag(XMLStream & xs, const std::string & tag, const std::string & attr, const std::string & tagtype)
{
	if (tag.empty() || tag == "NONE")
		return;

	// Special case for <para>: always considered as a paragraph.
	if (tag == "para" || tagtype == "paragraph" || tagtype == "block") {
		if (!xs.isLastTagCR())
			xs << xml::CR();
		xs << xml::CompTag(tag, attr);
		xs << xml::CR();
	} else if (tagtype == "inline") {
		xs << xml::CompTag(tag, attr);
	} else {
		xs.writeError("Unrecognised tag type '" + tagtype + "' for '" + tag + "'");
	}
}


// Higher-level convenience functions.

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
			if (prevlay.docbookwrappertag() == lay.docbookwrappertag() &&
					prevlay.docbookwrapperattr() == lay.docbookwrapperattr())
				openWrapper = !lay.docbookwrappermergewithprevious();
			else
				openWrapper = true;
		}
	}

	// Main logic.
	if (openWrapper)
		openTag(xs, lay.docbookwrappertag(), lay.docbookwrapperattr(), lay.docbookwrappertagtype());

	const string & tag = lay.docbooktag();
	if (tag != "NONE") {
		auto xmltag = xml::ParTag(tag, lay.docbookattr());
		if (!xs.isTagOpen(xmltag, 1)) { // Don't nest a paragraph directly in a paragraph.
			// TODO: required or not?
			// TODO: avoid creating a ParTag object just for this query...
			openTag(xs, lay.docbooktag(), lay.docbookattr(), lay.docbooktagtype());
			openTag(xs, lay.docbookinnertag(), lay.docbookinnerattr(), lay.docbookinnertagtype());
		}
	}

	openTag(xs, lay.docbookitemtag(), lay.docbookitemattr(), lay.docbookitemtagtype());
	openTag(xs, lay.docbookiteminnertag(), lay.docbookiteminnerattr(), lay.docbookiteminnertagtype());
}


void closeParTag(XMLStream & xs, Paragraph const * par, Paragraph const * nextpar)
{
	if (par == nextpar)
		nextpar = nullptr;

	// See comment in openParTag.
	Layout const & lay = par->layout();
	bool closeWrapper = lay.docbookwrappertag() != "NONE";
	if (nextpar != nullptr) {
		Layout const & nextlay = nextpar->layout();
		if (nextlay.docbookwrappertag() != "NONE") {
			if (nextlay.docbookwrappertag() == lay.docbookwrappertag() &&
					nextlay.docbookwrapperattr() == lay.docbookwrapperattr())
				closeWrapper = !nextlay.docbookwrappermergewithprevious();
			else
				closeWrapper = true;
		}
	}

	// Main logic.
	closeTag(xs, lay.docbookiteminnertag(), lay.docbookiteminnertagtype());
	closeTag(xs, lay.docbookitemtag(), lay.docbookitemtagtype());
	closeTag(xs, lay.docbookinnertag(), lay.docbookinnertagtype());
	closeTag(xs, lay.docbooktag(), lay.docbooktagtype());
	if (closeWrapper)
		closeTag(xs, lay.docbookwrappertag(), lay.docbookwrappertagtype());
}


void makeBibliography(
		Text const & text,
		Buffer const & buf,
		XMLStream & xs,
		OutputParams const & runparams,
		ParagraphList::const_iterator const & par)
{
	// If this is the first paragraph in a bibliography, open the bibliography tag.
	auto const * pbegin_before = text.paragraphs().getParagraphBefore(par);
	if (pbegin_before == nullptr || (pbegin_before && pbegin_before->layout().latextype != LATEX_BIB_ENVIRONMENT)) {
		xs << xml::StartTag("bibliography");
		xs << xml::CR();
	}

	// Start the precooked bibliography entry. This is very much like opening a paragraph tag.
	// Don't forget the citation ID!
	docstring attr;
	for (auto i = 0; i < par->size(); ++i) {
		Inset const *ip = par->getInset(i);
		if (!ip)
			continue;
		if (const auto * bibitem = dynamic_cast<const InsetBibitem*>(ip)) {
			attr = from_utf8("xml:id='") + bibitem->getParam("key") + from_utf8("'");
			break;
		}
	}
	xs << xml::StartTag(from_utf8("bibliomixed"), attr);

	// Generate the entry. Concatenate the different parts of the paragraph if any.
	auto const begin = text.paragraphs().begin();
	auto pars = par->simpleDocBookOnePar(buf, runparams, text.outerFont(std::distance(begin, par)), 0);
	for (auto & parXML : pars)
		xs << XMLStream::ESCAPE_NONE << parXML;

	// End the precooked bibliography entry.
	xs << xml::EndTag("bibliomixed");
	xs << xml::CR();

	// If this is the last paragraph in a bibliography, close the bibliography tag.
	auto const end = text.paragraphs().end();
	auto nextpar = par;
	++nextpar;
	bool endBibliography = nextpar == end || nextpar->layout().latextype != LATEX_BIB_ENVIRONMENT;

	if (endBibliography) {
		xs << xml::EndTag("bibliography");
		xs << xml::CR();
	}
}


void makeParagraph(
		Text const & text,
		Buffer const & buf,
		XMLStream & xs,
		OutputParams const & runparams,
		ParagraphList::const_iterator const & par)
{
	auto const begin = text.paragraphs().begin();
	auto const end = text.paragraphs().end();
	auto prevpar = text.paragraphs().getParagraphBefore(par);

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
							 text.outerFont(std::distance(begin, par))).fontInfo();

		if (first_font == our_font)
			special_case = true;
	}

	size_t nInsets = std::distance(par->insetList().begin(), par->insetList().end());

	// Plain layouts must be ignored.
	special_case |= buf.params().documentClass().isPlainLayout(par->layout()) && !runparams.docbook_force_pars;
	// Equations do not deserve their own paragraph (DocBook allows them outside paragraphs).
	// Exception: any case that generates an <inlineequation> must still get a paragraph to be valid.
	special_case |= nInsets == (size_t) par->size() && std::all_of(par->insetList().begin(), par->insetList().end(), [](InsetList::Element inset) {
		return inset.inset && inset.inset->asInsetMath() && inset.inset->asInsetMath()->getType() != hullSimple;
	});

	// TODO: Could get rid of this with a DocBook equivalent to htmlisblock? Not for all cases, unfortunately... See above for those that have been determined not to be allowable for this potential refactoring.
	if (!special_case && par->size() == 1 && par->getInset(0)) {
		Inset const * firstInset = par->getInset(0);

		// Floats cannot be in paragraphs.
		special_case = to_utf8(firstInset->layoutName()).substr(0, 6) == "Float:";

		// Bibliographies cannot be in paragraphs.
		if (!special_case && firstInset->asInsetCommand())
			special_case = firstInset->asInsetCommand()->params().getCmdName() == "bibtex";

		// ERTs are in comments, not paragraphs.
		if (!special_case && firstInset->lyxCode() == lyx::ERT_CODE)
			special_case = true;

		// Listings should not get into their own paragraph.
		if (!special_case && firstInset->lyxCode() == lyx::LISTINGS_CODE)
			special_case = true;

		// Boxes cannot get into their own paragraph.
		if (!special_case && firstInset->lyxCode() == lyx::BOX_CODE)
			special_case = true;
	}

	bool const open_par = runparams.docbook_make_pars
						  && !runparams.docbook_in_par
						  && !special_case;

	// We want to issue the closing tag if either:
	//   (i)  We opened it, and either docbook_in_par is false,
	//		or we're not in the last paragraph, anyway.
	//   (ii) We didn't open it and docbook_in_par is true,
	//		but we are in the first par, and there is a next par.
	bool const close_par = open_par && (!runparams.docbook_in_par);

	// Determine if this paragraph has some real content. Things like new pages are not caught
	// by Paragraph::empty(), even though they do not generate anything useful in DocBook.
	// Thus, remove all spaces (including new lines: \r, \n) before checking for emptiness.
	// std::all_of allows doing this check without having to copy the string.
	// Open and close tags around each contained paragraph.
	auto nextpar = par;
	++nextpar;
	auto pars = par->simpleDocBookOnePar(buf, runparams, text.outerFont(distance(begin, par)), 0, nextpar == end, special_case);
	for (docstring const & parXML : pars) {
		if (xml::isNotOnlySpace(parXML)) {
			if (open_par)
				openParTag(xs, &*par, prevpar);

			xs << XMLStream::ESCAPE_NONE << parXML;

			if (close_par)
				closeParTag(xs, &*par, (nextpar != end) ? &*nextpar : nullptr);
		}
	}
}


void makeEnvironment(Text const &text,
					 Buffer const &buf,
                     XMLStream &xs,
                     OutputParams const &runparams,
                     ParagraphList::const_iterator const & par)
{
	auto const end = text.paragraphs().end();
	auto nextpar = par;
	++nextpar;

	// Special cases for listing-like environments provided in layouts. This is quite ad-hoc, but provides a useful
	// default. This should not be used by too many environments (only LyX-Code right now).
	// This would be much simpler if LyX-Code was implemented as InsetListings...
	bool mimicListing = false;
	bool ignoreFonts = false;
	if (par->layout().docbooktag() == "programlisting") {
		mimicListing = true;
		ignoreFonts = true;
	}

	// Output the opening tag for this environment, but only if it has not been previously opened (condition
	// implemented in openParTag).
	auto prevpar = text.paragraphs().getParagraphBefore(par);
	openParTag(xs, &*par, prevpar); // TODO: switch in layout for par/block?

	// Generate the contents of this environment. There is a special case if this is like some environment.
	Layout const & style = par->layout();
	if (style.latextype == LATEX_COMMAND) {
		// Nothing to do (otherwise, infinite loops).
	} else if (style.latextype == LATEX_ENVIRONMENT) {
		// Generate the paragraph, if need be.
		auto pars = par->simpleDocBookOnePar(buf, runparams, text.outerFont(std::distance(text.paragraphs().begin(), par)), 0, false, ignoreFonts);

		if (mimicListing) {
			auto p = pars.begin();
			while (p != pars.end()) {
				openTag(xs, par->layout().docbookiteminnertag(), par->layout().docbookiteminnerattr(), par->layout().docbookiteminnertagtype());
				xs << XMLStream::ESCAPE_NONE << *p;
				closeTag(xs, par->layout().docbookiteminnertag(), par->layout().docbookiteminnertagtype());
				++p;

				if (p != pars.end())
					xs << xml::CR();
			}
		} else {
			for (auto const & p : pars) {
				openTag(xs, par->layout().docbookiteminnertag(), par->layout().docbookiteminnerattr(), par->layout().docbookiteminnertagtype());
				xs << XMLStream::ESCAPE_NONE << p;
				closeTag(xs, par->layout().docbookiteminnertag(), par->layout().docbookiteminnertagtype());
			}
		}
	} else {
		makeAny(text, buf, xs, runparams, par);
	}

	// Close the environment.
	closeParTag(xs, &*par, (nextpar != end) ? &*nextpar : nullptr); // TODO: switch in layout for par/block?
}


ParagraphList::const_iterator findEndOfEnvironment(
		ParagraphList::const_iterator const & pstart,
		ParagraphList::const_iterator const & pend)
{
	// Copy-paste from XHTML. Should be factored out at some point...
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


ParagraphList::const_iterator makeListEnvironment(Text const &text,
												  Buffer const &buf,
		                                          XMLStream &xs,
		                                          OutputParams const &runparams,
		                                          ParagraphList::const_iterator const & begin)
{
	auto par = begin;
	auto const end = text.paragraphs().end();
	auto const envend = findEndOfEnvironment(par, end);

	// Output the opening tag for this environment.
	Layout const & envstyle = par->layout();
	openTag(xs, envstyle.docbookwrappertag(), envstyle.docbookwrapperattr(), envstyle.docbookwrappertagtype());
	openTag(xs, envstyle.docbooktag(), envstyle.docbookattr(), envstyle.docbooktagtype());

	// Handle the content of the list environment, item by item.
	while (par != envend) {
		Layout const & style = par->layout();

		// Open the item wrapper.
		openTag(xs, style.docbookitemwrappertag(), style.docbookitemwrapperattr(), style.docbookitemwrappertagtype());

		// Generate the label, if need be. If it is taken from the text, sep != 0 and corresponds to the first
		// character after the label.
		pos_type sep = 0;
		if (style.labeltype != LABEL_NO_LABEL && style.docbookitemlabeltag() != "NONE") {
			if (style.labeltype == LABEL_MANUAL) {
				// Only variablelist gets here (or similar items defined as an extension in the layout).
				openTag(xs, style.docbookitemlabeltag(), style.docbookitemlabelattr(), style.docbookitemlabeltagtype());
				sep = 1 + par->firstWordDocBook(xs, runparams);
				closeTag(xs, style.docbookitemlabeltag(), style.docbookitemlabeltagtype());
			} else {
				// Usual cases: maybe there is something specified at the layout level. Highly unlikely, though.
				docstring const lbl = par->params().labelString();

				if (!lbl.empty()) {
					openTag(xs, style.docbookitemlabeltag(), style.docbookitemlabelattr(), style.docbookitemlabeltagtype());
					xs << lbl;
					closeTag(xs, style.docbookitemlabeltag(), style.docbookitemlabeltagtype());
				}
			}
		}

		// Open the item (after the wrapper and the label).
		openTag(xs, style.docbookitemtag(), style.docbookitemattr(), style.docbookitemtagtype());

		// Generate the content of the item.
		if (sep < par->size()) {
			auto pars = par->simpleDocBookOnePar(buf, runparams,
			                                     text.outerFont(std::distance(text.paragraphs().begin(), par)), sep);
			for (auto &p : pars) {
				openTag(xs, par->layout().docbookiteminnertag(), par->layout().docbookiteminnerattr(),
				        par->layout().docbookiteminnertagtype());
				xs << XMLStream::ESCAPE_NONE << p;
				closeTag(xs, par->layout().docbookiteminnertag(), par->layout().docbookiteminnertagtype());
			}
		} else {
			// DocBook doesn't like emptiness.
			compTag(xs, par->layout().docbookiteminnertag(), par->layout().docbookiteminnerattr(),
			        par->layout().docbookiteminnertagtype());
		}

		// If the next item is deeper, it must go entirely within this item (do it recursively).
		// By construction, with findEndOfEnvironment, depth can only stay constant or increase, never decrease.
		depth_type currentDepth = par->getDepth();
		++par;
		while (par != envend && par->getDepth() != currentDepth)
			par = makeAny(text, buf, xs, runparams, par);
		// Usually, this loop only makes one iteration, except in complex scenarios, like an item with a paragraph,
		// a list, and another paragraph; or an item with two types of list (itemise then enumerate, for instance).

		// Close the item.
		closeTag(xs, style.docbookitemtag(), style.docbookitemtagtype());
		closeTag(xs, style.docbookitemwrappertag(), style.docbookitemwrappertagtype());
	}

	// Close this environment in exactly the same way as it was opened.
	closeTag(xs, envstyle.docbooktag(), envstyle.docbooktagtype());
	closeTag(xs, envstyle.docbookwrappertag(), envstyle.docbookwrappertagtype());

	return envend;
}


void makeCommand(
		Text const & text,
		Buffer const & buf,
		XMLStream & xs,
		OutputParams const & runparams,
		ParagraphList::const_iterator const & par)
{
	// Unlike XHTML, no need for labels, as they are handled by DocBook tags.
	auto const begin = text.paragraphs().begin();
	auto const end = text.paragraphs().end();
	auto nextpar = par;
	++nextpar;

	// Generate this command.
	auto prevpar = text.paragraphs().getParagraphBefore(par);
	openParTag(xs, &*par, prevpar);

	auto pars = par->simpleDocBookOnePar(buf, runparams,text.outerFont(distance(begin, par)));
	for (auto & parXML : pars)
		// TODO: decide what to do with openParTag/closeParTag in new lines.
		xs << XMLStream::ESCAPE_NONE << parXML;

	closeParTag(xs, &*par, (nextpar != end) ? &*nextpar : nullptr);
}


bool isLayoutSectioning(Layout const & lay)
{
	return lay.category() == from_utf8("Sectioning");
}


using DocBookDocumentSectioning = tuple<bool, pit_type>;


struct DocBookInfoTag
{
	const set<pit_type> shouldBeInInfo;
	const set<pit_type> mustBeInInfo; // With the notable exception of the abstract!
	const set<pit_type> abstract;
	const bool abstractLayout;
	pit_type bpit;
	pit_type epit;

	DocBookInfoTag(const set<pit_type> & shouldBeInInfo, const set<pit_type> & mustBeInInfo,
				   const set<pit_type> & abstract, bool abstractLayout, pit_type bpit, pit_type epit) :
				   shouldBeInInfo(shouldBeInInfo), mustBeInInfo(mustBeInInfo), abstract(abstract),
				   abstractLayout(abstractLayout), bpit(bpit), epit(epit) {}
};


DocBookDocumentSectioning hasDocumentSectioning(ParagraphList const &paragraphs, pit_type bpit, pit_type const epit) {
	bool documentHasSections = false;

	while (bpit < epit) {
		Layout const &style = paragraphs[bpit].layout();
		documentHasSections |= isLayoutSectioning(style);

		if (documentHasSections)
			break;
		bpit += 1;
	}
	// Paragraphs before the first section: [ runparams.par_begin ; eppit )

	return make_tuple(documentHasSections, bpit);
}


bool hasOnlyNotes(Paragraph const & par)
{
	// Precondition: the paragraph is not empty. Otherwise, the function will always return true...
	for (int i = 0; i < par.size(); ++i)
		// If you find something that is not an inset (like actual text) or an inset that is not a note,
		// return false.
		if (!par.isInset(i) || par.getInset(i)->lyxCode() != NOTE_CODE)
			return false;
	return true;
}


DocBookInfoTag getParagraphsWithInfo(ParagraphList const &paragraphs,
									 pit_type bpit, pit_type const epit,
									 // Typically, bpit is the beginning of the document and epit the end *or* the first section.
									 bool documentHasSections) {
	set<pit_type> shouldBeInInfo;
	set<pit_type> mustBeInInfo;
	set<pit_type> abstractWithLayout;
	set<pit_type> abstractNoLayout;

	// Find the first non empty paragraph by mutating bpit.
	while (bpit < epit) {
		Paragraph const &par = paragraphs[bpit];
		if (par.empty() || hasOnlyNotes(par))
			bpit += 1;
		else
			break;
	}

	// Traverse everything that might belong to <info>.
	bool hasAbstractLayout = false;
	pit_type cpit = bpit;
	for (; cpit < epit; ++cpit) {
		// Skip paragraphs that don't generate anything in DocBook.
		Paragraph const & par = paragraphs[cpit];
		if (hasOnlyNotes(par))
			continue;

		// There should never be any section here. (Just a sanity check: if this fails, this function could end up
		// processing the whole document.)
		if (isLayoutSectioning(par.layout())) {
			LYXERR0("Assertion failed: section found in potential <info> paragraphs.");
			break;
		}

		// If this is marked as an abstract by the layout, put it in the right set.
		if (par.layout().docbookabstract()) {
			hasAbstractLayout = true;
			abstractWithLayout.emplace(cpit);
			continue;
		}

		// Based on layout information, store this paragraph in one set: should be in <info>, must be,
		// or abstract (either because of layout or of position).
		Layout const &style = par.layout();

		if (style.docbookininfo() == "always")
			mustBeInInfo.emplace(cpit);
		else if (style.docbookininfo() == "maybe")
			shouldBeInInfo.emplace(cpit);
		else if (documentHasSections && !hasAbstractLayout)
			abstractNoLayout.emplace(cpit);
		else // This should definitely not be in <info>.
			break;
	}
	// Now, cpit points to the first paragraph that no more has things that could go in <info>.
	// bpit is the beginning of the <info> part.

	return DocBookInfoTag(shouldBeInInfo, mustBeInInfo,
					      hasAbstractLayout ? abstractWithLayout : abstractNoLayout,
					      hasAbstractLayout, bpit, cpit);
}

} // end anonymous namespace


ParagraphList::const_iterator makeAny(Text const &text,
                                      Buffer const &buf,
                                      XMLStream &xs,
                                      OutputParams const &runparams,
                                      ParagraphList::const_iterator par)
{
	switch (par->layout().latextype) {
	case LATEX_COMMAND:
		makeCommand(text, buf, xs, runparams, par);
		break;
	case LATEX_ENVIRONMENT:
		makeEnvironment(text, buf, xs, runparams, par);
		break;
	case LATEX_LIST_ENVIRONMENT:
	case LATEX_ITEM_ENVIRONMENT:
		// Only case when makeAny() might consume more than one paragraph.
		return makeListEnvironment(text, buf, xs, runparams, par);
	case LATEX_PARAGRAPH:
		makeParagraph(text, buf, xs, runparams, par);
		break;
	case LATEX_BIB_ENVIRONMENT:
		makeBibliography(text, buf, xs, runparams, par);
		break;
	}
	++par;
	return par;
}


xml::FontTag docbookStartFontTag(xml::FontTypes type)
{
	return xml::FontTag(from_utf8(fontToDocBookTag(type)), from_utf8(fontToAttribute(type)), type);
}


xml::EndFontTag docbookEndFontTag(xml::FontTypes type)
{
	return xml::EndFontTag(from_utf8(fontToDocBookTag(type)), type);
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
		// Generate the abstract XML into a string before further checks.
		odocstringstream os2;
		XMLStream xs2(os2);
		for (auto const & p : info.abstract)
			makeAny(text, buf, xs2, runparams, paragraphs.iterator_at(p));

		// Actually output the abstract if there is something to do. Don't count line feeds or spaces in this,
		// even though they must be properly output if there is some abstract.
		abstract = os2.str();
		docstring cleaned = abstract;
		cleaned.erase(std::remove_if(cleaned.begin(), cleaned.end(), lyx::isSpace), cleaned.end());

		// Nothing? Then there is no abstract!
		if (cleaned.empty())
			hasAbstract = false;
	}

	// The abstract must go in <info>. Otherwise, decide whether to open <info> based on the layouts.
	bool needInfo = !info.mustBeInInfo.empty() || hasAbstract;

	// Start the <info> tag if required.
	if (needInfo) {
		xs.startDivision(false);
		xs << xml::StartTag("info");
		xs << xml::CR();
	}

	// Output the elements that should go in <info>, before and after the abstract.
	for (auto pit : info.shouldBeInInfo) // Typically, the title: these elements are so important and ubiquitous
		// that mandating a wrapper like <info> would repel users. Thus, generate them first.
		makeAny(text, buf, xs, runparams, paragraphs.iterator_at(pit));
	for (auto pit : info.mustBeInInfo)
		makeAny(text, buf, xs, runparams, paragraphs.iterator_at(pit));

	// If there is no title, generate one (required for the document to be valid).
	// This code is called for the main document, for table cells, etc., so be precise in this condition.
	if (text.isMainText() && info.shouldBeInInfo.empty()) {
		xs << xml::StartTag("title");
		xs << "Untitled Document";
		xs << xml::EndTag("title");
		xs << xml::CR();
	}

	// Always output the abstract as the last item of the <info>, as it requires special treatment (especially if
	// it contains several paragraphs that are empty).
	if (hasAbstract) {
		if (info.abstractLayout) {
			xs << XMLStream::ESCAPE_NONE << abstract;
			xs << xml::CR();
		} else {
			string tag = paragraphs[*info.abstract.begin()].layout().docbookforceabstracttag();
			if (tag == "NONE")
				tag = "abstract";

			if (!xs.isLastTagCR())
				xs << xml::CR();

			xs << xml::StartTag(tag);
			xs << xml::CR();
			xs << XMLStream::ESCAPE_NONE << abstract;
			xs << xml::EndTag(tag);
			xs << xml::CR();
		}
	}

	// End the <info> tag if it was started.
	if (needInfo) {
		if (!xs.isLastTagCR())
			xs << xml::CR();

		xs << xml::EndTag("info");
		xs << xml::CR();
		xs.endDivision();
	}
}


void docbookSimpleAllParagraphs(
		Text const & text,
		Buffer const & buf,
		XMLStream & xs,
		OutputParams const & runparams)
{
	// Handle the given text, supposing it has no sections (i.e. a "simple" text). The input may vary in length
	// between a single paragraph to a whole document.
	pit_type const bpit = runparams.par_begin;
	pit_type const epit = runparams.par_end;
	ParagraphList const &paragraphs = text.paragraphs();

	// First, the <info> tag.
	DocBookInfoTag info = getParagraphsWithInfo(paragraphs, bpit, epit, false);
	outputDocBookInfo(text, buf, xs, runparams, paragraphs, info);

	// Then, the content. It starts where the <info> ends.
	auto par = paragraphs.iterator_at(info.epit);
	auto end = paragraphs.iterator_at(epit);
	while (par != end) {
		if (!hasOnlyNotes(*par))
			par = makeAny(text, buf, xs, runparams, par);
		else
			++par;
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

	std::stack<std::pair<int, string>> headerLevels; // Used to determine when to open/close sections: store the depth
	// of the section and the tag that was used to open it.

	// Detect whether the document contains sections. If there are no sections, treatment is largely simplified.
	// In particular, there can't be an abstract, unless it is manually marked.
	bool documentHasSections;
	pit_type eppit;
	tie(documentHasSections, eppit) = hasDocumentSectioning(paragraphs, bpit, epit);

	// Deal with "simple" documents, i.e. those without sections.
	if (!documentHasSections) {
		docbookSimpleAllParagraphs(text, buf, xs, runparams);
		return;
	}

	// Output the first <info> tag (or just the title).
	DocBookInfoTag info = getParagraphsWithInfo(paragraphs, bpit, eppit, true);
	outputDocBookInfo(text, buf, xs, runparams, paragraphs, info);
	bpit = info.epit;

	// Then, iterate through the paragraphs of this document.
	bool currentlyInAppendix = false;

	auto par = text.paragraphs().iterator_at(bpit);
	auto end = text.paragraphs().iterator_at(epit);
	while (par != end) {
		OutputParams ourparams = runparams;

		if (par->params().startOfAppendix())
			currentlyInAppendix = true;
		if (hasOnlyNotes(*par)) {
			++par;
			continue;
		}

		Layout const &style = par->layout();

		// Think about adding <section> and/or </section>s.
		if (isLayoutSectioning(style)) {
			int level = style.toclevel;

			// Need to close a previous section if it has the same level or a higher one (close <section> if opening a
			// <h2> after a <h2>, <h3>, <h4>, <h5> or <h6>). More examples:
			//   - current: h2; back: h1; do not close any <section>
			//   - current: h1; back: h2; close two <section> (first the <h2>, then the <h1>, so a new <h1> can come)
			while (!headerLevels.empty() && level <= headerLevels.top().first) {
				// Output the tag only if it corresponds to a legit section.
				int stackLevel = headerLevels.top().first;
				if (stackLevel != Layout::NOT_IN_TOC) {
					xs << xml::EndTag(headerLevels.top().second);
					xs << xml::CR();
				}
				headerLevels.pop();
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
				docstring attrs;
				if (!id.empty())
					attrs = id;
				xs << xml::StartTag(sectionTag, attrs);
				xs << xml::CR();
			}
		}

		// Close all sections before the bibliography.
		// TODO: Only close all when the bibliography is at the end of the document? Or force to output the bibliography at the end of the document? Or don't care (as allowed by DocBook)?
		if (!par->insetList().empty()) {
			Inset const *firstInset = par->getInset(0);
			if (firstInset && (firstInset->lyxCode() == BIBITEM_CODE || firstInset->lyxCode() == BIBTEX_CODE)) {
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
		par = makeAny(text, buf, xs, ourparams, par);

		// Some special sections may require abstracts (mostly parts, in books).
		// TODO: docbookforceabstracttag is a bit contrived here, but it does the job. Having another field just for this would be cleaner, but that's just for <part> and <partintro>, so it's probably not worth the effort.
		if (isLayoutSectioning(style) && style.docbookforceabstracttag() != "NONE") {
			// This abstract may be found between the next paragraph and the next title.
			pit_type cpit = std::distance(text.paragraphs().begin(), par);
			pit_type ppit = std::get<1>(hasDocumentSectioning(paragraphs, cpit, epit));

			// Generate this abstract (this code corresponds to parts of outputDocBookInfo).
			DocBookInfoTag secInfo = getParagraphsWithInfo(paragraphs, cpit, ppit, true);

			if (!secInfo.abstract.empty()) {
				xs << xml::StartTag(style.docbookforceabstracttag());
				xs << xml::CR();
				for (auto const &p : secInfo.abstract)
					makeAny(text, buf, xs, runparams, paragraphs.iterator_at(p));
				xs << xml::EndTag(style.docbookforceabstracttag());
				xs << xml::CR();
			}

			// Skip all the text that just has been generated.
			par = paragraphs.iterator_at(ppit);
		}
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