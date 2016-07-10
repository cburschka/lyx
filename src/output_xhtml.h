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
#include <memory>


namespace lyx {

class Buffer;
class OutputParams;
class Text;

// Inspiration for the *Tag structs and for XHTMLStream
// came from MathStream and its cousins.

namespace html {

struct FontTag;
struct EndFontTag;

/// Attributes will be escaped automatically and so should NOT
/// be escaped before being passed to the constructor.
struct StartTag
{
	///
	explicit StartTag(std::string const & tag) : tag_(tag), keepempty_(false) {}
	///
	explicit StartTag(std::string const & tag, std::string const & attr,
		bool keepempty = false)
		: tag_(tag), attr_(attr), keepempty_(keepempty) {}
	///
	virtual ~StartTag() {}
	/// <tag_ attr_>
	virtual docstring writeTag() const;
	/// </tag_>
	virtual docstring writeEndTag() const;
	///
	virtual FontTag const * asFontTag() const { return 0; }
	///
	virtual bool operator==(StartTag const & rhs) const
		{ return tag_ == rhs.tag_; }
	///
	virtual bool operator!=(StartTag const & rhs) const
		{ return !(*this == rhs); }
	///
	virtual bool operator==(FontTag const & rhs) const;
	///
	std::string tag_;
	///
	std::string attr_;
	/// whether to keep things like "<tag></tag>" or discard them
	/// you would want this for td, e.g, but maybe not for a div
	bool keepempty_;
};


///
struct EndTag
{
	///
	explicit EndTag(std::string tag) : tag_(tag) {}
	///
	virtual ~EndTag() {}
	/// </tag_>
	virtual docstring writeEndTag() const;
	///
	bool operator==(StartTag const & rhs) const
		{ return tag_ == rhs.tag_; }
	///
	bool operator!=(StartTag const & rhs) const
		{ return !(*this == rhs); }
	///
	virtual EndFontTag const * asFontTag() const { return 0; }
	///
	std::string tag_;
};


/// Tags like <img />
/// Attributes will be escaped automatically and so should NOT
/// be escaped before being passed to the constructor.
struct CompTag
{
	///
	explicit CompTag(std::string const & tag)
		: tag_(tag) {}
	///
	explicit CompTag(std::string const & tag, std::string const & attr)
		: tag_(tag), attr_(attr) {}
	/// <tag_ attr_ />
	docstring writeTag() const;
	///
	std::string tag_;
	///
	std::string attr_;
};


/// A special case of StartTag, used exclusively for tags that wrap paragraphs.
struct ParTag : public StartTag
{
	///
	explicit ParTag(std::string const & tag, std::string attr,
	       std::string const & parid);
	///
	~ParTag() {}
};


///
enum FontTypes {
	// ranges
	FT_EMPH,
	FT_NOUN,
	FT_UBAR,
	FT_DBAR,
	FT_WAVE,
	FT_SOUT,
	// bold
	FT_BOLD,
	// shapes
	FT_UPRIGHT,
	FT_ITALIC,
	FT_SLANTED,
	FT_SMALLCAPS,
	// families
	FT_ROMAN,
	FT_SANS,
	FT_TYPE,
	// sizes
	FT_SIZE_TINY,
	FT_SIZE_SCRIPT,
	FT_SIZE_FOOTNOTE,
	FT_SIZE_SMALL,
	FT_SIZE_NORMAL,
	FT_SIZE_LARGE,
	FT_SIZE_LARGER,
	FT_SIZE_LARGEST,
	FT_SIZE_HUGE,
	FT_SIZE_HUGER,
	FT_SIZE_INCREASE,
	FT_SIZE_DECREASE
};


///
struct FontTag : public StartTag
{
	///
	explicit FontTag(FontTypes type);
	///
	FontTag const * asFontTag() const { return this; }
	///
	bool operator==(StartTag const &) const;
	///
	FontTypes font_type_;
};


///
struct EndFontTag : public EndTag
{
	///
	explicit EndFontTag(FontTypes type);
	///
	EndFontTag const * asFontTag() const { return this; }
	///
	FontTypes font_type_;
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
	/// sets a mark so we know what tags to close at the end.
	/// normally called at the start of a paragraph.
	void startDivision(bool keep_empty);
	/// clears the mark set by previous method.
	/// there should not be any other tags open before it on the stack,
	/// but if there are, we will close them.
	void endDivision();
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
	XHTMLStream & operator<<(html::ParTag const &);
	///
	XHTMLStream & operator<<(html::FontTag const &);
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
	/// This routine is for debugging the tag stack, etc. Code
	/// for it is disabled by default, however, so you will need
	/// to enable it if you want to use it.
	void dumpTagStack(std::string const & msg);
private:
	///
	void clearTagDeque();
	///
	bool isTagOpen(html::StartTag const &) const;
	///
	bool isTagOpen(html::EndTag const &) const;
	///
	bool isTagPending(html::StartTag const &) const;
	///
	void writeError(std::string const &) const;
	///
	odocstream & os_;
	///
	EscapeSettings escape_;
	// What we would really like to do here is simply use a
	// deque<StartTag>. But we want to store both StartTags and
	// sub-classes thereof on this stack, which means we run into the
	// so-called polymorphic class problem with the STL. We therefore have
	// to use a deque<StartTag *>, which leads to the question who will
	// own these pointers and how they will be deleted, so we use shared
	// pointers.
	///
	typedef std::shared_ptr<html::StartTag> TagPtr;
	typedef std::deque<TagPtr> TagDeque;
	///
	template <typename T>
	TagPtr makeTagPtr(T const & tag) { return std::make_shared<T>(tag); }
	///
	TagDeque pending_tags_;
	///
	TagDeque tag_stack_;
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
/// \p c must be ASCII
docstring escapeChar(char c, XHTMLStream::EscapeSettings e);

} // namespace html
} // namespace lyx

#endif
