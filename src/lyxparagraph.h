// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXPARAGRAPH_H
#define LYXPARAGRAPH_H

#ifdef __GNUG__
#pragma interface
#endif

//#define NEW_TABLE 1

#ifdef NEW_TABLE
#include <list>
#endif

#include <vector>

#include "insets/lyxinset.h"
#include "insets/insetbib.h"
#include "table.h"
#include "vspace.h"
#include "layout.h"

class BufferParams;
class LyXBuffer;
class TexRow;
struct LaTeXFeatures;

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
	LyXParagraph();
	/// this konstruktor inserts the new paragraph in a list
	LyXParagraph(LyXParagraph * par);
	/// the destruktors removes the new paragraph from the list
	~LyXParagraph();

	///
	void writeFile(FILE *, BufferParams &, char, char);

	///
	void validate(LaTeXFeatures &);
	
	///
	int GetID(){
		return id;
	}
	///
	void SetID(int id_arg){
		id = id_arg;
	}

	///
	void read();
	
	///
	void readSimpleWholeFile(FILE *);

	///
	LyXParagraph * TeXOnePar(string & file, TexRow & texrow,
				string & foot, TexRow & foot_texrow,
				int & foot_count);

	///
	LyXParagraph * TeXEnvironment(string & file, TexRow & texrow,
				     string & foot, TexRow & foot_texrow,
				     int & foot_count);
	
	///
	LyXParagraph * Clone();
	
	///
	bool HasSameLayout(LyXParagraph * par);
	
	///
	void MakeSameLayout(LyXParagraph * par);

	/// Is it the first par with same depth and layout?
	bool IsFirstInSequence() {
		LyXParagraph * dhook = DepthHook(GetDepth());
		return (dhook == this
			|| dhook->GetLayout() != GetLayout()
			|| dhook->GetDepth() != GetDepth());
	}


	///
	typedef char value_type;
	///
	typedef vector<value_type> TextContainer;
	//typedef deque<char> TextContainer;
	//typedef rope<char> TextContainer;
	typedef int size_type;
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
	///
	int counter[10];
public:
	///
	void setCounter(int i, int v) { counter[i] = v; }
	int getCounter(int i) { return counter[i]; }
	void incCounter(int i) { counter[i]++; }
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

	/** these function are able to hide open and closed footnotes
	 */ 
	LyXParagraph * NextAfterFootnote();
	///
	LyXParagraph * NextAfterFootnote() const;
	///
	LyXParagraph * PreviousBeforeFootnote();
	///
	LyXParagraph * LastPhysicalPar();
	///
	LyXParagraph * FirstPhysicalPar();

	/// returns the physical paragraph
	LyXParagraph * ParFromPos(size_type pos);
	/// returns the position in the physical par
	int PositionInParFromPos(size_type pos);

	/// for the environments
	LyXParagraph * DepthHook(int depth);
	///
	int BeginningOfMainBody();
	///
	string GetLabestring();
	
	/// the next two functions are for the manual labels
	string GetLabelWidthString();
	///
	void SetLabelWidthString(string const & s);
	///
	LyXTextClass::LayoutList::size_type GetLayout();
	///
	char GetAlign();
	///
	char GetDepth();
	///
	void SetLayout(LyXTextClass::LayoutList::size_type new_layout);
	///
	void SetOnlyLayout(LyXTextClass::LayoutList::size_type new_layout);
	///
	int GetFirstCounter(int i);
	///
	size_type Last();
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
	LyXFont GetFontSettings(size_type pos);
	/** Get fully instantiated font. If pos == -1, use the layout
	  font attached to this paragraph.
	  If pos == -2, use the label font of the layout attached here.
	  In all cases, the font is instantiated, i.e. does not have any
	  attributes with values LyXFont::INHERIT, LyXFont::IGNORE or 
	  LyXFont::TOGGLE.
	  */
	LyXFont getFont(size_type pos);
	///
	char GetChar(size_type pos);
	///
	char GetChar(size_type pos) const;
	///
	void SetFont(size_type pos, LyXFont const & font);
	///
        string GetWord(size_type &);
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
	Inset * ReturnNextInsetPointer(size_type & pos);
	///
	void OpenFootnote(size_type pos);
	///
	void CloseFootnote(size_type pos);
	/// important for cut and paste
	void CopyIntoMinibuffer(size_type pos);
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
	int GetPositionOfInset(Inset * inset);
	
	/// ok and now some footnote functions
	void OpenFootnotes();

	///
	void CloseFootnotes();
   
	///
	LyXParagraph * FirstSelfrowPar();

	///
	int ClearParagraph(){
		int i = 0;
		if (!IsDummy() && !table){
			while (Last()
			       && (IsNewline(0) 
				   || IsLineSeparator(0))){
				Erase(0);
				i++;
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
	bool IsDummy(){
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
	bool RoffContTableRows(FILE * file, size_type i,
			       int actcell);
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
#ifndef NEW_TABLE
		/// Pointer to next font entry
		FontTable * next;
#endif
	};
	///
	struct InsetTable {
		///
		size_type pos;
		///
		Inset * inset;
#ifndef NEW_TABLE
		///
		InsetTable * next;
#endif
	};
#ifdef NEW_TABLE
	///
	typedef list<FontTable> FontList;
	///
	typedef list<InsetTable> InsetList;
	///
	FontList fontlist;
	///
	InsetList insetlist;
#else
	///
	FontTable * fonttable;
	///
	InsetTable * insettable;
#endif
	///
	LyXParagraph * TeXDeeper(string & file, TexRow & texrow,
				   string & foot, TexRow & foot_texrow,
				   int & foot_count);
	///
	LyXParagraph * TeXFootnote(string & file, TexRow & texrow,
				   string & foot, TexRow & foot_texrow,
				   int & foot_count);
	///
	bool SimpleTeXOnePar(string & file, TexRow & texrow);
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
	int id;
	///
	static unsigned int paragraph_id;
};
#endif
