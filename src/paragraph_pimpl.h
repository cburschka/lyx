// -*- C++ -*-
/**
 * \file paragraph_pimpl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPH_PIMPL_H
#define PARAGRAPH_PIMPL_H

#include "paragraph.h"

#include "changes.h"
#include "lyxfont.h"
#include "ParagraphParameters.h"

#include <boost/scoped_ptr.hpp>


namespace lyx {

class LyXLayout;


class Paragraph::Pimpl {
public:
	///
	Pimpl(Paragraph * owner);
	/// "Copy constructor"
	Pimpl(Pimpl const &, Paragraph * owner);

	//
	// Change tracking
	//
	/// look up change at given pos
	Change const lookupChange(pos_type pos) const;
	/// is there a change within the given range ?
	bool isChanged(pos_type start, pos_type end) const;
	/// will the paragraph be physically merged with the next
	/// one if the imaginary end-of-par character is logically deleted?
	bool isMergedOnEndOfParDeletion(bool trackChanges) const;
	/// set change for the entire par
	void setChange(Change const & change);
	/// set change at given pos
	void setChange(pos_type pos, Change const & change);
	/// accept changes within the given range
	void acceptChanges(pos_type start, pos_type end);
	/// reject changes within the given range
	void rejectChanges(pos_type start, pos_type end);

	///
	value_type getChar(pos_type pos) const;
	///
	void insertChar(pos_type pos, value_type c, Change const & change);
	///
	void insertInset(pos_type pos, InsetBase * inset, Change const & change);
	/// (logically) erase the char at pos; return true if it was actually erased
	bool eraseChar(pos_type pos, bool trackChanges);
	/// (logically) erase the given range; return the number of chars actually erased
	int eraseChars(pos_type start, pos_type end, bool trackChanges);
	///
	InsetBase * inset_owner;

	/** A font entry covers a range of positions. Notice that the
	    entries in the list are inserted in random order.
	    I don't think it's worth the effort to implement a more effective
	    datastructure, because the number of different fonts in a paragraph
	    is limited. (Asger)
	    Nevertheless, I decided to store fontlist using a sorted vector:
	    fontlist = { {pos_1,font_1} , {pos_2,font_2} , ... } where
	    pos_1 < pos_2 < ..., font_{i-1} != font_i for all i,
	    and font_i covers the chars in positions pos_{i-1}+1,...,pos_i
	    (font_1 covers the chars 0,...,pos_1) (Dekel)
	*/
	class FontTable  {
	public:
		///
		FontTable(pos_type p, LyXFont const & f)
			: pos_(p), font_(f)
		{}
		///
		pos_type pos() const { return pos_; }
		///
		void pos(pos_type p) { pos_ = p; }
		///
		LyXFont const & font() const { return font_; }
		///
		void font(LyXFont const & f) { font_ = f;}
	private:
		/// End position of paragraph this font attribute covers
		pos_type pos_;
		/** Font. Interpretation of the font values:
		    If a value is LyXFont::INHERIT_*, it means that the font
		    attribute is inherited from either the layout of this
		    paragraph or, in the case of nested paragraphs, from the
		    layout in the environment one level up until completely
		    resolved.
		    The values LyXFont::IGNORE_* and LyXFont::TOGGLE are NOT
		    allowed in these font tables.
		*/
		LyXFont font_;
	};
	///
	friend class matchFT;
	///
	class matchFT {
	public:
		/// used by lower_bound and upper_bound
		int operator()(FontTable const & a, FontTable const & b) const {
			return a.pos() < b.pos();
		}
	};

	///
	typedef std::vector<FontTable> FontList;
	///
	FontList fontlist;

	///
	void simpleTeXBlanks(odocstream &, TexRow & texrow,
			     pos_type const i,
			     unsigned int & column,
			     LyXFont const & font,
			     LyXLayout const & style);
	///
	void simpleTeXSpecialChars(Buffer const &, BufferParams const &,
				   odocstream &, TexRow & texrow,
				   OutputParams const &,
				   LyXFont & font, LyXFont & running_font,
				   LyXFont & basefont,
				   LyXFont const & outerfont,
				   bool & open_font,
				   Change::Type & running_change,
				   LyXLayout const & style,
				   pos_type & i,
				   unsigned int & column, value_type const c);

	///
	void validate(LaTeXFeatures & features,
		      LyXLayout const & layout) const;

	///
	unsigned int id_;
	///
	static unsigned int paragraph_id;
	///
	ParagraphParameters params;

private:
	///
	pos_type size() const { return owner_->size(); }
	/// match a string against a particular point in the paragraph
	bool isTextAt(std::string const & str, pos_type pos) const;

	/// for recording and looking up changes
	Changes changes_;

	/// Who owns us?
	Paragraph * owner_;
};


} // namespace lyx

#endif
