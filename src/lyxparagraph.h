// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1999 The LyX Team.
 *
 *======================================================*/

#ifndef LYXPARAGRAPH_H
#define LYXPARAGRAPH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "definitions.h"
#include "insets/lyxinset.h"
#include "table.h"
#include "support/textutils.h"
#include "vspace.h"
#include "insets/insetbib.h"

class BufferParams;
class LyXBuffer;
class LyXLayout;
class TexRow;
struct LaTeXFeatures;

/// A LyXParagraph holds all text, attributes and insets in a text paragraph
class LyXParagraph  {
public:

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
	LyXParagraph(LyXParagraph *par);
	/// the destruktors removes the new paragraph from the list
	~LyXParagraph();

	///
	void writeFile(FILE*, BufferParams &, char, char);

	// ///
	//void writeLaTeX(FILE*, BufferParams &);

	///
	void validate(LaTeXFeatures&);
	
	///
	int GetID(){
		return id;
	}
	///
	void SetID(int id_arg){
		id = id_arg;
	}
	
	/** allocates more memory for the specified paragraph
	  pos is needed to specify the paragraph correctly. Remember the
	  closed footnotes
	  */
	void Enlarge(int pos, int number);

	/** make the allocated memory fit to the needed size
	  used to make a paragraph smaller
	  */
	void FitSize();
	
	///
	void read();
	
	///
	void readSimpleWholeFile(FILE *);

	///
	LyXParagraph* TeXOnePar(string &file, TexRow &texrow,
				string &foot, TexRow &foot_texrow,
				int &foot_count);

	///
	LyXParagraph* TeXEnvironment(string &file, TexRow &texrow,
				     string &foot, TexRow &foot_texrow,
				     int &foot_count);
	
	///
	LyXParagraph* Clone();
	
	///
	bool HasSameLayout(LyXParagraph *par);
	
	///
	void MakeSameLayout(LyXParagraph *par);

	/// Is it the first par with same depth and layout?
	bool IsFirstInSequence() {
		LyXParagraph *dhook = DepthHook(GetDepth());
		return (dhook == this
			|| dhook->GetLayout() != GetLayout()
			|| dhook->GetDepth() != GetDepth());
	}

	///
	int size;
	
	///
	char *text;

	/// 
	VSpace added_space_top;
	
	/// 
	VSpace added_space_bottom;
	
	///
	signed char layout;
	
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
	char align;
	
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
	int last;
	///
	LyXParagraph *next;
	///
	LyXParagraph *previous;

	/* table stuff -- begin*/
	///
	LyXTable *table;
	/* table stuff -- end*/

        /// 
        InsetBibKey* bibkey;  // ale970302

	/** these function are able to hide closed footnotes
	 */
	LyXParagraph *Next();
	
	///
	LyXParagraph *Previous();

	/** these function are able to hide open and closed footnotes
	 */ 
	LyXParagraph *NextAfterFootnote();
	///
	LyXParagraph *PreviousBeforeFootnote();
	///
	LyXParagraph *LastPhysicalPar();
	///
	LyXParagraph *FirstPhysicalPar();
	
	/// returns the physical paragraph
	LyXParagraph *ParFromPos(int pos);
	/// returns the position in the physical par
	int PositionInParFromPos(int pos);

	/// for the environments
	LyXParagraph* DepthHook(int depth);
	///
	int BeginningOfMainBody();
	///
	string GetLabestring();
	
	/// the next two functions are for the manual labels
	string GetLabelWidthString();
	///
	void SetLabelWidthString(const string &s);
	///
	int GetLayout();
	///
	char GetAlign();
	///
	char GetDepth();
	///
	void SetLayout(char new_layout);
	///
	void SetOnlyLayout(char new_layout);
	///
	int GetFirstCounter(int i);
	///
	int Last();
	
	/** This one resets all layout and dtp switches but not the font
	 of the single characters
	 */ 
	void Clear();
	///
	void Erase(int pos);

	/** the flag determines wether the layout should be copied
	 */ 
	void BreakParagraph(int pos, int flag);
	///
	void BreakParagraphConservative(int pos);

	/** paste this paragraph with the next one
	  be carefull, this doesent make any check at all
	  */ 
	void PasteParagraph();

	/** Get unistantiated font setting. Returns the difference
	  between the characters font and the layoutfont.
	  This is what is stored in the fonttable
	 */ 
	LyXFont GetFontSettings(int pos);
	
	/** Get fully instantiated font. If pos == -1, use the layout
	  font attached to this paragraph.
	  If pos == -2, use the label font of the layout attached here.
	  In all cases, the font is instantiated, i.e. does not have any
	  attributes with values LyXFont::INHERIT, LyXFont::IGNORE or 
	  LyXFont::TOGGLE.
	  */
	LyXFont getFont(int pos);
	
	///
	char GetChar(int pos);
	///
        string GetWord(int &);
	///
	void SetFont(int pos, LyXFont const & font);
	/// Returns the height of the highest font in range
	LyXFont::FONT_SIZE HighestFontInRange(int startpos, int endpos) const;
	///
	void InsertChar(int pos, char c);
	///
	void InsertInset(int pos, Inset *inset);
	///
	Inset* GetInset(int pos);
	
	/// used to remove the error messages
	int AutoDeleteInsets();

	///
	Inset* ReturnNextInsetPointer(int &pos);
	
	/// returns -1 if inset not found
	int GetPositionOfInset(Inset* inset);
	
	/// ok and now some footnote functions
	void OpenFootnotes();
	///
	void OpenFootnote(int pos);
	///
	void CloseFootnotes();
	///
	void CloseFootnote(int pos);
   
	/// important for cut and paste
	void CopyIntoMinibuffer(int pos);
	///
	void CutIntoMinibuffer(int pos);
	///
	void InsertFromMinibuffer(int pos);
	
	///
	LyXParagraph *FirstSelfrowPar();
	
	///
	bool IsHfill(int pos) {
		return IsHfillChar(GetChar(pos));
	}
	
	///
	bool IsInset(int pos) {
		return IsInsetChar(GetChar(pos));
	}
	
	///
	bool IsFloat(int pos) {
		return IsFloatChar(GetChar(pos));
	}
	
	///
	bool IsNewline(int pos) {
		bool tmp=false;
		if (pos>=0)
			tmp= IsNewlineChar(GetChar(pos));
		return tmp;
	}
	
	///
	bool IsSeparator(int pos) {
		return IsSeparatorChar(GetChar(pos));
	}
	
	///
	bool IsLineSeparator(int pos) {
		return IsLineSeparatorChar(GetChar(pos));
	}
	
	///
	bool IsKomma(int pos){
		return IsKommaChar(GetChar(pos));
	}
	
	/// Used by the spellchecker
	bool IsLetter(int pos);
	
	/// 
	bool IsWord( int pos ) {
	  return IsWordChar( GetChar(pos) ) ;
	}

	///
	int ClearParagraph(){
		int i=0;
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
        void SetPExtraType(int type, const char *width, const char *widthp);
	///
        void UnsetPExtraType();
	///
	bool RoffContTableRows(FILE *file, int i, int actcell);
	///
	bool linuxDocConvertChar(char c, string &sgml_string);
	///
	void SimpleDocBookOneTablePar(string &file, string &extra,
				      int &desc_on, int depth);
	///
	void DocBookContTableRows(string &file, string &extra, int &desc_on,
				  int i, int current_cell_number, int &column);
private:
	/** A font entry covers a range of positions. Notice that the
	  entries in the list are inserted in random order.
	  I don't think it's worth the effort to implement a more effective
	  datastructure, because the number of different fonts in a paragraph
	  is limited. (Asger)
	*/
	struct FontTable  {
		/// Start position of paragraph this font attribute covers
		int pos;
		/// Ending position of paragraph this font attribute covers
		int pos_end;

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
		/// Pointer to next font entry
		FontTable *next;
	};
	///
	struct InsetTable {
		///
		int pos;
		///
		Inset *inset;
		///
		InsetTable *next;
	};
	///
	FontTable *fonttable;
	///
	InsetTable *insettable;
	///
	LyXParagraph * TeXDeeper(string &file, TexRow &texrow,
				   string &foot, TexRow &foot_texrow,
				   int &foot_count);
	///
	LyXParagraph * TeXFootnote(string &file, TexRow &texrow,
				   string &foot, TexRow &foot_texrow,
				   int &foot_count);
	///
	bool SimpleTeXOnePar(string &file, TexRow &texrow);
	///
	bool SimpleTeXOneTablePar(string &file, TexRow &texrow);
	///
	bool TeXContTableRows(string &file, int i, int current_cell_number,
                              int &column, TexRow &texrow);
	///
	void SimpleTeXBlanks(string &file, TexRow &texrow,
			     int const i, int &column, LyXFont const &font,
			     LyXLayout const * const style);
	///
	void SimpleTeXSpecialChars(string &file, TexRow &texrow,
				   LyXFont &font, LyXFont &running_font,
				   LyXFont &basefont, bool &open_font,
				   LyXLayout const * const style,
				   int &i, int &column, char const c);
	///
	int id;
	///
	static unsigned int paragraph_id;
};

#endif
