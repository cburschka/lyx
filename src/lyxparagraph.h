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

#include <vector>
#include <list>

#include "insets/lyxinset.h"
#include "table.h"
#include "vspace.h"
#include "layout.h"
#include "support/block.h"
#include "direction.h"

class BufferParams;
class LyXBuffer;
class TexRow;
struct LaTeXFeatures;
class InsetBibKey;

using std::list;

/// A LyXParagraph holds all text, attributes and insets in a text paragraph
class LyXParagraph  {
public:
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
	///
	enum META_KIND {
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
		///
		META_NEWLINE,
		///
		META_PROTECTED_SEPARATOR,
		///
		META_INSET
	};

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
	
	///
	typedef char value_type;
	///
	typedef vector<value_type> TextContainer;
	///
	typedef int size_type;

	///
	LyXParagraph();
	/// this konstruktor inserts the new paragraph in a list
	LyXParagraph(LyXParagraph * par);
	/// the destruktors removes the new paragraph from the list
	~LyXParagraph();

	///
	LyXDirection getParDirection() const;
	///
	LyXDirection getLetterDirection(size_type pos) const;
	
	///
	void writeFile(ostream &, BufferParams const &, char, char) const;
	///
	void validate(LaTeXFeatures &) const;
	
	///
	int id() const {
		return id_;
	}
	///
	void id(int id_arg) {
		id_ = id_arg;
	}

	///
	void read();

	///
	void readSimpleWholeFile(istream &);

	///
	LyXParagraph * TeXOnePar(string & file, TexRow & texrow,
				string & foot, TexRow & foot_texrow,
				int & foot_count);
	///
	bool SimpleTeXOnePar(string & file, TexRow & texrow);

	///
	LyXParagraph * TeXEnvironment(string & file, TexRow & texrow,
				     string & foot, TexRow & foot_texrow,
				     int & foot_count);
	
	///
	LyXParagraph * Clone() const;
	
	///
	bool HasSameLayout(LyXParagraph const * par) const;
	
	///
	void MakeSameLayout(LyXParagraph const * par);

	/// Is it the first par with same depth and layout?
	bool IsFirstInSequence() const {
		LyXParagraph const * dhook = DepthHook(GetDepth());
		return (dhook == this
			|| dhook->GetLayout() != GetLayout()
			|| dhook->GetDepth() != GetDepth());
	}


	///
	TextContainer text;
	///
	size_type size() const { return text.size(); }
	/// 
	VSpace added_space_top;
	
	/// 
	VSpace added_space_bottom;
	
	///
	LyXTextClass::LayoutList::size_type layout;
	
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
   
	//@Man: the LyX- DTP-switches
	//@{
	///
	bool line_top;
	
	///
	bool line_bottom;
	
  	///
	bool pagebreak_top;
	
	///
	bool pagebreak_bottom;
	
	///
	LyXAlignment align;
	
	///
	char depth;
	
	///
        bool noindent;
	
private:
	block<int, 10> counter_;
public:
	///
	void setCounter(int i, int v) { counter_[i] = v; }
	int getCounter(int i) const { return counter_[i]; }
	void incCounter(int i) { counter_[i]++; }
	///
	bool start_of_appendix;

	///
	bool appendix;

	///
	char enumdepth;
	
	///
	char itemdepth;

        /* This is for the paragraph extra stuff */
        ///
        int pextra_type;
        ///
        string pextra_width;
        ///
        string pextra_widthp;
        ///
        int pextra_alignment;
        ///
        bool pextra_hfill;
        ///
        bool pextra_start_minipage;
        
        ///
	string labelstring;
	
	///
	string labelwidthstring;
	//@}
	
	///
	LyXParagraph * next;
	///
	LyXParagraph * previous;

	/* table stuff -- begin*/
	///
	LyXTable * table;
	/* table stuff -- end*/

        /// 
        InsetBibKey * bibkey;  // ale970302

	/** these function are able to hide closed footnotes
	 */
	LyXParagraph * Next();
	
	///
	LyXParagraph * Previous();
	///
	LyXParagraph const * Previous() const;

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
	LyXParagraph * FirstPhysicalPar();
	///
	LyXParagraph const * FirstPhysicalPar() const;

	/// returns the physical paragraph
	LyXParagraph * ParFromPos(size_type pos);
	/// returns the position in the physical par
	int PositionInParFromPos(size_type pos) const;

	/// for the environments
	LyXParagraph * DepthHook(int depth);
	/// for the environments
	LyXParagraph const * DepthHook(int depth) const;
	///
	int BeginningOfMainBody() const;
	///
	string GetLabestring() const;
	
	/// the next two functions are for the manual labels
	string GetLabelWidthString() const;
	///
	void SetLabelWidthString(string const & s);
	///
	LyXTextClass::LayoutList::size_type GetLayout() const;
	///
	char GetAlign() const;
	///
	char GetDepth() const;
	///
	void SetLayout(LyXTextClass::LayoutList::size_type new_layout);
	///
	void SetOnlyLayout(LyXTextClass::LayoutList::size_type new_layout);
	///
	int GetFirstCounter(int i) const;
	///
	size_type Last() const;
	///
	void Erase(size_type pos);
	/** the flag determines wether the layout should be copied
	 */ 
	void BreakParagraph(size_type pos, int flag);
	///
	void BreakParagraphConservative(size_type pos);
	/** Get unistantiated font setting. Returns the difference
	  between the characters font and the layoutfont.
	  This is what is stored in the fonttable
	 */
	LyXFont GetFontSettings(size_type pos) const;
	/** Get fully instantiated font. If pos == -1, use the layout
	  font attached to this paragraph.
	  If pos == -2, use the label font of the layout attached here.
	  In all cases, the font is instantiated, i.e. does not have any
	  attributes with values LyXFont::INHERIT, LyXFont::IGNORE or 
	  LyXFont::TOGGLE.
	  */
	LyXFont getFont(size_type pos) const;
	///
	char GetChar(size_type pos);
	///
	char GetChar(size_type pos) const;
	///
	void SetFont(size_type pos, LyXFont const & font);
	///
        string GetWord(size_type &) const;
	/// Returns the height of the highest font in range
	LyXFont::FONT_SIZE HighestFontInRange(size_type startpos,
					      size_type endpos) const;
	///
	void InsertChar(size_type pos, char c);
	///
	void InsertInset(size_type pos, Inset * inset);
	///
	Inset * GetInset(size_type pos);
	///
	Inset const * GetInset(size_type pos) const;
	///
	Inset * ReturnNextInsetPointer(size_type & pos);
	///
	void OpenFootnote(size_type pos);
	///
	void CloseFootnote(size_type pos);
	/// important for cut and paste
	void CopyIntoMinibuffer(size_type pos) const;
	///
	void CutIntoMinibuffer(size_type pos);
	///
	void InsertFromMinibuffer(size_type pos);

	///
	bool IsHfill(size_type pos) const;
	///
	bool IsInset(size_type pos) const;
	///
	bool IsFloat(size_type pos) const;
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
	void PasteParagraph();

	/// used to remove the error messages
	int AutoDeleteInsets();

	/// returns -1 if inset not found
	int GetPositionOfInset(Inset * inset) const;
	
	/// ok and now some footnote functions
	void OpenFootnotes();

	///
	void CloseFootnotes();
   
	///
	LyXParagraph * FirstSelfrowPar();

	///
	int ClearParagraph() {
		int i = 0;
		if (!IsDummy() && !table){
			while (Last()
			       && (IsNewline(0) 
				   || IsLineSeparator(0))){
				Erase(0);
				++i;
			}
		}
		return i;
	}
	
	/** A paragraph following a footnote is a "dummy". A paragraph
	  with a footnote in it is stored as three paragraphs:
	  First a paragraph with the text up to the footnote, then
	  one (or more) paragraphs with the footnote, and finally
	  the a paragraph with the text after the footnote. Only the
	  first paragraph keeps information  about layoutparameters, */
	bool IsDummy() const {
		return (footnoteflag == LyXParagraph::NO_FOOTNOTE && previous
			&& previous->footnoteflag != LyXParagraph::NO_FOOTNOTE);
	}

        /* If I set a PExtra Indent on one paragraph of a ENV_LIST-TYPE
           I have to set it on each of it's elements */
	///
        void SetPExtraType(int type, char const * width, char const * widthp);
	///
        void UnsetPExtraType();
	///
	bool RoffContTableRows(ostream &, size_type i, int actcell);
	///
	void DocBookContTableRows(string & file, string & extra, int & desc_on,
				  size_type i,
				  int current_cell_number, int & column);
	///
	bool linuxDocConvertChar(char c, string & sgml_string);
	///
	void SimpleDocBookOneTablePar(string & file, string & extra,
				      int & desc_on, int depth);
private:
	/** A font entry covers a range of positions. Notice that the
	  entries in the list are inserted in random order.
	  I don't think it's worth the effort to implement a more effective
	  datastructure, because the number of different fonts in a paragraph
	  is limited. (Asger)
	*/
	struct FontTable  {
		/// Start position of paragraph this font attribute covers
		size_type pos;
		/// Ending position of paragraph this font attribute covers
		size_type pos_end;
		/** Font. Interpretation of the font values:
		If a value is LyXFont::INHERIT_*, it means that the font 
		attribute is inherited from either the layout of this
		paragraph or, in the case of nested paragraphs, from the 
		layout in the environment one level up until completely 
		resolved.
		The values LyXFont::IGNORE_* and LyXFont::TOGGLE are NOT 
		allowed in these font tables.
		*/
		LyXFont font;
	};
	///
	struct InsetTable {
		///
		size_type pos;
		///
		Inset * inset;
	};
	///
	typedef list<FontTable> FontList;
	///
	FontList fontlist;
	///
	typedef list<InsetTable> InsetList;
	///
	InsetList insetlist;
	///
	LyXParagraph * TeXDeeper(string & file, TexRow & texrow,
				   string & foot, TexRow & foot_texrow,
				   int & foot_count);
	///
	LyXParagraph * TeXFootnote(string & file, TexRow & texrow,
				   string & foot, TexRow & foot_texrow,
				   int & foot_count,
				   LyXDirection par_direction);
	///
	bool SimpleTeXOneTablePar(string & file, TexRow & texrow);
	///
	bool TeXContTableRows(string & file, size_type i,
			      int current_cell_number,
                              int & column, TexRow & texrow);
	///
	void SimpleTeXBlanks(string & file, TexRow & texrow,
			     size_type const i,
			     int & column, LyXFont const & font,
			     LyXLayout const & style);
	///
	void SimpleTeXSpecialChars(string & file, TexRow & texrow,
				   LyXFont & font, LyXFont & running_font,
				   LyXFont & basefont, bool & open_font,
				   LyXLayout const & style,
				   size_type & i,
				   int & column, char const c);
	///
	unsigned int id_;
	///
	static unsigned int paragraph_id;
};
#endif
