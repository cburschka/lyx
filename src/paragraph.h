// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef PARAGRAPH_H
#define PARAGRAPH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include <vector>

#include "insets/inset.h"
#include "layout.h"

class ParagraphParameters;
class BufferParams;
class TexRow;
struct LaTeXFeatures;
class InsetBibKey;
class BufferView;
class Language;

// After 1.2.0 is released, during 1.3.0cvs, we enable this. And after
// a while we verify that reading of 1.2.x files work perfectly we remove
// this code completely. (Lgb)
//#define NO_PEXTRA_REALLY 1

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
	typedef std::vector<value_type> TextContainer;
	///
	/* This should be TextContainer::size_type, but we need
	   signed values for now.
	*/
	typedef TextContainer::difference_type size_type;

	///
	Paragraph();
	/// this constructor inserts the new paragraph in a list
	explicit
	Paragraph(Paragraph * par);
	///
	Paragraph(Paragraph const &);
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
	string const asString(Buffer const *, size_type beg, size_type end);
	
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
	Inset * InInset();
	///
	void setInsetOwner(Inset * i);
	///
	void deleteInsetsLyXText(BufferView *);
	///
	void resizeInsetsLyXText(BufferView *);

	///
	size_type size() const;
	///
	void setContentsFromPar(Paragraph * par);
	///
	void clearContents();

	///
	LyXTextClass::LayoutList::size_type layout;

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
	LyXTextClass::LayoutList::size_type getLayout() const;
	///
	char getAlign() const;
	///
	depth_type getDepth() const;
	///
	void setLayout(LyXTextClass::LayoutList::size_type new_layout);
	///
	void setOnlyLayout(LyXTextClass::LayoutList::size_type new_layout);
	///
	int getFirstCounter(int i) const;
	///
	void erase(size_type pos);
	/** the flag determines wether the layout should be copied
	 */ 
	void breakParagraph(BufferParams const &, size_type pos, int flag);
	///
	void breakParagraphConservative(BufferParams const &, size_type pos);
	/** Get unistantiated font setting. Returns the difference
	    between the characters font and the layoutfont.
	    This is what is stored in the fonttable
	*/
	LyXFont const
	getFontSettings(BufferParams const &, size_type pos) const;
	///
	LyXFont const getFirstFontSettings() const;

	/** Get fully instantiated font. If pos == -1, use the layout
	    font attached to this paragraph.
	    If pos == -2, use the label font of the layout attached here.
	    In all cases, the font is instantiated, i.e. does not have any
	    attributes with values LyXFont::INHERIT, LyXFont::IGNORE or 
	    LyXFont::TOGGLE.
	*/
	LyXFont const getFont(BufferParams const &, size_type pos) const;
	///
	value_type getChar(size_type pos) const;
	///
	value_type getUChar(BufferParams const &, size_type pos) const;
	/// The position must already exist.
	void setChar(size_type pos, value_type c);
	///
	void setFont(size_type pos, LyXFont const & font);
	///
        string const getWord(size_type &) const;
	/// Returns the height of the highest font in range
	LyXFont::FONT_SIZE highestFontInRange(size_type startpos,
					      size_type endpos) const;
	///
	void insertChar(size_type pos, value_type c);
	///
	void insertChar(size_type pos, value_type c, LyXFont const &);
	///
	void insertInset(size_type pos, Inset * inset);
	///
	void insertInset(size_type pos, Inset * inset, LyXFont const &);
	///
	bool insertInsetAllowed(Inset * inset);
	///
	Inset * getInset(size_type pos);
	///
	Inset const * getInset(size_type pos) const;
	/** important for cut and paste
	    Temporary change from BufferParams to Buffer. Will revert when we
	    get rid of the argument to Inset::clone(Buffer const &) */
	void copyIntoMinibuffer(Buffer const &, size_type pos) const;
	///
	void cutIntoMinibuffer(BufferParams const &, size_type pos);
	///
	bool insertFromMinibuffer(size_type pos);

	///
	bool isHfill(size_type pos) const;
	///
	bool isInset(size_type pos) const;
	///
	bool isNewline(size_type pos) const;
	///
	bool isSeparator(size_type pos) const;
	///
	bool isLineSeparator(size_type pos) const;
	///
	bool isKomma(size_type pos) const;
	/// Used by the spellchecker
	bool isLetter(size_type pos) const;
	/// 
	bool isWord(size_type pos) const;

	/** This one resets all layout and dtp switches but not the font
	    of the single characters
	*/ 
	void clear();

	/** paste this paragraph with the next one
	    be carefull, this doesent make any check at all
	*/ 
	void pasteParagraph(BufferParams const &);

	/// used to remove the error messages
	int autoDeleteInsets();

	/// returns -1 if inset not found
	int getPositionOfInset(Inset * inset) const;

	///
	int stripLeadingSpaces(LyXTextClassList::size_type tclass); 

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
	bool linuxDocConvertChar(char c, string & sgml_string);

	ParagraphParameters & params();
	ParagraphParameters const & params() const;
private:
	
	///
	struct InsetTable {
		///
		size_type pos;
		///
		Inset * inset;
		///
		InsetTable(size_type p, Inset * i) : pos(p), inset(i) {}
	};

	///
	typedef std::vector<InsetTable> InsetList;
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
		Inset * operator*() { return (*it).inset; }
		///
		size_type getPos() const {return (*it).pos; }
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
	///
	inset_iterator InsetIterator(size_type pos);

private:
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
