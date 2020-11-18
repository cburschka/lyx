// -*- C++ -*-
/**
 * \file xml.h
 * This file is part of LyX, the document processor.
 * License details can be found in the file COPYING.
 *
 * \author José Matos
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XML_H
#define XML_H

#include "support/docstring.h"

#include <deque>
#include <memory>

namespace lyx {

class Buffer;
class Paragraph;
class OutputParams;

// Inspiration for the *Tag structs and for XMLStream
// came from MathStream and its cousins.

namespace xml {
struct StartTag;
struct EndTag;
struct CompTag;
struct ParTag;
struct FontTag;
struct CR;
}

class XMLStream {
public:
	///
	explicit XMLStream(odocstream & os): os_(os), escape_(ESCAPE_ALL), is_last_tag_cr_(true) {}
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
	XMLStream & operator<<(docstring const &);
	///
	XMLStream & operator<<(const char *);
	///
	XMLStream & operator<<(char_type);
	///
	XMLStream & operator<<(int);
	///
	XMLStream & operator<<(char);
	///
	XMLStream & operator<<(xml::StartTag const &);
	///
	XMLStream & operator<<(xml::EndTag const &);
	///
	XMLStream & operator<<(xml::CompTag const &);
	///
	XMLStream & operator<<(xml::ParTag const &);
	///
	XMLStream & operator<<(xml::FontTag const &);
	///
	XMLStream & operator<<(xml::CR const &);
	///
	enum EscapeSettings {
		ESCAPE_NONE,
		ESCAPE_AND, // meaning &
		ESCAPE_ALL, // meaning <, >, &, at present, except things that are forbidden in comments
		ESCAPE_COMMENTS // Anything that is forbidden within comments
	};
	/// Sets what we are going to escape on the NEXT write.
	/// Everything is reset for the next time.
	XMLStream & operator<<(EscapeSettings);
	/// This routine is for debugging the tag stack, etc. Code
	/// for it is disabled by default, however, so you will need
	/// to enable it if you want to use it.
	void dumpTagStack(std::string const & msg);
	///
	bool isTagOpen(xml::StartTag const &, int maxdepth = -1) const;
	///
	bool isTagOpen(xml::EndTag const &, int maxdepth = -1) const;
	///
	bool isTagPending(xml::StartTag const &, int maxdepth = -1) const;
	/// Is the last tag that was added to the stream a new line (CR)? This is mostly to known
	/// whether a new line must be added. Therefore, consider that an empty stream just had a CR,
	/// that simplifies the logic using this code.
	bool isLastTagCR() const { return is_last_tag_cr_; };
	///
	void writeError(std::string const &);
	///
	void writeError(docstring const &);
	///
	typedef std::shared_ptr<xml::StartTag> TagPtr;
	/// Returns the last element on the tag stack. XMLStream keeps ownership of the item.
	TagPtr getLastStackTag();
private:
	///
	void clearTagDeque();
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
	typedef std::deque<TagPtr> TagDeque;
	///
	template <typename T>
	TagPtr makeTagPtr(T const & tag) { return std::make_shared<T>(tag); }
	///
	TagDeque pending_tags_;
	///
	TagDeque tag_stack_;
	///
	bool is_last_tag_cr_;
};

namespace xml {

/// Escape the given character, if necessary, to an entity.
docstring escapeChar(char_type c, XMLStream::EscapeSettings e);

/// Escape the given character, if necessary, to an entity.
docstring escapeChar(char c, XMLStream::EscapeSettings e);

/// Escape a word instead of a single character
docstring escapeString(docstring const & raw, XMLStream::EscapeSettings e=XMLStream::ESCAPE_ALL);

/// cleans \param str for use as an attribute by replacing all non-altnum by "_"
docstring cleanAttr(docstring const & str);

/// \p c must be ASCII
docstring escapeChar(char c, XMLStream::EscapeSettings e);

/// replaces illegal characters from ID attributes
docstring cleanID(docstring const &orig);

/// returns a unique numeric ID
docstring uniqueID(docstring const & label);

/// determines whether a string only contains space characters
bool isNotOnlySpace(docstring const & str);

/// trims the string to the left, i.e. remove any space-like character at the beginning of the string
docstring trimLeft(docstring const & str);

struct FontTag;
struct EndFontTag;

/// Attributes will be escaped automatically and so should NOT
/// be escaped before being passed to the constructor.
struct StartTag
{
	///
	explicit StartTag(std::string const & tag) : tag_(from_ascii(tag)), keepempty_(false), tagtype_("none") {}
	///
	explicit StartTag(docstring const & tag) : tag_(tag), keepempty_(false), tagtype_("none") {}
	///
	explicit StartTag(docstring const & tag, docstring const & attr,
					  bool keepempty = false, std::string const & tagtype = "none")
			: tag_(tag), attr_(attr), keepempty_(keepempty), tagtype_(tagtype) {}
	///
	explicit StartTag(std::string const & tag, std::string const & attr,
					  bool keepempty = false, std::string const & tagtype = "none")
			: tag_(from_ascii(tag)), attr_(from_utf8(attr)), keepempty_(keepempty), tagtype_(tagtype) {}
	///
	explicit StartTag(std::string const & tag, docstring const & attr,
					  bool keepempty = false, std::string const & tagtype = "none")
			: tag_(from_ascii(tag)), attr_(attr), keepempty_(keepempty), tagtype_(tagtype) {}
	///
	virtual ~StartTag() = default;
	/// <tag_ attr_>
	virtual docstring writeTag() const;
	/// </tag_>
	virtual docstring writeEndTag() const;
	///
	virtual FontTag const * asFontTag() const { return nullptr; }
	///
	virtual bool operator==(StartTag const & rhs) const
	{ return tag_ == rhs.tag_; }
	///
	virtual bool operator!=(StartTag const & rhs) const
	{ return !(*this == rhs); }
	///
	virtual bool operator==(FontTag const & rhs) const;
	///
	docstring tag_;
	///
	docstring attr_;
	/// whether to keep things like "<tag></tag>" or discard them
	/// you would want this for td, e.g, but maybe not for a div
	bool keepempty_;
	/// Type of tag for new-line behaviour. Either "paragraph", "inline", "block", or "none" (default).
	std::string tagtype_;
};


///
struct EndTag
{
	///
	explicit EndTag(std::string const & tag, std::string const & tagtype = "none")
	        : tag_(from_ascii(tag)), tagtype_(tagtype) {}
	///
	explicit EndTag(docstring const & tag, std::string const & tagtype = "none")
	        : tag_(tag), tagtype_(tagtype) {}
	///
	virtual ~EndTag() = default;
	/// </tag_>
	virtual docstring writeEndTag() const;
	///
	bool operator==(StartTag const & rhs) const
	{ return tag_ == rhs.tag_; }
	///
	bool operator!=(StartTag const & rhs) const
	{ return !(*this == rhs); }
	///
	virtual EndFontTag const * asFontTag() const { return nullptr; }
	///
	docstring tag_;
	/// Type of tag for new-line behaviour. Either "paragraph", "inline", "block", or "none" (default).
	/// The value should match that of the corresponding xml::StartTag.
	std::string tagtype_;
};


/// Tags like <img />
/// Attributes will be escaped automatically and so should NOT
/// be escaped before being passed to the constructor.
struct CompTag
{
	///
	explicit CompTag(std::string const & tag)
			: tag_(from_utf8(tag)), tagtype_("none") {}
	///
	explicit CompTag(std::string const & tag, std::string const & attr, std::string const & tagtype = "none")
			: tag_(from_utf8(tag)), attr_(from_utf8(attr)), tagtype_(tagtype) {}
	///
	explicit CompTag(std::string const & tag, docstring const & attr, std::string const & tagtype = "none")
			: tag_(from_utf8(tag)), attr_(attr), tagtype_(tagtype) {}
	/// <tag_ attr_ />
	docstring writeTag() const;
	///
	docstring tag_;
	///
	docstring attr_;
	/// Type of tag for new-line behaviour. Either "paragraph", "inline", "block", or "none" (default).
	std::string tagtype_;
};


/// A special case of StartTag, used exclusively for tags that wrap paragraphs.
struct ParTag : public StartTag
{
	///
	explicit ParTag(std::string const & tag, const std::string & attr): StartTag(tag, from_utf8(attr)) {}
	///
	~ParTag() override = default;
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
	FT_XOUT,
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

	// When updating this list, also update fontToTag in both output_docbook.cpp and output_xhtml.cpp,
	// fontToRole in output_docbook.cpp, and fontToAttribute in output_xhtml.cpp.
};


///
struct FontTag : public StartTag
{
	///
	FontTag(docstring const & tag, FontTypes type): StartTag(tag), font_type_(type) {}
	///
	FontTag(std::string const & tag, FontTypes type): StartTag(from_utf8(tag)), font_type_(type) {}
	///
	FontTag(docstring const & tag, docstring const & attr, FontTypes type): StartTag(tag, attr), font_type_(type) {}
	///
	FontTag(std::string const & tag, std::string const & attr, FontTypes type): StartTag(from_utf8(tag), from_utf8(attr)), font_type_(type) {}
	///
	FontTag const * asFontTag() const override { return this; }
	///
	bool operator==(StartTag const &) const override;
	///
	FontTypes font_type_;
};


///
struct EndFontTag : public EndTag
{
	///
	EndFontTag(docstring const & tag, FontTypes type): EndTag(tag), font_type_(type) {}
	///
	EndFontTag(std::string const & tag, FontTypes type): EndTag(from_utf8(tag)), font_type_(type) {}
	///
	EndFontTag const * asFontTag() const override { return this; }
	///
	FontTypes font_type_;
};


// trivial struct for output of newlines
struct CR{};

// an illegal tag for internal use
xml::StartTag const parsep_tag("&LyX_parsep_tag&");

/// Open tag
void openTag(odocstream & os, std::string const & name,
             std::string const & attribute = std::string());

/// Open tag
void openTag(Buffer const & buf, odocstream & os,
             OutputParams const & runparams, Paragraph const & par);

/// Close tag
void closeTag(odocstream & os, std::string const & name);

/// Close tag
void closeTag(odocstream & os, Paragraph const & par);

} // namespace xml

} // namespace lyx

#endif // XML_H
