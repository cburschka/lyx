// -*- C++ -*-
/**
 *  \file paragraph.h
 *  Copyright 1995 Matthias Ettrich
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 */

#ifndef PARAGRAPH_H
#define PARAGRAPH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "insets/inset.h" // Just for Inset::Code
#include "lyxfont.h" // Just for LyXFont::FONT_SIZE
#include "support/types.h"

class ParagraphParameters;
class BufferParams;
class TexRow;
class LaTeXFeatures;
class InsetBibKey;
class BufferView;
class Language;

// After 1.2.0 is released, during 1.3.0cvs, we enable this. And after
// a while we verify that reading of 1.2.x files work perfectly we remove
// this code completely. (Lgb)
#define NO_PEXTRA_REALLY 1

// Define this if you want to try out the new storage container for
// paragraphs. std::container instead of obfuscated homegrown
// linked list. (Lgb)
// This is non working and far from finished.
// #define NO_NEXT 1

/// A Paragraph holds all text, attributes and insets in a text paragraph
class Paragraph  {
public:
#ifndef NO_PEXTRA_REALLY
	///
	enum PEXTRA_TYPE {
		///
		PEXTRA_NONE,
		///
		PEXTRA_INDENT,
		///
		PEXTRA_MINIPAGE,
		///
		PEXTRA_FLOATFLT
	};
	///
	enum MINIPAGE_ALIGNMENT {
		///
		MINIPAGE_ALIGN_TOP,
		///
		MINIPAGE_ALIGN_MIDDLE,
		///
		MINIPAGE_ALIGN_BOTTOM
	};
#endif
	///
	enum META_KIND {
		///
		META_HFILL = 1,
		///
		META_NEWLINE,
		///
		META_INSET
	};
	///
	typedef char value_type;
	/// The same as ParameterStruct::depth_type
	typedef unsigned int depth_type;

	///
	Paragraph();
	/// this constructor inserts the new paragraph in a list
	explicit
	Paragraph(Paragraph * par);
	///
	Paragraph(Paragraph const &, bool same_ids);
	/// the destructor removes the new paragraph from the list
	~Paragraph();

	///
	Language const * getParLanguage(BufferParams const &) const;
	///
	bool isRightToLeftPar(BufferParams const &) const;
	///
	void changeLanguage(BufferParams const & bparams,
			    Language const * from, Language const * to);
	///
	bool isMultiLingual(BufferParams const &);

	///
	string const asString(Buffer const *, bool label);
	///
	string const asString(Buffer const *, lyx::pos_type beg, lyx::pos_type end,
			      bool label);

	///
	void writeFile(Buffer const *, std::ostream &, BufferParams const &,
		       depth_type) const;
	///
	void validate(LaTeXFeatures &) const;

	///
	int id() const;
	///
	void id(int id_arg);
	///
	void read();

	///
	Paragraph * TeXOnePar(Buffer const *, BufferParams const &,
				 std::ostream &, TexRow & texrow,
				 bool moving_arg);

	///
	int startTeXParParams(BufferParams const &, std::ostream &) const;

	///
	int endTeXParParams(BufferParams const &, std::ostream &) const;


	///
	bool simpleTeXOnePar(Buffer const *, BufferParams const &,
			     std::ostream &, TexRow & texrow, bool moving_arg);

	///
	Paragraph * TeXEnvironment(Buffer const *, BufferParams const &,
				      std::ostream &, TexRow & texrow);
	///
	bool hasSameLayout(Paragraph const * par) const;

	///
	void makeSameLayout(Paragraph const * par);

	/// Is it the first par with same depth and layout?
	bool isFirstInSequence() const;

	/** Check if the current paragraph is the last paragraph in a
	    proof environment */
	int getEndLabel(BufferParams const &) const;
	///
	Inset * inInset() const;
	///
	void setInsetOwner(Inset * i);
	///
	void deleteInsetsLyXText(BufferView *);
	///
	void resizeInsetsLyXText(BufferView *);

	///
	lyx::pos_type size() const;
	///
	void setContentsFromPar(Paragraph * par);
	///
	void clearContents();

	///
	string const & layout() const;
	///
	void layout(string const & new_layout);

	///
	void setCounter(int i, int v);
	///
	int getCounter(int i) const;
	///
	void incCounter(int i);

	///
	char enumdepth;

	///
	char itemdepth;

	///
	InsetBibKey * bibkey;  // ale970302

	///
	void next(Paragraph *);
	/** these function are able to hide closed footnotes
	 */
	Paragraph * next();
	///
	Paragraph const * next() const;

	///
	void previous(Paragraph *);
	///
	Paragraph * previous();
	///
	Paragraph const * previous() const;

	/// for the environments
	Paragraph * depthHook(depth_type depth);
	/// for the environments
	Paragraph const * depthHook(depth_type depth) const;
	///
	Paragraph * outerHook();
	///
	Paragraph const * outerHook() const;
	///
	int beginningOfMainBody() const;
	///
	string const & getLabelstring() const;

	/// the next two functions are for the manual labels
	string const getLabelWidthString() const;
	///
	void setLabelWidthString(string const & s);
	///
	char getAlign() const;
	/// The nesting depth of a paragraph
	depth_type getDepth() const;
	/// The maximal possible depth of a paragraph after this one
	depth_type getMaxDepthAfter(Buffer const *) const;
	///
	void applyLayout(string const & new_layout);
	///
	int getFirstCounter(int i) const;
	///
	void erase(lyx::pos_type pos);
	/** the flag determines wether the layout should be copied
	 */
	void breakParagraph(BufferParams const &, lyx::pos_type pos, int flag);
	///
	void breakParagraphConservative(BufferParams const &, lyx::pos_type pos);
	/** Get unistantiated font setting. Returns the difference
	    between the characters font and the layoutfont.
	    This is what is stored in the fonttable
	*/
	LyXFont const
	getFontSettings(BufferParams const &, lyx::pos_type pos) const;
	///
	LyXFont const getFirstFontSettings() const;

	/** Get fully instantiated font. If pos == -1, use the layout
	    font attached to this paragraph.
	    If pos == -2, use the label font of the layout attached here.
	    In all cases, the font is instantiated, i.e. does not have any
	    attributes with values LyXFont::INHERIT, LyXFont::IGNORE or
	    LyXFont::TOGGLE.
	*/
	LyXFont const getFont(BufferParams const &, lyx::pos_type pos) const;
	LyXFont const getLayoutFont(BufferParams const &) const;
	LyXFont const getLabelFont(BufferParams const &) const;
	///
	value_type getChar(lyx::pos_type pos) const;
	///
	value_type getUChar(BufferParams const &, lyx::pos_type pos) const;
	/// The position must already exist.
	void setChar(lyx::pos_type pos, value_type c);
	/// pos <= size() (there is a dummy font change at the end of each par)
	void setFont(lyx::pos_type pos, LyXFont const & font);
	/// Returns the height of the highest font in range
	LyXFont::FONT_SIZE highestFontInRange(lyx::pos_type startpos,
					      lyx::pos_type endpos,
					      LyXFont::FONT_SIZE const def_size) const;
	///
	void insertChar(lyx::pos_type pos, value_type c);
	///
	void insertChar(lyx::pos_type pos, value_type c, LyXFont const &);
	///
	bool checkInsertChar(LyXFont &);
	///
	void insertInset(lyx::pos_type pos, Inset * inset);
	///
	void insertInset(lyx::pos_type pos, Inset * inset, LyXFont const &);
	///
	bool insetAllowed(Inset::Code code);
	///
	Inset * getInset(lyx::pos_type pos);
	///
	Inset const * getInset(lyx::pos_type pos) const;
	/** important for cut and paste
	    Temporary change from BufferParams to Buffer. Will revert when we
	    get rid of the argument to Inset::clone(Buffer const &) */
	void copyIntoMinibuffer(Buffer const &, lyx::pos_type pos) const;
	///
	void cutIntoMinibuffer(BufferParams const &, lyx::pos_type pos);
	///
	bool insertFromMinibuffer(lyx::pos_type pos);

	///
	bool isHfill(lyx::pos_type pos) const;
	///
	bool isInset(lyx::pos_type pos) const;
	///
	bool isNewline(lyx::pos_type pos) const;
	///
	bool isSeparator(lyx::pos_type pos) const;
	///
	bool isLineSeparator(lyx::pos_type pos) const;
	///
	bool isKomma(lyx::pos_type pos) const;
	/// Used by the spellchecker
	bool isLetter(lyx::pos_type pos) const;
	///
	bool isWord(lyx::pos_type pos) const;

	/** paste this paragraph with the next one
	    be carefull, this doesent make any check at all
	*/
	void pasteParagraph(BufferParams const &);

	/// returns -1 if inset not found
	int getPositionOfInset(Inset const * inset) const;

	/// some good comment here John?
	Paragraph * getParFromID(int id) const;

	///
	int stripLeadingSpaces(lyx::textclass_type tclass);

#ifndef NO_PEXTRA_REALLY
	/* If I set a PExtra Indent on one paragraph of a ENV_LIST-TYPE
	   I have to set it on each of it's elements */
	///
	void setPExtraType(BufferParams const &, int type,
			   string const & width, string const & widthp);
	///
	void unsetPExtraType(BufferParams const &);
#endif
	///
	bool sgmlConvertChar(char c, string & sgml_string);
	///
	bool isFreeSpacing() const;

	ParagraphParameters & params();
	ParagraphParameters const & params() const;
private:
	///
	string layout_;
public:
	/** Both these definitions must be made public to keep Compaq cxx 6.5
	 *  happy.
	 */
	///
	struct InsetTable {
		///
		lyx::pos_type pos;
		///
		Inset * inset;
		///
		InsetTable(lyx::pos_type p, Inset * i) : pos(p), inset(i) {}
	};

	///
	typedef std::vector<InsetTable> InsetList;
private:
	///
	InsetList insetlist;
public:
	///
	class inset_iterator {
	public:
		///
		inset_iterator() {}
		//
		inset_iterator(InsetList::iterator const & iter);
		///
		inset_iterator & operator++() {
			++it;
			return *this;
		}
		///
		Inset * operator*() { return it->inset; }
		///
		Inset * operator->() { return it->inset; }

		///
		lyx::pos_type getPos() const { return it->pos; }
		///
		bool operator==(inset_iterator const & iter) const {
			return it == iter.it;
		}
		///
		bool operator!=(inset_iterator const & iter) const {
			return it != iter.it;
		}
	private:
		///
		InsetList::iterator it;
	};
	///
	friend class inset_iterator;

	///
	inset_iterator inset_iterator_begin();
	///
	inset_iterator inset_iterator_end();
	/// returns inset iterator of the first inset at or after pos.
	inset_iterator InsetIterator(lyx::pos_type pos);

private:
	/// if anything uses this we don't want it to.
	Paragraph(Paragraph const &);
	///
	Paragraph * next_;
	///
	Paragraph * previous_;

	struct Pimpl;
	///
	friend struct Paragraph::Pimpl;
	///
	Pimpl * pimpl_;
};

#endif
