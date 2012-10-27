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


string escapeChar(char c, XHTMLStream::EscapeSettings e)
{
	string str;
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
string htmlize(string const & str, XHTMLStream::EscapeSettings e)
{
	ostringstream d;
	string::const_iterator it = str.begin();
	string::const_iterator en = str.end();
	for (; it != en; ++it)
		d << escapeChar(*it, e);
	return d.str();
}


string cleanAttr(string const & str)
{
	string newname;
	string::const_iterator it = str.begin();
	string::const_iterator en = str.end();
	for (; it != en; ++it)
		newname += isAlnumASCII(*it) ? *it : '_';
	return newname;	
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


bool isFontTag(string const & s)
{
	// others?
	return s == "em" || s == "strong" || s == "i" || s == "b";
}


docstring StartTag::asTag() const
{
	string output = "<" + tag_;
	if (!attr_.empty())
		output += " " + html::htmlize(attr_, XHTMLStream::ESCAPE_NONE);
	output += ">";
	return from_utf8(output);
}


docstring StartTag::asEndTag() const
{
	string output = "</" + tag_ + ">";
	return from_utf8(output);
}


docstring EndTag::asEndTag() const
{
	string output = "</" + tag_ + ">";
	return from_utf8(output);
}


docstring CompTag::asTag() const
{
	string output = "<" + tag_;
	if (!attr_.empty())
		output += " " + html::htmlize(attr_, XHTMLStream::ESCAPE_NONE);
	output += " />";
	return from_utf8(output);
}

} // namespace html



////////////////////////////////////////////////////////////////
///
/// XHTMLStream
///
////////////////////////////////////////////////////////////////

XHTMLStream::XHTMLStream(odocstream & os) 
		: os_(os), escape_(ESCAPE_ALL)
{}


#if 0
void XHTMLStream::dumpTagStack(string const & msg) const
{
	writeError(msg + ": Tag Stack");
	TagStack::const_reverse_iterator it = tag_stack_.rbegin();
	TagStack::const_reverse_iterator en = tag_stack_.rend();
	for (; it != en; ++it) {
		writeError(it->tag_);
	}
	writeError("Pending Tags");
	it = pending_tags_.rbegin();
	en = pending_tags_.rend();
	for (; it != en; ++it) {
		writeError(it->tag_);
	}
	writeError("End Tag Stack");
}
#endif


void XHTMLStream::writeError(std::string const & s) const
{
	LYXERR0(s);
	os_ << from_utf8("<!-- Output Error: " + s + " -->\n");
}


namespace {
	// an illegal tag for internal use
	static string const parsep_tag = "&LyX_parsep_tag&";
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
	html::StartTag curtag = tag_stack_.back();
	while (html::isFontTag(curtag.tag_)) {
		os_ << curtag.asEndTag();
		tag_stack_.pop_back();
		if (tag_stack_.empty())
			// this probably shouldn't happen, since then the
			// font tags weren't in any other tag. but that
			// problem will likely be caught elsewhere.
			return true;
		curtag = tag_stack_.back();
	}
	
	if (curtag.tag_ == parsep_tag)
		return true;

	// so we've hit a non-font tag.
	writeError("Tags still open in closeFontTags(). Probably not a problem,\n"
	           "but you might want to check these tags:");
	TagStack::const_reverse_iterator it = tag_stack_.rbegin();
	TagStack::const_reverse_iterator const en = tag_stack_.rend();
	for (; it != en; ++it) {
		string const tagname = it->tag_;
		if (tagname == parsep_tag)
			break;
		writeError(it->tag_);
	}
	return false;
}


void XHTMLStream::startParagraph(bool keep_empty)
{
	pending_tags_.push_back(html::StartTag(parsep_tag));
	if (keep_empty)
		clearTagDeque();
}


void XHTMLStream::endParagraph()
{
	if (isTagPending(parsep_tag)) {
		// this case is normal. it just means we didn't have content,
		// so the parsep_tag never got moved onto the tag stack.
		while (!pending_tags_.empty()) {
			// clear all pending tags up to and including the parsep tag.
			// note that we work from the back, because we want to get rid
			// of everything that hasn't been used.
			html::StartTag const cur_tag = pending_tags_.back();
			string const & tag = cur_tag.tag_;
			pending_tags_.pop_back();
			if (tag == parsep_tag)
				break;
		}
		return;
	}

	if (!isTagOpen(parsep_tag)) {
		writeError("No paragraph separation tag found in endParagraph().");
		return;
	}

	// this case is also normal, if the parsep tag is the last one 
	// on the stack. otherwise, it's an error.
	while (!tag_stack_.empty()) {
		html::StartTag const cur_tag = tag_stack_.back();
		string const & tag = cur_tag.tag_;
		tag_stack_.pop_back();
		if (tag == parsep_tag)
			break;
		writeError("Tag `" + tag + "' still open at end of paragraph. Closing.");
		os_ << cur_tag.asEndTag();
	}
}


void XHTMLStream::clearTagDeque()
{
	while (!pending_tags_.empty()) {
		html::StartTag const & tag = pending_tags_.front();
		if (tag.tag_ != parsep_tag)
			// tabs?
			os_ << tag.asTag();
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
	string const d = html::escapeChar(c, escape_);
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
	pending_tags_.push_back(tag);
	if (tag.keepempty_)
		clearTagDeque();
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(html::CompTag const & tag) 
{
	if (tag.tag_.empty())
		return *this;
	clearTagDeque();
	// tabs?
	os_ << tag.asTag();
	*this << html::CR();
	return *this;
}


XHTMLStream & XHTMLStream::operator<<(html::CR const &)
{
	// tabs?
	os_ << from_ascii("\n");
	return *this;
}


bool XHTMLStream::isTagOpen(string const & stag) const
{
	TagStack::const_iterator sit = tag_stack_.begin();
	TagStack::const_iterator const sen = tag_stack_.end();
	for (; sit != sen; ++sit)
		if (sit->tag_ == stag) 
			return true;
	return false;
}


bool XHTMLStream::isTagPending(string const & stag) const
{
	TagStack::const_iterator sit = pending_tags_.begin();
	TagStack::const_iterator const sen = pending_tags_.end();
	for (; sit != sen; ++sit)
		if (sit->tag_ == stag)
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

	// make sure there are tags to be closed
	if (tag_stack_.empty()) {
		writeError("Tried to close `" + etag.tag_
		         + "' when no tags were open!");
		return *this;		
	}

	// first make sure we're not closing an empty tag
	if (!pending_tags_.empty()) {
		html::StartTag const & stag = pending_tags_.back();
		if (etag.tag_ == stag.tag_)  {
			// we have <tag></tag>, so we discard it and remove it 
			// from the pending_tags_.
			pending_tags_.pop_back();
			return *this;
		}
		// there is a pending tag that isn't the one we are trying
		// to close. 
		// is this tag itself pending?
		// non-const iterators because we may call erase().
		TagStack::iterator dit = pending_tags_.begin();
		TagStack::iterator const den = pending_tags_.end();
		for (; dit != den; ++dit) {
			if (dit->tag_ == etag.tag_) {
				// it was pending, so we just erase it
				writeError("Tried to close pending tag `" + etag.tag_ 
				        + "' when other tags were pending. Last pending tag is `"
				        + pending_tags_.back().tag_ + "'. Tag discarded.");
				pending_tags_.erase(dit);
				return *this;
			}
		}
		// so etag isn't itself pending. is it even open?
		if (!isTagOpen(etag.tag_)) {
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
			estr += dit->tag_ + "\n";
		writeError(estr);
		// clear the pending tags...
		pending_tags_.clear();
		// ...and then just fall through.
	}

	// is the tag we are closing the last one we opened?
	if (etag.tag_ == tag_stack_.back().tag_) {
		// output it...
		os_ << etag.asEndTag();
		// ...and forget about it
		tag_stack_.pop_back();
		return *this;
	} 
	
	// we are trying to close a tag other than the one last opened. 
	// let's first see if this particular tag is still open somehow.
	if (!isTagOpen(etag.tag_)) {
		writeError("Tried to close `" + etag.tag_ 
		        + "' when tag was not open. Tag discarded.");
		return *this;
	}
	
	// so the tag was opened, but other tags have been opened since
	// and not yet closed.
	// if it's a font tag, though...
	if (html::isFontTag(etag.tag_)) {
		// it won't be a problem if the other tags open since this one
		// are also font tags.
		TagStack::const_reverse_iterator rit = tag_stack_.rbegin();
		TagStack::const_reverse_iterator ren = tag_stack_.rend();
		for (; rit != ren; ++rit) {
			if (rit->tag_ == etag.tag_)
				break;
			if (!html::isFontTag(rit->tag_)) {
				// we'll just leave it and, presumably, have to close it later.
				writeError("Unable to close font tag `" + etag.tag_ 
				        + "' due to open non-font tag `" + rit->tag_ + "'.");
				return *this;
			}
		}
		
		// so we have e.g.:
		//    <em>this is <strong>bold
		// and are being asked to closed em. we want:
		//    <em>this is <strong>bold</strong></em><strong>
		// first, we close the intervening tags...
		html::StartTag curtag = tag_stack_.back();
		// ...remembering them in a stack.
		TagStack fontstack;
		while (curtag.tag_ != etag.tag_) {
			os_ << curtag.asEndTag();
			fontstack.push_back(curtag);
			tag_stack_.pop_back();
			curtag = tag_stack_.back();
		}
		// now close our tag...
		os_ << etag.asEndTag();
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
	html::StartTag curtag = tag_stack_.back();
	while (curtag.tag_ != etag.tag_) {
		writeError(curtag.tag_);
		if (curtag.tag_ != parsep_tag)
			os_ << curtag.asEndTag();
		tag_stack_.pop_back();
		curtag = tag_stack_.back();
	}
	// curtag is now the one we actually want.
	os_ << curtag.asEndTag();
	tag_stack_.pop_back();
	
	return *this;
}

// End code for XHTMLStream

namespace {
	
// convenience functions

inline void openTag(XHTMLStream & xs, Layout const & lay)
{
	xs << html::StartTag(lay.htmltag(), lay.htmlattr());
}


void openTag(XHTMLStream & xs, Layout const & lay, 
             ParagraphParameters const & params)
{
	// FIXME Are there other things we should handle here?
	string const align = alignmentToCSS(params.align());
	if (align.empty()) {
		openTag(xs, lay);
		return;
	}
	string attrs = lay.htmlattr() + " style='text-align: " + align + ";'";
	xs << html::StartTag(lay.htmltag(), attrs);
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
			buf.params().documentClass().counters().step(lay.counter, OutputUpdate);
		// FIXME We should see if there's a label to be output and
		// do something with it.
		if (par != pbegin)
			xs << html::CR();

		// If we are already in a paragraph, and this is the first one, then we
		// do not want to open the paragraph tag.
		// we also do not want to open it if the current layout does not permit
		// multiple paragraphs.
		bool const opened = runparams.html_make_pars &&
			(par != pbegin || !runparams.html_in_par);
		if (opened)
			openTag(xs, lay, par->params());
		docstring const deferred = 
			par->simpleLyXHTMLOnePar(buf, xs, runparams, text.outerFont(distance(begin, par)));

		// We want to issue the closing tag if either:
		//   (i)  We opened it, and either html_in_par is false,
		//        or we're not in the last paragraph, anyway.
		//   (ii) We didn't open it and html_in_par is true, 
		//        but we are in the first par, and there is a next par.
		ParagraphList::const_iterator nextpar = par;
		++nextpar;
		bool const needclose = 
			(opened && (!runparams.html_in_par || nextpar != pend))
			|| (!opened && runparams.html_in_par && par == pbegin && nextpar != pend);
		if (needclose) {
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

	
ParagraphList::const_iterator makeEnvironmentHtml(Buffer const & buf,
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
	openTag(xs, bstyle);
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
		Counters & cnts = buf.params().documentClass().counters();
		docstring const & cntr = style.counter;
		if (!style.counter.empty() 
		    && (par == pbegin || !isNormalEnv(style)) 
				&& cnts.hasCounter(cntr)
		)
			cnts.step(cntr, OutputUpdate);
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
					closeItemTag(xs, *lastlay);
					lastlay = 0;
				}
				
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
					}	else { // some kind of list
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
					text.outerFont(distance(begin, par)), sep);
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
				send = searchEnvironmentHtml(par, pend);
				par = makeEnvironmentHtml(buf, xs, runparams, text, par, send);
			}
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraphHtml(par, pend);
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
		buf.params().documentClass().counters().step(style.counter, OutputUpdate);

	openTag(xs, style, pbegin->params());

	// Label around sectioning number:
	// FIXME Probably need to account for LABEL_MANUAL
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
	LASSERT(bpit < epit, /* */);

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
			send = searchEnvironmentHtml(par, pend);
			par = makeEnvironmentHtml(buf, xs, ourparams, text, par, send);
			break;
		}
		case LATEX_BIB_ENVIRONMENT: {
			// FIXME Same fix here.
			send = searchEnvironmentHtml(par, pend);
			par = makeBibliography(buf, xs, ourparams, text, par, send);
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraphHtml(par, pend);
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
