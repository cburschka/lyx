// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXPARAGRAPH_H
#define LYXPARAGRAPH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include <vector>
#include <list>
#include <boost/array.hpp>

#include "insets/lyxinset.h"
#include "ParagraphParameters.h"
#include "support/LAssert.h"

class BufferParams;
class LyXBuffer;
class TexRow;
struct LaTeXFeatures;
class InsetBibKey;
class BufferView;
class Language;

// I dare you to enable this and help me find all the bugs that then show
// up. (Lgb)
#define NEW_INSETS 1
#define NO_PEXTRA 1

/// A LyXParagraph holds all text, attributes and insets in a text paragraph
class LyXParagraph  {
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
#ifndef NEW_INSETS
		///
		META_FOOTNOTE = 1,
		///
		META_MARGIN,
		///
		META_FIG,
		///
		META_TAB,
		///
		META_ALGORITHM,
		///
		META_WIDE_FIG,
		///
		META_WIDE_TAB,
		///
		META_HFILL,
#else
		///
		META_HFILL = 1,
#endif
		///
		META_NEWLINE,
		//
		//META_PROTECTED_SEPARATOR,
		///
		META_INSET
	};
#ifndef NEW_INSETS

	/// The footnoteflag
	enum footnote_flag {
		///
		NO_FOOTNOTE,
		///
		OPEN_FOOTNOTE,
		///
		CLOSED_FOOTNOTE
	};

	/// The footnotekinds
	enum footnote_kind {
		///
		FOOTNOTE,
		///
		MARGIN,
		///
		FIG,
		///
		TAB,
		///
		ALGORITHM,  // Bernhard, 970807
		///
		WIDE_FIG,   // CFO-G, 971106
		///
		WIDE_TAB    // CFO-G, 971106
	};
#endif
	///
	typedef char value_type;
	///
	typedef std::vector<value_type> TextContainer;
	///
	/* This should be TextContainer::size_type, but we need
	   signed values for now.
	*/
	typedef TextContainer::difference_type size_type;

	///
	LyXParagraph();
	/// this constructor inserts the new paragraph in a list
	explicit
	LyXParagraph(LyXParagraph * par);
	/// the destructor removes the new paragraph from the list
	~LyXParagraph();

	///
	Language const * getParLanguage(BufferParams const &) const;
	///
	bool isRightToLeftPar(BufferParams const &) const;
	///
	void ChangeLanguage(BufferParams const & bparams,
			    Language const * from, Language const * to);
	///
	bool isMultiLingual(BufferParams const &);
	///

	string const String(Buffer const *, bool label);
	///
	string const String(Buffer const *, size_type beg, size_type end);
	
	///
	void writeFile(Buffer const *, std::ostream &, BufferParams const &,
		       char, char) const;
	///
	void validate(LaTeXFeatures &) const;
	
	///
	int id() const;
	///
	void id(int id_arg);
	///
	void read();

	///
	LyXParagraph * TeXOnePar(Buffer const *, BufferParams const &,
				 std::ostream &, TexRow & texrow,
				 bool moving_arg
#ifndef NEW_INSETS
				 ,
				 std::ostream & foot, TexRow & foot_texrow,
				 int & foot_count
#endif
		);
	///
	bool SimpleTeXOnePar(Buffer const *, BufferParams const &,
			     std::ostream &, TexRow & texrow, bool moving_arg);

	///
	LyXParagraph * TeXEnvironment(Buffer const *, BufferParams const &,
				      std::ostream &, TexRow & texrow
#ifndef NEW_INSETS
				      ,std::ostream & foot, TexRow & foot_texrow,
				      int & foot_count
#endif
		);
	///
	LyXParagraph * Clone() const;
	
	///
	bool HasSameLayout(LyXParagraph const * par) const;
	
	///
	void MakeSameLayout(LyXParagraph const * par);

	/// Is it the first par with same depth and layout?
	bool IsFirstInSequence() const;

	/** Check if the current paragraph is the last paragraph in a
	    proof environment */
	int GetEndLabel(BufferParams const &) const;
	///
	Inset * InInset();
	///
	void SetInsetOwner(Inset * i);
	///
	void deleteInsetsLyXText(BufferView *);
	///
	void resizeInsetsLyXText(BufferView *);
private:
	///
	TextContainer text;
	///
	Inset * inset_owner;

public:
	///
	inline
	size_type size() const;
	///
	void fitToSize();
	///
	void setContentsFromPar(LyXParagraph * par);
	///
	void clearContents();

	ParagraphParameters params;
	
	///
	LyXTextClass::LayoutList::size_type layout;
#ifndef NEW_INSETS
	/**
	  \begin{itemize}
	  \item no footnote, closed footnote, 
	  \item open footnote, where footnote
	  \item means footnote-environment
	  \end{itemize}
	 */
	footnote_flag footnoteflag;

	/// footnote, margin, fig, tab
	footnote_kind footnotekind;
#endif
	
private:
	///
	boost::array<int, 10> counter_;
public:
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
#ifdef NEW_INSETS
private:
#endif
	///
	LyXParagraph * next_;
	///
	LyXParagraph * previous_;
public:
        /// 
        InsetBibKey * bibkey;  // ale970302

	///
	void next(LyXParagraph *);
	/** these function are able to hide closed footnotes
	 */
	LyXParagraph * next();
	///
	LyXParagraph const * next() const;

	///
	void previous(LyXParagraph *);
	///
	LyXParagraph * previous();
	///
	LyXParagraph const * previous() const;

#ifndef NEW_INSETS
	/** these function are able to hide open and closed footnotes
	 */ 
	LyXParagraph * NextAfterFootnote();
	///
	LyXParagraph const * NextAfterFootnote() const;
	
	///
	LyXParagraph * PreviousBeforeFootnote();
	///
	LyXParagraph * LastPhysicalPar();
	///
	LyXParagraph const * LastPhysicalPar() const;
	
	///
	LyXParagraph * FirstPhysicalPar();
	///
	LyXParagraph const * FirstPhysicalPar() const;
	/// returns the physical paragraph
	LyXParagraph * ParFromPos(size_type pos);
	/// returns the position in the physical par
	int PositionInParFromPos(size_type pos) const;
#endif

	/// for the environments
	LyXParagraph * DepthHook(int depth);
	/// for the environments
	LyXParagraph const * DepthHook(int depth) const;
	///
	int BeginningOfMainBody() const;
	///
	string const & GetLabelstring() const;
	
	/// the next two functions are for the manual labels
	string const GetLabelWidthString() const;
	///
	void SetLabelWidthString(string const & s);
	///
	inline
	LyXTextClass::LayoutList::size_type GetLayout() const;
	///
	char GetAlign() const;
	///
	char GetDepth() const;
#ifndef NEW_INSETS
	///
	void SetLayout(BufferParams const &,
		       LyXTextClass::LayoutList::size_type new_layout);
	///
	void SetOnlyLayout(BufferParams const &,
			   LyXTextClass::LayoutList::size_type new_layout);
	///
	size_type Last() const;
#else
	///
	void SetLayout(LyXTextClass::LayoutList::size_type new_layout);
	///
	void SetOnlyLayout(LyXTextClass::LayoutList::size_type new_layout);
#endif
	///
	int GetFirstCounter(int i) const;
	///
	void Erase(size_type pos);
	/** the flag determines wether the layout should be copied
	 */ 
	void BreakParagraph(BufferParams const &, size_type pos, int flag);
	///
	void BreakParagraphConservative(BufferParams const &, size_type pos);
	/** Get unistantiated font setting. Returns the difference
	  between the characters font and the layoutfont.
	  This is what is stored in the fonttable
	 */
	LyXFont const
	GetFontSettings(BufferParams const &, size_type pos) const;
	///
	LyXFont const GetFirstFontSettings() const;

	/** Get fully instantiated font. If pos == -1, use the layout
	    font attached to this paragraph.
	    If pos == -2, use the label font of the layout attached here.
	    In all cases, the font is instantiated, i.e. does not have any
	    attributes with values LyXFont::INHERIT, LyXFont::IGNORE or 
	    LyXFont::TOGGLE.
	*/
	LyXFont const getFont(BufferParams const &, size_type pos) const;
	///
	value_type GetChar(size_type pos) const;
	///
	value_type GetUChar(BufferParams const &, size_type pos) const;
	/// The position must already exist.
	void SetChar(size_type pos, value_type c);
	///
	void SetFont(size_type pos, LyXFont const & font);
	///
        string const GetWord(size_type &) const;
	/// Returns the height of the highest font in range
	LyXFont::FONT_SIZE HighestFontInRange(size_type startpos,
					      size_type endpos) const;
	///
	void InsertChar(size_type pos, value_type c);
	///
	void InsertChar(size_type pos, value_type c, LyXFont const &);
	///
	void InsertInset(size_type pos, Inset * inset);
	///
	void InsertInset(size_type pos, Inset * inset, LyXFont const &);
	///
	bool InsertInsetAllowed(Inset * inset);
	///
	Inset * GetInset(size_type pos);
	///
	Inset const * GetInset(size_type pos) const;
#ifndef NEW_INSETS
	///
	void OpenFootnote(size_type pos);
	///
	void CloseFootnote(size_type pos);
#endif
	/** important for cut and paste
	    Temporary change from BufferParams to Buffer. Will revert when we
	    get rid of the argument to Inset::Clone(Buffer const &) */
	void CopyIntoMinibuffer(Buffer const &, size_type pos) const;
	///
	void CutIntoMinibuffer(BufferParams const &, size_type pos);
	///
	bool InsertFromMinibuffer(size_type pos);

	///
	bool IsHfill(size_type pos) const;
	///
	bool IsInset(size_type pos) const;
#ifndef NEW_INSETS
	///
	bool IsFloat(size_type pos) const;
#endif
	///
	bool IsNewline(size_type pos) const;
	///
	bool IsSeparator(size_type pos) const;
	///
	bool IsLineSeparator(size_type pos) const;
	///
	bool IsKomma(size_type pos) const;
	/// Used by the spellchecker
	bool IsLetter(size_type pos) const;
	/// 
	bool IsWord(size_type pos) const;

	/** This one resets all layout and dtp switches but not the font
	 of the single characters
	 */ 
	void Clear();

	/** paste this paragraph with the next one
	  be carefull, this doesent make any check at all
	  */ 
	void PasteParagraph(BufferParams const &);

	/// used to remove the error messages
	int AutoDeleteInsets();

	/// returns -1 if inset not found
	int GetPositionOfInset(Inset * inset) const;

#ifndef NEW_INSETS
	/// ok and now some footnote functions
	void OpenFootnotes();

	///
	void CloseFootnotes();
	///
	LyXParagraph * FirstSelfrowPar();
#endif

	///
	int StripLeadingSpaces(LyXTextClassList::size_type tclass); 

#ifndef NEW_INSETS
	/** A paragraph following a footnote is a "dummy". A paragraph
	    with a footnote in it is stored as three paragraphs:
	    First a paragraph with the text up to the footnote, then
	    one (or more) paragraphs with the footnote, and finally
	    the a paragraph with the text after the footnote. Only the
	    first paragraph keeps information  about layoutparameters, */
	bool IsDummy() const;
#endif
#ifndef NO_PEXTRA_REALLY
        /* If I set a PExtra Indent on one paragraph of a ENV_LIST-TYPE
           I have to set it on each of it's elements */
	///
        void SetPExtraType(BufferParams const &, int type,
			   string const & width, string const & widthp);
	///
        void UnsetPExtraType(BufferParams const &);
#endif
	///
	bool linuxDocConvertChar(char c, string & sgml_string);
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
	friend struct matchIT;
	///
	struct matchIT {
		/// used by lower_bound and upper_bound
		inline
		int operator()(LyXParagraph::InsetTable const & a,
			       LyXParagraph::InsetTable const & b) const {
			return a.pos < b.pos;
		}
	};
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
	struct FontTable  {
		///
		FontTable(size_type p, LyXFont const & f)
			: pos_(p)
		{
			font_ = container.get(f);
		}
		///
		size_type pos() const { return pos_; }
		///
		void pos(size_type p) { pos_ = p; }
		///
		LyXFont const & font() const { return *font_; }
		///
		void font(LyXFont const & f) { font_ = container.get(f);}
	private:
		/// End position of paragraph this font attribute covers
		size_type pos_;
		/** Font. Interpretation of the font values:
		If a value is LyXFont::INHERIT_*, it means that the font 
		attribute is inherited from either the layout of this
		paragraph or, in the case of nested paragraphs, from the 
		layout in the environment one level up until completely 
		resolved.
		The values LyXFont::IGNORE_* and LyXFont::TOGGLE are NOT 
		allowed in these font tables.
		*/
		boost::shared_ptr<LyXFont> font_;
		///
		static ShareContainer<LyXFont> container;
	};
	///
	friend struct matchFT;
	///
	struct matchFT {
		/// used by lower_bound and upper_bound
		inline
		int operator()(LyXParagraph::FontTable const & a,
			       LyXParagraph::FontTable const & b) const {
			return a.pos() < b.pos();
		}
	};

	///
	typedef std::vector<FontTable> FontList;
	///
	FontList fontlist;
	///
	typedef std::vector<InsetTable> InsetList;
	///
	InsetList insetlist;
	///
	LyXParagraph * TeXDeeper(Buffer const *, BufferParams const &,
				 std::ostream &, TexRow & texrow
#ifndef NEW_INSETS
				 ,std::ostream & foot, TexRow & foot_texrow,
				 int & foot_count
#endif
		);
#ifndef NEW_INSETS
	///
	LyXParagraph * TeXFootnote(Buffer const *, BufferParams const &,
				   std::ostream &, TexRow & texrow,
				   std::ostream & foot, TexRow & foot_texrow,
				   int & foot_count,
				   bool parent_is_rtl);
#endif
	///
	void SimpleTeXBlanks(std::ostream &, TexRow & texrow,
			     size_type const i,
			     int & column, LyXFont const & font,
			     LyXLayout const & style);
	///
	void SimpleTeXSpecialChars(Buffer const *, BufferParams const &,
				   std::ostream &, TexRow & texrow,
				   bool moving_arg,
				   LyXFont & font, LyXFont & running_font,
				   LyXFont & basefont, bool & open_font,
				   LyXLayout const & style,
				   size_type & i,
				   int & column, value_type const c);
	///
	unsigned int id_;
	///
	static unsigned int paragraph_id;
public:
	///
	class inset_iterator {
	public:
		///
		inset_iterator() {}
		//
		inset_iterator(InsetList::iterator const & iter) : it(iter) {};
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
	inset_iterator inset_iterator_begin();
	///
	inset_iterator inset_iterator_end();
	///
	inset_iterator InsetIterator(size_type pos);
};


#ifdef NEW_INSETS
inline
LyXParagraph::value_type
LyXParagraph::GetChar(LyXParagraph::size_type pos) const
{
	Assert(pos <= size());
	// This is stronger, and I belive that this is the assertion
	// that we should really use. (Lgb)
	//Assert(pos < size());

	// Then this has no meaning. (Lgb)
	if (!size() || pos == size()) return '\0';
	
	return text[pos];
}
#endif

inline
int LyXParagraph::id() const
{
	return id_;
}


inline
void  LyXParagraph::id(int id_arg)
{
	id_ = id_arg;
}


inline
bool LyXParagraph::IsFirstInSequence() const
{
	LyXParagraph const * dhook = DepthHook(GetDepth());
	return (dhook == this
		|| dhook->GetLayout() != GetLayout()
		|| dhook->GetDepth() != GetDepth());
}


inline
Inset * LyXParagraph::InInset()
{
	return inset_owner;
}


inline
LyXParagraph::size_type LyXParagraph::size() const
{
	return text.size();
}


inline
void LyXParagraph::clearContents()
{
	text.clear();
}


inline
void LyXParagraph::setCounter(int i, int v)
{
	counter_[i] = v;
}


inline
int LyXParagraph::getCounter(int i) const
{
	return counter_[i];
}


inline
void LyXParagraph::incCounter(int i)
{
	counter_[i]++;
}


inline
void LyXParagraph::SetChar(size_type pos, value_type c)
{
	text[pos] = c;
}


inline
LyXTextClass::size_type LyXParagraph::GetLayout() const
{
#ifndef NEW_INSETS
	return FirstPhysicalPar()->layout;
#else
	return layout;
#endif
}


inline
LyXParagraph::inset_iterator LyXParagraph::inset_iterator_begin()
{
	return inset_iterator(insetlist.begin());
}


inline
LyXParagraph::inset_iterator LyXParagraph::inset_iterator_end()
{
	return inset_iterator(insetlist.end());
}
#endif
