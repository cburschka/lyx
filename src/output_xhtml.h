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

#include "support/docstream.h"

#include <deque>
#include <vector>

namespace lyx {

class Buffer;
class OutputParams;
class Text;

// Inspiration for the *Tag structs and for XHTMLStream
// came from MathStream and its cousins.

struct StartTag {
	///
	StartTag(std::string const & tag) : tag_(tag) {}
	///
	StartTag(std::string const & tag, std::string const & attr, 
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
	EndTag(std::string tag) : tag_(tag) {}
	/// </tag_>
	docstring asEndTag() const;
	///
	std::string tag_;
};


// Tags like <img />
struct CompTag {
	///
	CompTag(std::string const & tag, std::string const & attr)
		: tag_(tag), attr_(attr) {}
	/// <tag_ attr_ />
	docstring asTag() const;
	///
	std::string tag_;
	///
	std::string attr_;
};


class XHTMLStream {
public:
	///
	explicit XHTMLStream(odocstream & os);
	///
	void cr();
	///
	odocstream & os() { return os_; }
	///
	// int & tab() { return tab_; }
	/// closes any font tags that are eligible to be closed,
	/// i.e., last on the tag_stack_.
	/// \return false if there are open font tags we could not close.
	/// because they are "blocked" by open non-font tags on the stack.
	bool closeFontTags();
	///
	XHTMLStream & operator<<(docstring const &);
	///
	XHTMLStream & operator<<(char_type);
	///
	XHTMLStream & operator<<(StartTag const &);
	///
	XHTMLStream & operator<<(EndTag const &);
	///
	XHTMLStream & operator<<(CompTag const &);
private:
	///
	void clearTagDeque();
	///
	bool isTagOpen(std::string const &);
	///
	odocstream & os_;
	///
	// int tab_;
	///
	typedef std::deque<StartTag> TagDeque;
	///
	typedef std::vector<StartTag> TagStack;
	/// holds start tags until we know there is content in them.
	TagDeque pending_tags_;
	/// remembers the history, so we can make sure we nest properly.
	TagStack tag_stack_;
};

///
void xhtmlParagraphs(Text const & text,
		       Buffer const & buf,
		       XHTMLStream & xs,
		       OutputParams const & runparams);

namespace html {
///
docstring escapeChar(char_type c);
/// converts a string to a form safe for links, etc
docstring htmlize(docstring const & str);

// to be removed
/// \return true if tag was opened, false if not 
bool openTag(odocstream & os, std::string const & tag, 
						 std::string const & attr);
/// \return true if tag was opened, false if not 
bool closeTag(odocstream & os, std::string const & tag);
}
} // namespace lyx

#endif
