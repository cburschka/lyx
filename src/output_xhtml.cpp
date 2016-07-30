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
#include "sgml.h"
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

namespace html {

docstring escapeChar(char_type c, XHTMLStream::EscapeSettings e)
{
	docstring str;
	switch (e) {
	case XHTMLStream::ESCAPE_NONE:
		str += c;
		break;
	case XHTMLStream::ESCAPE_ALL:
		if (c == '<') {
			str += "&lt;";
			break;
		} else if (c == '>') {
			str += "&gt;";
			break;
		}
	// fall through
	case XHTMLStream::ESCAPE_AND:
		if (c == '&')
			str += "&amp;";
		else
			str	+=c ;
		break;
	}
	return str;
}


// escape what needs escaping
docstring htmlize(docstring const & str, XHTMLStream::EscapeSettings e)
{
	odocstringstream d;
	docstring::const_iterator it = str.begin();
	docstring::const_iterator en = str.end();
	for (; it != en; ++it)
		d << escapeChar(*it, e);
	return d.str();
}


docstring escapeChar(char c, XHTMLStream::EscapeSettings e)
{
	LATTEST(static_cast<unsigned char>(c) < 0x80);
	return escapeChar(static_cast<char_type>(c), e);
}


docstring cleanAttr(docstring const & str)
{
	docstring newname;
	docstring::const_iterator it = str.begin();
	docstring::const_iterator en = str.end();
	for (; it != en; ++it) {
		char_type const c = *it;
		newname += isAlnumASCII(c) ? c : char_type('_');
	}
	return newname;
}


docstring StartTag::writeTag() const
{
	docstring output = '<' + from_utf8(tag_);
	if (!attr_.empty())
		output += ' ' + html::htmlize(from_utf8(attr_), XHTMLStream::ESCAPE_NONE);
	output += ">";
	return output;
}


docstring StartTag::writeEndTag() const
{
	string output = "</" + tag_ + ">";
	return from_utf8(output);
}


bool StartTag::operator==(FontTag const & rhs) const
{
	return rhs == *this;
}


docstring EndTag::writeEndTag() const
{
	string output = "</" + tag_ + ">";
	return from_utf8(output);
}


ParTag::ParTag(std::string const & tag, std::string attr,
       std::string const & parid)
  : StartTag(tag)
{
	if (!parid.empty())
		attr += " id='" + parid + "'";
	attr_ = attr;
}


docstring CompTag::writeTag() const
{
	docstring output = '<' + from_utf8(tag_);
	if (!attr_.empty())
		output += ' ' + html::htmlize(from_utf8(attr_), XHTMLStream::ESCAPE_NONE);
	output += " />";
	return output;
}



namespace {

string fontToTag(html::FontTypes type)
{
	switch(type) {
	case FT_EMPH:
		return "em";
	case FT_BOLD:
		return "b";
	case FT_NOUN:
		return "dfn";
	case FT_UBAR:
	case FT_WAVE:
	case FT_DBAR:
		return "u";
	case FT_SOUT:
		return "del";
	case FT_ITALIC:
		return "i";
	case FT_UPRIGHT:
	case FT_SLANTED:
	case FT_SMALLCAPS:
	case FT_ROMAN:
	case FT_SANS:
	case FT_TYPE:
	case FT_SIZE_TINY:
	case FT_SIZE_SCRIPT:
	case FT_SIZE_FOOTNOTE:
	case FT_SIZE_SMALL:
	case FT_SIZE_NORMAL:
	case FT_SIZE_LARGE:
	case FT_SIZE_LARGER:
	case FT_SIZE_LARGEST:
	case FT_SIZE_HUGE:
	case FT_SIZE_HUGER:
	case FT_SIZE_INCREASE:
	case FT_SIZE_DECREASE:
		return "span";
	}
	// kill warning
	return "";
}

string fontToAttribute(html::FontTypes type)
{
	switch(type) {
	case FT_EMPH:
	case FT_BOLD:
		return "";
	case FT_NOUN:
		return "class='lyxnoun'";
	case FT_UBAR:
		return "";
	case FT_DBAR:
		return "class='dline'";
	case FT_SOUT:
		return "class='strikeout'";
	case FT_WAVE:
		return "class='wline'";
	case FT_ITALIC:
		return "";
	case FT_UPRIGHT:
		return "style='font-style:normal;'";
	case FT_SLANTED:
		return "style='font-style:oblique;'";
	case FT_SMALLCAPS:
		return "style='font-variant:small-caps;'";
	case FT_ROMAN:
		return "style='font-family:serif;'";
	case FT_SANS:
		return "style='font-family:sans-serif;'";
	case FT_TYPE:
		return "style='font-family:monospace;'";
	case FT_SIZE_TINY:
	case FT_SIZE_SCRIPT:
	case FT_SIZE_FOOTNOTE:
		return "style='font-size:x-small;'";
	case FT_SIZE_SMALL:
		return "style='font-size:small;'";
	case FT_SIZE_NORMAL:
		return "style='font-size:normal;'";
	case FT_SIZE_LARGE:
		return "style='font-size:large;'";
	case FT_SIZE_LARGER:
	case FT_SIZE_LARGEST:
		return "style='font-size:x-large;'";
	case FT_SIZE_HUGE:
	case FT_SIZE_HUGER:
		return "style='font-size:xx-large;'";
	case FT_SIZE_INCREASE:
		return "style='font-size:larger;'";
	case FT_SIZE_DECREASE:
		return "style='font-size:smaller;'";
	}
	// kill warning
	return "";
}

} // end anonymous namespace


FontTag::FontTag(FontTypes type)
  : StartTag(fontToTag(type), fontToAttribute(type)), font_type_(type)
{}


bool FontTag::operator==(StartTag const & tag) const
{
	FontTag const * const ftag = tag.asFontTag();
	if (!ftag)
		return false;
	return (font_type_ == ftag->font_type_);
}


EndFontTag::EndFontTag(FontTypes type)
	  : EndTag(fontToTag(type)), font_type_(type)
{}

} // namespace html



////////////////////////////////////////////////////////////////
///
/// XHTMLStream
///
////////////////////////////////////////////////////////////////

XHTMLStream::XHTMLStream(odocstream & os)
  : os_(os), escape_(ESCAPE_ALL)
{}


#ifdef XHTML_DEBUG
void XHTMLStream::dumpTagStack(string const & msg)
{
	*this << html::CR();
	writeError(msg);
	*this << html::CR();
	writeError("Tag Stack");
	TagDeque::const_reverse_iterator it = tag_stack_.rbegin();
	TagDeque::const_reverse_iterator en = tag_stack_.rend();
	for (; it != en; ++it) {
		writeError(it->get()->tag_);
	}
	writeError("End Tag Stack");
	*this << html::CR();
	writeError("Pending Tags");
	it = pending_tags_.rbegin();
	en = pending_tags_.rend();
	for (; it != en; ++it) {
		writeError(it->get()->tag_);
	}
	writeError("End Pending Tags");
	*this << html::CR();
}
#endif


void XHTMLStream::writeError(std::string const & s) const
{
	LYXERR0(s);
	os_ << from_utf8("<!-- Output Error: " + s + " -->\n");
}


namespace {
	// an illegal tag for internal use
	static html::StartTag const parsep_tag("&LyX_parsep_tag&");
}


bool XHTMLStream::closeFontTags()
{
	if (isTagPending(parsep_tag))
		// we haven't had any content
		return true;

	// this may be a useless check, since we ought at least to have
	// the parsep_tag. but it can't hurt too much to be careful.
	if (tag_stack_.empty())
		return true;

	// first, we close any open font tags we can close
	TagPtr curtag = tag_stack_.back();
	while (curtag->asFontTag()) {
		os_ << curtag->writeEndTag();
		tag_stack_.pop_back();
		// this shouldn't happen, since then the font tags
		// weren't in any other tag.
		LBUFERR(!tag_stack_.empty());
		curtag = tag_stack_.back();
	}

	if (*curtag == parsep_tag)
		return true;

	// so we've hit a non-font tag.
	writeError("Tags still open in closeFontTags(). Probably not a problem,\n"
	           "but you might want to check these tags:");
	TagDeque::const_reverse_iterator it = tag_stack_.rbegin();
	TagDeque::const_reverse_iterator const en = tag_stack_.rend();
	for (; it != en; ++it) {
		if (**it == parsep_tag)
			break;
		writeError((*it)->tag_);
	}
	return false;
}


void XHTMLStream::startDivision(bool keep_empty)
{
	pending_tags_.push_back(makeTagPtr(html::StartTag(parsep_tag)));
	if (keep_empty)
		clearTagDeque();
}


void XHTMLStream::endDivision()
{
	if (isTagPending(parsep_tag)) {
		// this case is normal. it just means we didn't have content,
		// so the parsep_tag never got moved onto the tag stack.
		while (!pending_tags_.empty()) {
			// clear all pending tags up to and including the parsep tag.
			// note that we work from the back, because we want to get rid
			// of everything that hasn't been used.
			TagPtr const cur_tag = pending_tags_.back();
			pending_tags_.pop_back();
			if (*cur_tag == parsep_tag)
				break;
		}
		return;
	}

	if (!isTagOpen(parsep_tag)) {
		writeError("No division separation tag found in endDivision().");
		return;
	}

	// this case is also normal, if the parsep tag is the last one
	// on the stack. otherwise, it's an error.
	while (!tag_stack_.empty()) {
		TagPtr const cur_tag = tag_stack_.back();
		tag_stack_.pop_back();
		if (*cur_tag == parsep_tag)
			break;
		writeError("Tag `" + cur_tag->tag_ + "' still open at end of paragraph. Closing.");
		os_ << cur_tag->writeEndTag();
	}
}


void XHTMLStream::clearTagDeque()
{
	while (!pending_tags_.empty()) {
		TagPtr const tag = pending_tags_.front();
		if (*tag != parsep_tag)
			// tabs?
			os_ << tag->writeTag();
		tag_stack_.push_back(tag);
		pending_tags_.pop_front();
	}
}


XHTMLStream & XHTMLStream::operator<<(docstring const & d)
{
	clearTagDeque();
	os_ << html::htmlize(d, escape_);
	escape_ = ESCAPE_ALL;
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(const char * s)
{
	clearTagDeque();
	docstring const d = from_ascii(s);
	os_ << html::htmlize(d, escape_);
	escape_ = ESCAPE_ALL;
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(char_type c)
{
	clearTagDeque();
	os_ << html::escapeChar(c, escape_);
	escape_ = ESCAPE_ALL;
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(char c)
{
	clearTagDeque();
	os_ << html::escapeChar(c, escape_);
	escape_ = ESCAPE_ALL;
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(int i)
{
	clearTagDeque();
	os_ << i;
	escape_ = ESCAPE_ALL;
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(EscapeSettings e)
{
	escape_ = e;
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(html::StartTag const & tag)
{
	if (tag.tag_.empty())
		return *this;
	pending_tags_.push_back(makeTagPtr(tag));
	if (tag.keepempty_)
		clearTagDeque();
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(html::ParTag const & tag)
{
	if (tag.tag_.empty())
		return *this;
	pending_tags_.push_back(makeTagPtr(tag));
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(html::CompTag const & tag)
{
	if (tag.tag_.empty())
		return *this;
	clearTagDeque();
	os_ << tag.writeTag();
	*this << html::CR();
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(html::FontTag const & tag)
{
	if (tag.tag_.empty())
		return *this;
	pending_tags_.push_back(makeTagPtr(tag));
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(html::CR const &)
{
	// tabs?
	os_ << from_ascii("\n");
	return *this;
}


bool XHTMLStream::isTagOpen(html::StartTag const & stag) const
{
	TagDeque::const_iterator sit = tag_stack_.begin();
	TagDeque::const_iterator const sen = tag_stack_.end();
	for (; sit != sen; ++sit)
		if (**sit == stag)
			return true;
	return false;
}


bool XHTMLStream::isTagOpen(html::EndTag const & etag) const
{
	TagDeque::const_iterator sit = tag_stack_.begin();
	TagDeque::const_iterator const sen = tag_stack_.end();
	for (; sit != sen; ++sit)
		if (etag == **sit)
			return true;
	return false;
}


bool XHTMLStream::isTagPending(html::StartTag const & stag) const
{
	TagDeque::const_iterator sit = pending_tags_.begin();
	TagDeque::const_iterator const sen = pending_tags_.end();
	for (; sit != sen; ++sit)
		if (**sit == stag)
			return true;
	return false;
}


// this is complicated, because we want to make sure that
// everything is properly nested. the code ought to make
// sure of that, but we won't assert (yet) if we run into
// a problem. we'll just output error messages and try our
// best to make things work.
XHTMLStream & XHTMLStream::operator<<(html::EndTag const & etag)
{
	if (etag.tag_.empty())
		return *this;

	// if this tag is pending, we can simply discard it.
	if (!pending_tags_.empty()) {

		if (etag == *pending_tags_.back()) {
			// we have <tag></tag>, so we discard it and remove it
			// from the pending_tags_.
			pending_tags_.pop_back();
			return *this;
		}

		// there is a pending tag that isn't the one we are trying
		// to close.

		// is this tag itself pending?
		// non-const iterators because we may call erase().
		TagDeque::iterator dit = pending_tags_.begin();
		TagDeque::iterator const den = pending_tags_.end();
		for (; dit != den; ++dit) {
			if (etag == **dit) {
				// it was pending, so we just erase it
				writeError("Tried to close pending tag `" + etag.tag_
				        + "' when other tags were pending. Last pending tag is `"
				        + to_utf8(pending_tags_.back()->writeTag())
				        + "'. Tag discarded.");
				pending_tags_.erase(dit);
				return *this;
			}
		}
		// so etag isn't itself pending. is it even open?
		if (!isTagOpen(etag)) {
			writeError("Tried to close `" + etag.tag_
			         + "' when tag was not open. Tag discarded.");
			return *this;
		}
		// ok, so etag is open.
		// our strategy will be as below: we will do what we need to
		// do to close this tag.
		string estr = "Closing tag `" + etag.tag_
		        + "' when other tags are pending. Discarded pending tags:\n";
		for (dit = pending_tags_.begin(); dit != den; ++dit)
			estr += to_utf8(html::htmlize((*dit)->writeTag(), XHTMLStream::ESCAPE_ALL)) + "\n";
		writeError(estr);
		// clear the pending tags...
		pending_tags_.clear();
		// ...and then just fall through.
	}

	// make sure there are tags to be closed
	if (tag_stack_.empty()) {
		writeError("Tried to close `" + etag.tag_
		         + "' when no tags were open!");
		return *this;
	}

	// is the tag we are closing the last one we opened?
	if (etag == *tag_stack_.back()) {
		// output it...
		os_ << etag.writeEndTag();
		// ...and forget about it
		tag_stack_.pop_back();
		return *this;
	}

	// we are trying to close a tag other than the one last opened.
	// let's first see if this particular tag is still open somehow.
	if (!isTagOpen(etag)) {
		writeError("Tried to close `" + etag.tag_
		        + "' when tag was not open. Tag discarded.");
		return *this;
	}

	// so the tag was opened, but other tags have been opened since
	// and not yet closed.
	// if it's a font tag, though...
	if (etag.asFontTag()) {
		// it won't be a problem if the other tags open since this one
		// are also font tags.
		TagDeque::const_reverse_iterator rit = tag_stack_.rbegin();
		TagDeque::const_reverse_iterator ren = tag_stack_.rend();
		for (; rit != ren; ++rit) {
			if (etag == **rit)
				break;
			if (!(*rit)->asFontTag()) {
				// we'll just leave it and, presumably, have to close it later.
				writeError("Unable to close font tag `" + etag.tag_
				        + "' due to open non-font tag `" + (*rit)->tag_ + "'.");
				return *this;
			}
		}

		// so we have e.g.:
		//    <em>this is <strong>bold
		// and are being asked to closed em. we want:
		//    <em>this is <strong>bold</strong></em><strong>
		// first, we close the intervening tags...
		TagPtr curtag = tag_stack_.back();
		// ...remembering them in a stack.
		TagDeque fontstack;
		while (etag != *curtag) {
			os_ << curtag->writeEndTag();
			fontstack.push_back(curtag);
			tag_stack_.pop_back();
			curtag = tag_stack_.back();
		}
		os_ << etag.writeEndTag();
		tag_stack_.pop_back();

		// ...and restore the other tags.
		rit = fontstack.rbegin();
		ren = fontstack.rend();
		for (; rit != ren; ++rit)
			pending_tags_.push_back(*rit);
		return *this;
	}

	// it wasn't a font tag.
	// so other tags were opened before this one and not properly closed.
	// so we'll close them, too. that may cause other issues later, but it
	// at least guarantees proper nesting.
	writeError("Closing tag `" + etag.tag_
	        + "' when other tags are open, namely:");
	TagPtr curtag = tag_stack_.back();
	while (etag != *curtag) {
		writeError(curtag->tag_);
		if (*curtag != parsep_tag)
			os_ << curtag->writeEndTag();
		tag_stack_.pop_back();
		curtag = tag_stack_.back();
	}
	// curtag is now the one we actually want.
	os_ << curtag->writeEndTag();
	tag_stack_.pop_back();

	return *this;
}

// End code for XHTMLStream

namespace {

// convenience functions

inline void openParTag(XHTMLStream & xs, Layout const & lay,
                       std::string parlabel)
{
	xs << html::ParTag(lay.htmltag(), lay.htmlattr(), parlabel);
}


void openParTag(XHTMLStream & xs, Layout const & lay,
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
	xs << html::ParTag(lay.htmltag(), attrs, parlabel);
}


inline void closeTag(XHTMLStream & xs, Layout const & lay)
{
	xs << html::EndTag(lay.htmltag());
}


inline void openLabelTag(XHTMLStream & xs, Layout const & lay)
{
	xs << html::StartTag(lay.htmllabeltag(), lay.htmllabelattr());
}


inline void closeLabelTag(XHTMLStream & xs, Layout const & lay)
{
	xs << html::EndTag(lay.htmllabeltag());
}


inline void openItemTag(XHTMLStream & xs, Layout const & lay)
{
	xs << html::StartTag(lay.htmlitemtag(), lay.htmlitemattr(), true);
}


void openItemTag(XHTMLStream & xs, Layout const & lay,
             ParagraphParameters const & params)
{
	// FIXME Are there other things we should handle here?
	string const align = alignmentToCSS(params.align());
	if (align.empty()) {
		openItemTag(xs, lay);
		return;
	}
	string attrs = lay.htmlattr() + " style='text-align: " + align + ";'";
	xs << html::StartTag(lay.htmlitemtag(), attrs);
}


inline void closeItemTag(XHTMLStream & xs, Layout const & lay)
{
	xs << html::EndTag(lay.htmlitemtag());
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
					    XHTMLStream & xs,
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
			xs << html::CR();

		// We want to open the paragraph tag if:
		//   (i) the current layout permits multiple paragraphs
		//  (ii) we are either not already inside a paragraph (HTMLIsBlock) OR
		//       we are, but this is not the first paragraph
		// But we do not want to open the paragraph tag if this paragraph contains
		// only one item, and that item is "inline", i.e., not HTMLIsBlock (such 
		// as a branch). That is the "special case" we handle first.
		Inset const * specinset = par->size() == 1 ? par->getInset(0) : 0;
		bool const special_case =  
			specinset && !specinset->getLayout().htmlisblock();

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
			xs << html::CR();
		}

		if (!deferred.empty()) {
			xs << XHTMLStream::ESCAPE_NONE << deferred << html::CR();
		}
	}
	return pend;
}


ParagraphList::const_iterator makeBibliography(Buffer const & buf,
				XHTMLStream & xs,
				OutputParams const & runparams,
				Text const & text,
				ParagraphList::const_iterator const & pbegin,
				ParagraphList::const_iterator const & pend)
{
	// FIXME XHTML
	// Use TextClass::htmlTOCLayout() to figure out how we should look.
	xs << html::StartTag("h2", "class='bibliography'")
	   << pbegin->layout().labelstring(false)
	   << html::EndTag("h2")
	   << html::CR()
	   << html::StartTag("div", "class='bibliography'")
	   << html::CR();
	makeParagraphs(buf, xs, runparams, text, pbegin, pend);
	xs << html::EndTag("div");
	return pend;
}


bool isNormalEnv(Layout const & lay)
{
	return lay.latextype == LATEX_ENVIRONMENT
	    || lay.latextype == LATEX_BIB_ENVIRONMENT;
}


ParagraphList::const_iterator makeEnvironment(Buffer const & buf,
					      XHTMLStream & xs,
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
	xs << html::CR();

	// we will on occasion need to remember a layout from before.
	Layout const * lastlay = 0;

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
			// There are two possiblities in this case.
			// One is that we are still in the environment in which we
			// started---which we will be if the depth is the same.
			if (par->params().depth() == origdepth) {
				LATTEST(bstyle == style);
				if (lastlay != 0) {
					closeItemTag(xs, *lastlay);
					lastlay = 0;
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
							xs << html::CR();
						}
					} else { // some kind of list
						if (style.labeltype == LABEL_MANUAL) {
							openLabelTag(xs, style);
							sep = par->firstWordLyXHTML(xs, runparams);
							closeLabelTag(xs, style);
							xs << html::CR();
						}
						else {
							openLabelTag(xs, style);
							xs << par->params().labelString();
							closeLabelTag(xs, style);
							xs << html::CR();
						}
					}
				} // end label output

				if (labelfirst)
					openItemTag(xs, style, par->params());

				par->simpleLyXHTMLOnePar(buf, xs, runparams,
					text.outerFont(distance(begin, par)), true, true, sep);
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
				xs << html::CR();
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

	if (lastlay != 0)
		closeItemTag(xs, *lastlay);
	closeTag(xs, bstyle);
	xs << html::CR();
	return pend;
}


void makeCommand(Buffer const & buf,
		 XHTMLStream & xs,
		 OutputParams const & runparams,
		 Text const & text,
		 ParagraphList::const_iterator const & pbegin)
{
	Layout const & style = pbegin->layout();
	if (!style.counter.empty())
		buf.masterBuffer()->params().
		    documentClass().counters().step(style.counter, OutputUpdate);

	bool const make_parid = !runparams.for_toc && runparams.html_make_pars;
	
	if (style.labeltype == LABEL_ABOVE)
		xs << html::StartTag("div")
		   << pbegin->params().labelString()
		   << html::EndTag("div");
	else if (style.labeltype == LABEL_CENTERED)
		xs << html::StartTag("div", "style = \"text-align: center;\"")
		   << pbegin->params().labelString()
		   << html::EndTag("div");

	openParTag(xs, style, pbegin->params(),
	           make_parid ? pbegin->magicLabel() : "");

	// Label around sectioning number:
	// FIXME Probably need to account for LABEL_MANUAL
	if (style.labeltype != LABEL_NO_LABEL &&
	    style.labeltype != LABEL_ABOVE &&
	    style.labeltype != LABEL_CENTERED ) {
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
	xs << html::CR();
}

} // end anonymous namespace


void xhtmlParagraphs(Text const & text,
		       Buffer const & buf,
		       XHTMLStream & xs,
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
		{ xs << XHTMLStream::ESCAPE_NONE << "<!-- XHTML output error! -->\n"; return; });

	OutputParams ourparams = runparams;
	ParagraphList::const_iterator const pend =
		(epit == (int) paragraphs.size()) ?
			paragraphs.end() : paragraphs.constIterator(epit);
	while (bpit < epit) {
		ParagraphList::const_iterator par = paragraphs.constIterator(bpit);
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
