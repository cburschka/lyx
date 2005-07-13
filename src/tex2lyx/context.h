// -*- C++ -*-
/**
 * \file context.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "lyxtextclass.h"

#include <iosfwd>


/*!
 * Small helper struct that holds font properties.
 * The names are in LyX language, not LaTeX.
 * We don't use LyXFont, because it pulls in a lot of dependencies and has
 * more strings than needed (e.g. font family error1 etc.).
 * If more font related stuff is needed, it might be good to change to
 * LyXFont.
 */
class Font {
public:
	Font()
	{
		init();
	}
	void init()
	{
		size = "normal";
		family = "default";
		series = "default";
		shape = "default";
	}
	std::string size;
	std::string family;
	std::string series;
	std::string shape;
};


/// Output changed font parameters if \p oldfont and \p newfont differ
void output_font_change(std::ostream & os, Font const & oldfont,
                        Font const & newfont);


// A helper struct
class Context {
public:
	Context(bool need_layout_,
		LyXTextClass const & textclass_,
		LyXLayout_ptr layout_ = LyXLayout_ptr(),
		LyXLayout_ptr parent_layout_= LyXLayout_ptr(),
	        Font font_ = Font());

	/// Output a \\begin_layout if requested
	void check_layout(std::ostream & os);

	/// Output a \\end_layout if needed
	void check_end_layout(std::ostream & os);

	/// Output a \\begin_deeper if needed
	void check_deeper(std::ostream & os);

	/// Output a \\end_deeper if needed
	void check_end_deeper(std::ostream & os);

	/// dump content on stream (for debugging purpose), with
	/// description \c desc.
	void dump(std::ostream &, std::string const & desc = "context") const;

	/// Are we just beginning a new paragraph?
	bool atParagraphStart() const { return need_layout; }

	/// Begin an item in a list environment
	void set_item();

	/// Start a new paragraph
	void new_paragraph(std::ostream & os);

	/// Add extra stuff if not already there
	void add_extra_stuff(std::string const &);

	/// Do we need to output some \\begin_layout command before the
	/// next characters?
	bool need_layout;
	/// Do we need to output some \\end_layout command
	bool need_end_layout;
	/// We may need to add something after this \\begin_layout command
	std::string extra_stuff;
	/// If there has been an \\begin_deeper, we'll need a matching
	/// \\end_deeper
	bool need_end_deeper;
	/// If we are in an itemize-like environment, we need an \\item
	/// for each paragraph, otherwise this has to be a deeper
	/// paragraph.
	bool has_item;
	/// we are handling a standard paragraph in an itemize-like
	/// environment
	bool deeper_paragraph;

	/// The textclass of the document. Could actually be a global variable
	LyXTextClass const & textclass;
	/// The layout of the current paragraph
	LyXLayout_ptr layout;
	/// The layout of the outer paragraph (for environment layouts)
	LyXLayout_ptr parent_layout;
	/// font attributes of this context
	Font font;
	/// font attributes of normal text
	static Font normalfont;
};

#endif
