/**
 * \file xml.cpp
 * This file is part of LyX, the document processor.
 * License details can be found in the file COPYING.
 *
 * \author José Matos
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "xml.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Layout.h"
#include "Paragraph.h"
#include "Text.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <atomic>
#include <map>
#include <functional>
#include <QThreadStorage>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace xml {


docstring escapeChar(char_type c, XMLStream::EscapeSettings e)
{
	docstring str;
	switch (e) { // For HTML: always ESCAPE_NONE. For XML: it depends, hence the parameter.
	case XMLStream::ESCAPE_NONE:
	case XMLStream::ESCAPE_COMMENTS:
		str += c;
		break;
	case XMLStream::ESCAPE_ALL:
		if (c == '<') {
			str += "&lt;";
			break;
		} else if (c == '>') {
			str += "&gt;";
			break;
		}
		// fall through
	case XMLStream::ESCAPE_AND:
		if (c == '&')
			str += "&amp;";
		else
			str	+=c ;
		break;
	}
	return str;
}


docstring escapeChar(char c, XMLStream::EscapeSettings e)
{
	LATTEST(static_cast<unsigned char>(c) < 0x80);
	return escapeChar(static_cast<char_type>(c), e);
}


docstring escapeString(docstring const & raw, XMLStream::EscapeSettings e)
{
	docstring bin;
	bin.reserve(raw.size() * 2); // crude approximation is sufficient
	for (size_t i = 0; i != raw.size(); ++i) {
		char_type c = raw[i];
		if (e == XMLStream::ESCAPE_COMMENTS && c == '-' && i > 0 && raw[i - 1] == '-')
			bin += "&#45;";
		else
			bin += xml::escapeChar(c, e);
	}

	return bin;
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
	docstring output = '<' + tag_;
	if (!attr_.empty()) {
		docstring attributes = xml::trimLeft(xml::escapeString(attr_, XMLStream::ESCAPE_NONE));
		if (!attributes.empty())
			output += ' ' + attributes;
	}
	output += ">";
	return output;
}


docstring StartTag::writeEndTag() const
{
	return from_utf8("</") + tag_ + from_utf8(">");
}


bool StartTag::operator==(FontTag const &rhs) const
{
	return rhs == *this;
}


docstring EndTag::writeEndTag() const
{
	return from_utf8("</") + tag_ + from_utf8(">");
}


docstring CompTag::writeTag() const
{
	docstring output = '<' + tag_;
	if (!attr_.empty()) {
		// Erase the beginning of the attributes if it contains space characters: this function deals with that
		// automatically.
		docstring attributes = escapeString(attr_, XMLStream::ESCAPE_NONE);
		attributes.erase(attributes.begin(), std::find_if(attributes.begin(), attributes.end(),
                                                          [](char_type c) {return !isSpace(c);}));
		if (!attributes.empty()) {
			output += ' ' + attributes;
		}
	}
	output += " />";
	return output;
}


bool FontTag::operator==(StartTag const & tag) const
{
	FontTag const * const ftag = tag.asFontTag();
	if (!ftag)
		return false;
	return (font_type_ == ftag->font_type_);
}

} // namespace xml


void XMLStream::writeError(std::string const &s)
{
	LYXERR0(s);
	*this << ESCAPE_NONE << from_utf8("<!-- Output Error: " + s + " -->");
	*this << xml::CR();
}


void XMLStream::writeError(docstring const &s)
{
	LYXERR0(s);
	*this << ESCAPE_NONE << from_utf8("<!-- Output Error: ");
	*this << s;
	*this << ESCAPE_NONE << from_utf8(" -->");
	*this << xml::CR();
}


XMLStream::TagPtr XMLStream::getLastStackTag()
{
	return tag_stack_.back();
}


bool XMLStream::closeFontTags()
{
	if (isTagPending(xml::parsep_tag))
		// we haven't had any content
		return true;

	// this may be a useless check, since we ought at least to have
	// the parsep_tag. but it can't hurt too much to be careful.
	if (tag_stack_.empty())
		return true;

	// first, we close any open font tags we can close
	TagPtr *curtag = &tag_stack_.back();
	while ((*curtag)->asFontTag()) {
		if (**curtag != xml::parsep_tag)
			os_ << (*curtag)->writeEndTag();
		tag_stack_.pop_back();
		if (tag_stack_.empty())
			return true;
		curtag = &tag_stack_.back();
	}

	if (**curtag == xml::parsep_tag)
		return true;

	// so we've hit a non-font tag.
	writeError("Tags still open in closeFontTags(). Probably not a problem,\n"
					   "but you might want to check these tags:");
	TagDeque::const_reverse_iterator it = tag_stack_.rbegin();
	TagDeque::const_reverse_iterator const en = tag_stack_.rend();
	for (; it != en; ++it) {
		if (**it == xml::parsep_tag)
			break;
		writeError((*it)->tag_);
	}
	return false;
}


void XMLStream::startDivision(bool keep_empty)
{
	pending_tags_.push_back(makeTagPtr(xml::StartTag(xml::parsep_tag)));
	if (keep_empty)
		clearTagDeque();
}


void XMLStream::endDivision()
{
	if (isTagPending(xml::parsep_tag)) {
		// this case is normal. it just means we didn't have content,
		// so the parsep_tag never got moved onto the tag stack.
		while (!pending_tags_.empty()) {
			// clear all pending tags up to and including the parsep tag.
			// note that we work from the back, because we want to get rid
			// of everything that hasn't been used.
			TagPtr const cur_tag = pending_tags_.back();
			pending_tags_.pop_back();
			if (*cur_tag == xml::parsep_tag)
				break;
		}

#ifdef	XHTML_DEBUG
		dumpTagStack("EndDivision");
#endif

		return;
	}

	if (!isTagOpen(xml::parsep_tag)) {
		writeError("No division separation tag found in endDivision().");
		return;
	}

	// this case is also normal, if the parsep tag is the last one
	// on the stack. otherwise, it's an error.
	while (!tag_stack_.empty()) {
		TagPtr const cur_tag = tag_stack_.back();
		tag_stack_.pop_back();
		if (*cur_tag == xml::parsep_tag)
			break;
		writeError("Tag `" + cur_tag->tag_ + "' still open at end of paragraph. Closing.");
		os_ << cur_tag->writeEndTag();
	}

#ifdef	XHTML_DEBUG
	dumpTagStack("EndDivision");
#endif
}


void XMLStream::clearTagDeque()
{
	while (!pending_tags_.empty()) {
		TagPtr const & tag = pending_tags_.front();
		if (*tag != xml::parsep_tag)
			// tabs?
			os_ << tag->writeTag();
		tag_stack_.push_back(tag);
		pending_tags_.pop_front();
	}
}


XMLStream &XMLStream::operator<<(docstring const &d)
{
	is_last_tag_cr_ = false;
	clearTagDeque();
	os_ << xml::escapeString(d, escape_);
	escape_ = ESCAPE_ALL;
	return *this;
}


XMLStream &XMLStream::operator<<(const char *s)
{
	is_last_tag_cr_ = false;
	clearTagDeque();
	docstring const d = from_ascii(s);
	os_ << xml::escapeString(d, escape_);
	escape_ = ESCAPE_ALL;
	return *this;
}


XMLStream &XMLStream::operator<<(char_type c)
{
	is_last_tag_cr_ = false;
	clearTagDeque();
	os_ << xml::escapeChar(c, escape_);
	escape_ = ESCAPE_ALL;
	return *this;
}


XMLStream &XMLStream::operator<<(char c)
{
	is_last_tag_cr_ = false;
	clearTagDeque();
	os_ << xml::escapeChar(c, escape_);
	escape_ = ESCAPE_ALL;
	return *this;
}


XMLStream &XMLStream::operator<<(int i)
{
	is_last_tag_cr_ = false;
	clearTagDeque();
	os_ << i;
	escape_ = ESCAPE_ALL;
	return *this;
}


XMLStream &XMLStream::operator<<(EscapeSettings e)
{
	// Don't update is_last_tag_cr_ here, as this does not output anything.
	escape_ = e;
	return *this;
}


XMLStream &XMLStream::operator<<(xml::StartTag const &tag)
{
	is_last_tag_cr_ = false;
	if (tag.tag_.empty())
		return *this;
	pending_tags_.push_back(makeTagPtr(tag));
	if (tag.keepempty_)
		clearTagDeque();
	return *this;
}


XMLStream &XMLStream::operator<<(xml::ParTag const &tag)
{
	is_last_tag_cr_ = false;
	if (tag.tag_.empty())
		return *this;
	pending_tags_.push_back(makeTagPtr(tag));
	return *this;
}


XMLStream &XMLStream::operator<<(xml::CompTag const &tag)
{
	is_last_tag_cr_ = false;
	if (tag.tag_.empty())
		return *this;
	clearTagDeque();
	os_ << tag.writeTag();
	return *this;
}


XMLStream &XMLStream::operator<<(xml::FontTag const &tag)
{
	is_last_tag_cr_ = false;
	if (tag.tag_.empty())
		return *this;
	pending_tags_.push_back(makeTagPtr(tag));
	return *this;
}


XMLStream &XMLStream::operator<<(xml::CR const &)
{
	is_last_tag_cr_ = true;
	clearTagDeque();
	os_ << from_ascii("\n");
	return *this;
}


bool XMLStream::isTagOpen(xml::StartTag const &stag, int maxdepth) const
{
	auto sit = tag_stack_.begin();
	auto sen = tag_stack_.cend();
	for (; sit != sen && maxdepth != 0; ++sit) {
		if (**sit == stag)
			return true;
		maxdepth -= 1;
	}
	return false;
}


bool XMLStream::isTagOpen(xml::EndTag const &etag, int maxdepth) const
{
	auto sit = tag_stack_.begin();
	auto sen = tag_stack_.cend();
	for (; sit != sen && maxdepth != 0; ++sit) {
		if (etag == **sit)
			return true;
		maxdepth -= 1;
	}
	return false;
}


bool XMLStream::isTagPending(xml::StartTag const &stag, int maxdepth) const
{
	auto sit = pending_tags_.begin();
	auto sen = pending_tags_.cend();
	for (; sit != sen && maxdepth != 0; ++sit) {
		if (**sit == stag)
			return true;
		maxdepth -= 1;
	}
	return false;
}


// this is complicated, because we want to make sure that
// everything is properly nested. the code ought to make
// sure of that, but we won't assert (yet) if we run into
// a problem. we'll just output error messages and try our
// best to make things work.
XMLStream &XMLStream::operator<<(xml::EndTag const &etag)
{
	is_last_tag_cr_ = false;

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
				writeError("Tried to close pending tag `" + to_utf8(etag.tag_)
						   + "' when other tags were pending. Last pending tag is `"
						   + to_utf8(pending_tags_.back()->writeTag())
						   + "'. Tag discarded.");
				if (!pending_tags_.empty())
					pending_tags_.erase(dit);
				return *this;
			}
		}
		// so etag isn't itself pending. is it even open?
		if (!isTagOpen(etag)) {
			writeError("Tried to close `" + to_utf8(etag.tag_)
					   + "' when tag was not open. Tag discarded.");
			return *this;
		}
		// ok, so etag is open.
		// our strategy will be as below: we will do what we need to
		// do to close this tag.
		string estr = "Closing tag `" + to_utf8(etag.tag_)
					  + "' when other tags are pending. Discarded pending tags:\n";
		for (dit = pending_tags_.begin(); dit != den; ++dit)
			estr += to_utf8(xml::escapeString((*dit)->writeTag(), XMLStream::ESCAPE_ALL)) + "\n";
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
		TagPtr *curtag = &tag_stack_.back();
		// ...remembering them in a stack.
		TagDeque fontstack;
		while (etag != **curtag) {
			os_ << (*curtag)->writeEndTag();
			fontstack.push_back(*curtag);
			tag_stack_.pop_back();
			curtag = &tag_stack_.back();
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
	TagPtr *curtag = &tag_stack_.back();
	while (etag != **curtag) {
		writeError((*curtag)->tag_);
		if (**curtag != xml::parsep_tag)
			os_ << (*curtag)->writeEndTag();
		tag_stack_.pop_back();
		curtag = &tag_stack_.back();
	}
	// curtag is now the one we actually want.
	os_ << (*curtag)->writeEndTag();
	tag_stack_.pop_back();

	return *this;
}


docstring xml::uniqueID(docstring const & label)
{
	// thread-safe
	static atomic_uint seed(1000);
	return label + convert<docstring>(++seed);
}


bool xml::isNotOnlySpace(docstring const & str)
{
	for (auto const & c: str) {
		if (c != ' ' && c != '\t' && c != '\n' && c != '\v' && c != '\f' && c != '\r')
		return true;
	}
	return false;
}


docstring xml::trimLeft(docstring const & str)
{
	size_t i = 0;
	for (auto const & c: str) {
		if (c != ' ' && c != '\t' && c != '\n' && c != '\v' && c != '\f' && c != '\r')
			return str.substr(i, docstring::npos);
		i++;
	}
	return str;
}


docstring xml::cleanID(docstring const & orig)
{
	// The standard xml:id only allows letters, digits, '-' and '.' in a name.
	// This routine replaces illegal characters by '-' or '.' and adds a number for uniqueness if need be.

	// Use a cache of already mangled names: the alterations may merge several IDs as one. This ensures that the IDs
	// are not mixed up in the document.
	// This code could be improved: it uses Qt outside the GUI part. Any TLS implementation could do the trick.
	typedef map<docstring, docstring> MangledMap;
	static QThreadStorage<MangledMap> tMangledNames;
	static QThreadStorage<int> tMangleID;

	// If the name is already known, just return it.
	MangledMap & mangledNames = tMangledNames.localData();
	auto const known = mangledNames.find(orig);
	if (known != mangledNames.end())
		return known->second;

	// Start creating the mangled name by iterating over the characters.
	docstring content;
	auto it = orig.cbegin();
	auto end = orig.cend();

	// Make sure it starts with a letter.
	if (!isAlphaASCII(*it))
		content += "x";

	// Parse the ID character by character and change what needs to.
	bool mangle = false; // Indicates whether the ID had to be changed, i.e. if ID no more ensured to be unique.
	for (; it != end; ++it) {
		char_type c = *it;
		if (isAlphaASCII(c) || isDigitASCII(c) || c == '-' || c == '.' || c == '_') {
			content += c;
		} else if (c == ':' || c == ',' || c == ';' || c == '!') {
			mangle = true;
			content += ".";
		} else { // Other invalid characters, such as ' '.
			mangle = true;
			content += "-";
		}
	}

	// If there had to be a change, check if ID unicity is still guaranteed.
	// This avoids having a clash if satisfying XML requirements for ID makes two IDs identical, like "a:b" and "a!b",
	// as both of them would be transformed as "a.b". With this procedure, one will become "a.b" and the other "a.b-1".
	if (mangle && mangledNames.find(content) != mangledNames.end()) {
		int & mangleID = tMangleID.localData();
		if (mangleID > 0)
			content += "-" + convert<docstring>(mangleID);
		mangleID += 1;
	}

	// Save the new ID to avoid recomputing it afterwards and to ensure stability over the document.
	mangledNames[orig] = content;
	return content;
}


void xml::openTag(odocstream & os, string const & name, string const & attribute)
{
    // FIXME UNICODE
    // This should be fixed in layout files later.
    string param = subst(attribute, "<", "\"");
    param = subst(param, ">", "\"");

    // Note: we ignore the name if it empty or if it is a comment "<!-- -->" or
    // if the name is *dummy*.
    // We ignore dummy because dummy is not a valid docbook element and it is
    // the internal name given to single paragraphs in the latex output.
    // This allow us to simplify the code a lot and is a reasonable compromise.
    if (!name.empty() && name != "!-- --" && name != "dummy") {
        os << '<' << from_ascii(name);
        if (!param.empty())
            os << ' ' << from_ascii(param);
        os << '>';
    }
}


void xml::closeTag(odocstream & os, string const & name)
{
    if (!name.empty() && name != "!-- --" && name != "dummy")
        os << "</" << from_ascii(name) << '>';
}


void xml::openTag(Buffer const & buf, odocstream & os,
                   OutputParams const & runparams, Paragraph const & par)
{
    Layout const & style = par.layout();
    string const & name = style.latexname();
    string param = style.latexparam();
    Counters & counters = buf.params().documentClass().counters();

    string id = par.getID(buf, runparams);

    string attribute;
    if (!id.empty()) {
        if (param.find('#') != string::npos) {
            string::size_type pos = param.find("id=<");
            string::size_type end = param.find(">");
            if( pos != string::npos && end != string::npos)
                param.erase(pos, end-pos + 1);
        }
        attribute = id + ' ' + param;
    } else {
        if (param.find('#') != string::npos) {
            // FIXME UNICODE
            if (!style.counter.empty())
                // This uses InternalUpdate at the moment becuase xml output
                // does not do anything with tracked counters, and it would need
                // to track layouts if it did want to use them.
                counters.step(style.counter, InternalUpdate);
            else
                counters.step(from_ascii(name), InternalUpdate);
            int i = counters.value(from_ascii(name));
            attribute = subst(param, "#", convert<string>(i));
        } else {
            attribute = param;
        }
    }
    openTag(os, name, attribute);
}


void xml::closeTag(odocstream & os, Paragraph const & par)
{
    Layout const & style = par.layout();
    closeTag(os, style.latexname());
}


void openInlineTag(XMLStream & xs, const docstring & tag, const docstring & attr)
{
	xs << xml::StartTag(tag, attr);
}


void closeInlineTag(XMLStream & xs, const docstring & tag)
{
	xs << xml::EndTag(tag);
}


void openParTag(XMLStream & xs, const docstring & tag, const docstring & attr)
{
	if (!xs.isLastTagCR())
		xs << xml::CR();
	xs << xml::StartTag(tag, attr);
}


void closeParTag(XMLStream & xs, const docstring & tag)
{
	xs << xml::EndTag(tag);
	xs << xml::CR();
}


void openBlockTag(XMLStream & xs, const docstring & tag, const docstring & attr)
{
	if (!xs.isLastTagCR())
		xs << xml::CR();
	xs << xml::StartTag(tag, attr);
	xs << xml::CR();
}


void closeBlockTag(XMLStream & xs, const docstring & tag)
{
	if (!xs.isLastTagCR())
		xs << xml::CR();
	xs << xml::EndTag(tag);
	xs << xml::CR();
}


void xml::openTag(XMLStream & xs, const docstring & tag, const docstring & attr, const std::string & tagtype)
{
	if (tag.empty() || tag == "NONE") // Common check to be performed elsewhere, if it was not here.
		return;

	if (tag == "para" || tagtype == "paragraph") // Special case for <para>: always considered as a paragraph.
		openParTag(xs, tag, attr);
	else if (tagtype == "block")
		openBlockTag(xs, tag, attr);
	else if (tagtype == "inline")
		openInlineTag(xs, tag, attr);
	else if (tagtype == "none")
		xs << xml::StartTag(tag, attr);
	else
		xs.writeError("Unrecognised tag type '" + tagtype + "' for '" + to_utf8(tag) + " " + to_utf8(attr) + "'");
}


void xml::openTag(XMLStream & xs, const std::string & tag, const std::string & attr, const std::string & tagtype)
{
	xml::openTag(xs, from_utf8(tag), from_utf8(attr), tagtype);
}


void xml::openTag(XMLStream & xs, const docstring & tag, const std::string & attr, const std::string & tagtype)
{
	xml::openTag(xs, tag, from_utf8(attr), tagtype);
}


void xml::openTag(XMLStream & xs, const std::string & tag, const docstring & attr, const std::string & tagtype)
{
	xml::openTag(xs, from_utf8(tag), attr, tagtype);
}


void xml::closeTag(XMLStream & xs, const docstring & tag, const std::string & tagtype)
{
	if (tag.empty() || tag == "NONE" || tag == "IGNORE")
		return;

	if (tag == "para" || tagtype == "paragraph") // Special case for <para>: always considered as a paragraph.
		closeParTag(xs, tag);
	else if (tagtype == "block")
		closeBlockTag(xs, tag);
	else if (tagtype == "inline")
		closeInlineTag(xs, tag);
	else if (tagtype == "none")
		xs << xml::EndTag(tag);
	else
		xs.writeError("Unrecognised tag type '" + tagtype + "' for '" + to_utf8(tag) + "'");
}


void xml::closeTag(XMLStream & xs, const std::string & tag, const std::string & tagtype)
{
	xml::closeTag(xs, from_utf8(tag), tagtype);
}


void xml::compTag(XMLStream & xs, const docstring & tag, const docstring & attr, const std::string & tagtype)
{
	if (tag.empty() || tag == from_ascii("NONE"))
		return;

	// Special case for <para>: always considered as a paragraph.
	if (tag == from_ascii("para") || tagtype == "paragraph" || tagtype == "block") {
		if (!xs.isLastTagCR())
			xs << xml::CR();
		xs << xml::CompTag(tag, attr);
		xs << xml::CR();
	} else if (tagtype == "inline") {
		xs << xml::CompTag(tag, attr);
	} else {
		xs.writeError("Unrecognised tag type '" + tagtype + "' for '" + to_utf8(tag) + "'");
	}
}


void xml::compTag(XMLStream & xs, const std::string & tag, const std::string & attr, const std::string & tagtype)
{
	xml::compTag(xs, from_utf8(tag), from_utf8(attr), tagtype);
}


void xml::compTag(XMLStream & xs, const docstring & tag, const std::string & attr, const std::string & tagtype)
{
	xml::compTag(xs, tag, from_utf8(attr), tagtype);
}


void xml::compTag(XMLStream & xs, const std::string & tag, const docstring & attr, const std::string & tagtype)
{
	xml::compTag(xs, from_utf8(tag), attr, tagtype);
}


} // namespace lyx
