// -*- C++ -*-
/**
 * \file output_xhtml.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUT_XHTML_H
#define OUTPUT_XHTML_H

#include "LayoutEnums.h"
#include "support/docstream.h"
#include "support/strfwd.h"

#include <deque>

namespace lyx {

class Buffer;
class OutputParams;
class Text;

// Inspiration for the *Tag structs and for XHTMLStream
// came from MathStream and its cousins.

namespace html {
/// Attributes will be escaped automatically and so should NOT
/// be escaped before being passed to the constructor.
struct StartTag {
	///
	explicit StartTag(std::string const & tag) : tag_(tag), keepempty_(false) {}
	///
	explicit StartTag(std::string const & tag, std::string const & attr, 
		bool keepempty = false) 
		: tag_(tag), attr_(attr), keepempty_(keepempty) {}
	/// <tag_ attr_>
	docstring asTag() const;
	/// </tag_>
	docstring asEndTag() const;
	///
	std::string tag_;
	///
	std::string attr_;
	/// whether to keep things like "<tag></tag>" or discard them
	/// you would want this for td, e.g, but maybe not for a div
	bool keepempty_;
};


struct EndTag {
	///
	explicit EndTag(std::string tag) : tag_(tag) {}
	/// </tag_>
	docstring asEndTag() const;
	///
	std::string tag_;
};


// FIXME XHTML
// We need to allow these to be deferrable, which means it should
// inherit from StartTag. This is probably better, anyway, but we'll
// need to re-work a bit of code....
/// Tags like <img />
/// Attributes will be escaped automatically and so should NOT
/// be escaped before being passed to the constructor.
struct CompTag {
	///
	explicit CompTag(std::string const & tag)
		: tag_(tag) {}
	///
	explicit CompTag(std::string const & tag, std::string const & attr)
		: tag_(tag), attr_(attr) {}
	/// <tag_ attr_ />
	docstring asTag() const;
	///
	std::string tag_;
	///
	std::string attr_;
};

// trivial struct for output of newlines
struct CR{};

} // namespace html

class XHTMLStream {
public:
	///
	explicit XHTMLStream(odocstream & os);
	///
	odocstream & os() { return os_; }
	///
	// int & tab() { return tab_; }
	/// closes any font tags that are eligible to be closed,
	/// i.e., last on the tag_stack_.
	/// \return false if there are open font tags we could not close.
	/// because they are "blocked" by open non-font tags on the stack.
	bool closeFontTags();
	/// call at start of paragraph. sets a mark so we know what tags
	/// to close at the end. 
	void startParagraph(bool keep_empty);
	/// call at end of paragraph to clear that mark. note that this
	/// will also close any tags still open. 
	void endParagraph();
	///
	XHTMLStream & operator<<(docstring const &);
	///
	XHTMLStream & operator<<(const char *);
	///
	XHTMLStream & operator<<(char_type);
	///
	XHTMLStream & operator<<(int);
	///
	XHTMLStream & operator<<(char);
	///
	XHTMLStream & operator<<(html::StartTag const &);
	///
	XHTMLStream & operator<<(html::EndTag const &);
	///
	XHTMLStream & operator<<(html::CompTag const &);
	///
	XHTMLStream & operator<<(html::CR const &);
	///
	enum EscapeSettings {
		ESCAPE_NONE,
		ESCAPE_AND, // meaning &
		ESCAPE_ALL  // meaning <, >, &, at present
	};
	/// Sets what we are going to escape on the NEXT write.
	/// Everything is reset for the next time.
	XHTMLStream & operator<<(EscapeSettings);
private:
	///
	void clearTagDeque();
	///
	bool isTagOpen(std::string const &) const;
	///
	bool isTagPending(std::string const &) const;
	///
	void writeError(std::string const &) const;
	///
	odocstream & os_;
	///
	typedef std::deque<html::StartTag> TagStack;
	/// holds start tags until we know there is content in them.
	TagStack pending_tags_;
	/// remembers the history, so we can make sure we nest properly.
	TagStack tag_stack_;
	/// 
	EscapeSettings escape_;
};

///
void xhtmlParagraphs(Text const & text,
		       Buffer const & buf,
		       XHTMLStream & xs,
		       OutputParams const & runparams);

/// \return a string appropriate for setting alignment in CSS
/// Does NOT return "justify" for "block"
std::string alignmentToCSS(LyXAlignment align);

namespace html {
///
docstring escapeChar(char_type c, XHTMLStream::EscapeSettings e);
/// converts a string to a form safe for links, etc
docstring htmlize(docstring const & str, XHTMLStream::EscapeSettings e);
/// cleans \param str for use as an atttribute by replacing
/// all non-alnum by "_"
docstring cleanAttr(docstring const & str);
///
std::string escapeChar(char c, XHTMLStream::EscapeSettings e);
/// 
std::string htmlize(std::string const & str, XHTMLStream::EscapeSettings e);
/// 
std::string cleanAttr(std::string const & str);

} // namespace html
} // namespace lyx

#endif
