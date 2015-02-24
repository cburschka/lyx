// -*- C++ -*-
/**
 * \file Context.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "tex2lyx.h"

#include <iosfwd>


namespace lyx {


/*!
 * Small helper struct that holds font properties.
 * The names are in LyX language, not LaTeX.
 * We don't use Font, because it pulls in a lot of dependencies and has
 * more strings than needed (e.g. font family error1 etc.).
 * If more font related stuff is needed, it might be good to change to
 * Font.
 */
class TeXFont {
public:
	TeXFont()
	{
		init();
	}
	void init()
	{
		size = "default";
		family = "default";
		series = "default";
		shape = "default";
		language = "english";
	}
	std::string size;
	std::string family;
	std::string series;
	std::string shape;
	std::string language;
};


bool operator==(TeXFont const &, TeXFont const &);


inline bool operator!=(TeXFont const & f1, TeXFont const & f2)
{
	return !operator==(f1, f2);
}


/// Output changed font parameters if \p oldfont and \p newfont differ
void output_font_change(std::ostream & os, TeXFont const & oldfont,
			TeXFont const & newfont);


/*!
 * A helper struct.
 *
 * Every bit of text has a corresponding context.
 * Usage: Parsing begins with a global context. A new context is opened for
 * every new LaTeX group, e.g. at the beginning of a new environment.
 * The old context is used again after the group is closed.
 *
 * Since not all paragraph parameters in LyX have the same scoping as their
 * LaTeX counterpart we may have to transfer context properties (e. g. the
 * font) from and to the parent context.
 */
class Context {
public:
	Context(bool need_layout_,
		TeX2LyXDocClass const & textclass_,
		Layout const * layout_ = 0,
		Layout const * parent_layout_= 0,
		TeXFont font_ = TeXFont());
	~Context();

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

	/*!
	 *  Add paragraph-level extra stuff if not already there. This
	 *  will be reset at the next check_layout()
	 */
	void add_par_extra_stuff(std::string const &);

	/// Do we need to output some \\begin_layout command before the
	/// next characters?
	bool need_layout;
	/// Do we need to output some \\end_layout command
	bool need_end_layout;
	/// We may need to add something after each \\begin_layout command
	std::string extra_stuff;
	/// We may need to add something after this \\begin_layout command
	std::string par_extra_stuff;
	/// If there has been an \\begin_deeper, we'll need a matching
	/// \\end_deeper
	bool need_end_deeper;
	/// If we are in an itemize-like environment, we need an \item
	/// for each paragraph, otherwise this has to be a deeper
	/// paragraph.
	bool has_item;
	/// we are handling a standard paragraph in an itemize-like
	/// environment
	bool deeper_paragraph;
	/*!
	 * Inside of unknown environments we may not allow font and layout
	 * changes.
	 * Otherwise things like
	 * \\large\\begin{foo}\\huge bar\\end{foo}
	 * would not work.
	 */
	bool new_layout_allowed;
	/// May -- be converted to endash and --- to emdash?
	bool merging_hyphens_allowed;
	/// Did we output anything yet in any context?
	static bool empty;

	/// The textclass of the document. Could actually be a global variable
	TeX2LyXDocClass const & textclass;
	/// The layout of the current paragraph
	Layout const * layout;
	/// The layout of the outer paragraph (for environment layouts)
	Layout const * parent_layout;
	/// font attributes of this context
	TeXFont font;
	/// font attributes of normal text
	static TeXFont normalfont;

private:
	void begin_layout(std::ostream & os, Layout const * const & l);
};


} // namespace lyx

#endif
