// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000-2001 The LyX Team.
 *
 *           @author: Jürgen Vigna
 *
 * ====================================================== */
#ifndef TABULAR_H
#define TABULAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>
#include <vector>

#include "lyxlex.h"
#include "layout.h"
#include "LString.h"
#include "insets/insettext.h"
#include "lyxlength.h"

class InsetTabular;
class LaTeXFeatures;
class Buffer;

/* The features the text class offers for tables */ 

///
class LyXTabular  {
public:
	///
	enum Feature {
		///
		APPEND_ROW = 0,
		///
		APPEND_COLUMN,
		///
		DELETE_ROW,
		///
		DELETE_COLUMN,
		///
		TOGGLE_LINE_TOP,
		///
		TOGGLE_LINE_BOTTOM,
		///
		TOGGLE_LINE_LEFT,
		///
		TOGGLE_LINE_RIGHT,
		///
		ALIGN_LEFT,
		///
		ALIGN_RIGHT,
		///
		ALIGN_CENTER,
		///
		VALIGN_TOP,
		///
		VALIGN_BOTTOM,
		///
		VALIGN_CENTER,
		///
		M_TOGGLE_LINE_TOP,
		///
		M_TOGGLE_LINE_BOTTOM,
		///
		M_TOGGLE_LINE_LEFT,
		///
		M_TOGGLE_LINE_RIGHT,
		///
		M_ALIGN_LEFT,
		///
		M_ALIGN_RIGHT,
		///
		M_ALIGN_CENTER,
		///
		M_VALIGN_TOP,
		///
		M_VALIGN_BOTTOM,
		///
		M_VALIGN_CENTER,
		///
		MULTICOLUMN,
		///
		SET_ALL_LINES,
		///
		UNSET_ALL_LINES,
		///
		SET_LONGTABULAR,
		///
		UNSET_LONGTABULAR,
		///
		SET_PWIDTH,
		///
		SET_MPWIDTH,
		///
		SET_ROTATE_TABULAR,
		///
		UNSET_ROTATE_TABULAR,
		///
		SET_ROTATE_CELL,
		///
		UNSET_ROTATE_CELL,
		///
		SET_USEBOX,
		///
		SET_LTHEAD,
		UNSET_LTHEAD,
		///
		SET_LTFIRSTHEAD,
		UNSET_LTFIRSTHEAD,
		///
		SET_LTFOOT,
		UNSET_LTFOOT,
		///
		SET_LTLASTFOOT,
		UNSET_LTLASTFOOT,
		///
		SET_LTNEWPAGE,
		///
		SET_SPECIAL_COLUMN,
		///
		SET_SPECIAL_MULTI,
		///
		LAST_ACTION
	};
	///
	enum {
		///
		CELL_NORMAL = 0,
		///
		CELL_BEGIN_OF_MULTICOLUMN,
		///
		CELL_PART_OF_MULTICOLUMN
	};

	///
	enum VAlignment {
		///
		LYX_VALIGN_TOP = 0,
		///
		LYX_VALIGN_BOTTOM = 1,
		///
		LYX_VALIGN_CENTER = 2
	};

	enum BoxType {
		///
		BOX_NONE = 0,
		///
		BOX_PARBOX = 1,
		///
		BOX_MINIPAGE = 2
	};

	struct lttype {
		// constructor
		lttype();
		// we have this header type (is set in the GetLT... functions)
		bool set;
		// double borders on top
		bool topDL;
		// double borders on bottom
		bool bottomDL;
		// used for FirstHeader & LastFooter and if this is true
		// all the rows marked as FirstHeader or LastFooter are
		// ignored in the output and it is set to be empty!
		bool empty;
	};
	///
	typedef struct lttype ltType;
	
	/* konstruktor */
	///
	LyXTabular(InsetTabular *, int columns_arg, int rows_arg);
	///
	LyXTabular(InsetTabular *, LyXTabular const &, bool same_id = false);
	///
	explicit
	LyXTabular(Buffer const *, InsetTabular *, LyXLex & lex);
	///
	LyXTabular & operator=(LyXTabular const &);
	///
	LyXTabular * clone(InsetTabular *, bool same_id = false);
	
	/// Returns true if there is a topline, returns false if not
	bool TopLine(int cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool BottomLine(int cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool LeftLine(int cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool RightLine(int cell, bool onlycolumn = false) const;
	
	///
	bool TopAlreadyDrawed(int cell) const;
	///
	bool LeftAlreadyDrawed(int cell) const;
	///
	bool IsLastRow(int cell) const;

	///
	int GetAdditionalHeight(int row) const;
	///
	int GetAdditionalWidth(int cell) const;
	
	/* returns the maximum over all rows */
	///
	int GetWidthOfColumn(int cell) const;
	///
	int GetWidthOfTabular() const;
	///
	int GetAscentOfRow(int row) const;
	///
	int GetDescentOfRow(int row) const;
	///
	int GetHeightOfTabular() const;
	/// Returns true if a complete update is necessary, otherwise false
	bool SetAscentOfRow(int row, int height);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetDescentOfRow(int row, int height);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetWidthOfCell(int cell, int new_width);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetAllLines(int cell, bool line);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetTopLine(int cell, bool line, bool onlycolumn = false);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetBottomLine(int cell, bool line, bool onlycolumn = false);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetLeftLine(int cell, bool line, bool onlycolumn = false);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetRightLine(int cell, bool line, bool onlycolumn = false);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetAlignment(int cell, LyXAlignment align,
			  bool onlycolumn = false);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetVAlignment(int cell, VAlignment align,
			   bool onlycolumn = false);
	///
	bool SetColumnPWidth(int cell, LyXLength const & width);
	///
	bool SetMColumnPWidth(int cell, LyXLength const & width);
	///
	bool SetAlignSpecial(int cell, string const & special, Feature what);
	///
	LyXAlignment GetAlignment(int cell, bool onlycolumn = false) const;
	///
	VAlignment GetVAlignment(int cell, bool onlycolumn = false) const;
	///
	LyXLength const GetPWidth(int cell) const;
	///
	LyXLength const GetColumnPWidth(int cell) const;
	///
	LyXLength const GetMColumnPWidth(int cell) const;
	///
	string const GetAlignSpecial(int cell, int what) const;
	///
	int GetWidthOfCell(int cell) const;
	///
	int GetBeginningOfTextInCell(int cell) const;
	///
	void AppendRow(int cell);
	///
	void DeleteRow(int row);
	///
	void AppendColumn(int cell);
	///
	void DeleteColumn(int column);
	///
	bool IsFirstCellInRow(int cell) const;
	///
	int GetFirstCellInRow(int row) const;
	///
	bool IsLastCellInRow(int cell) const;
	///
	int GetLastCellInRow(int row) const;
	///
	int GetNumberOfCells() const;
	///
	int NumberOfCellsInRow(int cell) const;
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex &);
	///
	void OldFormatRead(LyXLex &, string const &);
	//
	// helper function for Latex returns number of newlines
	///
	int TeXTopHLine(std::ostream &, int row) const;
	///
	int TeXBottomHLine(std::ostream &, int row) const;
	///
	int TeXCellPreamble(std::ostream &, int cell) const;
	///
	int TeXCellPostamble(std::ostream &, int cell) const;
	///
	int TeXLongtableHeaderFooter(std::ostream &, Buffer const * buf,
	                             bool fragile, bool fp) const;
	///
	bool isValidRow(int const row) const;
	///
	int TeXRow(std::ostream &, int const row, Buffer const * buf,
	           bool fragile, bool fp) const;
	///
	int Latex(Buffer const *, std::ostream &, bool, bool) const;
	/// auxiliary function for docbook rows
	int docbookRow(Buffer const * buf, std::ostream & os, int row) const;
	///
	int DocBook(Buffer const * buf, std::ostream & os) const;
	///
	// helper function for Latex returns number of newlines
	///
	int AsciiTopHLine(std::ostream &, int row,
			  std::vector<unsigned int> const &) const;
	///
	int AsciiBottomHLine(std::ostream &, int row,
			     std::vector<unsigned int> const &) const;
	///
	int AsciiPrintCell(Buffer const *, std::ostream &,
			   int cell, int row, int column,
			   std::vector<unsigned int> const &) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	bool IsMultiColumn(int cell, bool real = false) const;
	///
	void SetMultiColumn(int cell, int number);
	///
	int UnsetMultiColumn(int cell); // returns number of new cells
	///
	bool IsPartOfMultiColumn(int row, int column) const;
	///
	int row_of_cell(int cell) const;
	///
	int column_of_cell(int cell) const;
	///
	int right_column_of_cell(int cell) const;
	///
	void SetLongTabular(bool);
	///
	bool IsLongTabular() const;
	///
	void SetRotateTabular(bool);
	///
	bool GetRotateTabular() const;
	///
	void SetRotateCell(int cell, bool);
	///
	bool GetRotateCell(int cell) const;
	///
	bool NeedRotating() const;
	///
	bool IsLastCell(int cell) const;
	///
	int GetCellAbove(int cell) const;
	///
	int GetCellBelow(int cell) const;
	///
	int GetLastCellAbove(int cell) const;
	///
	int GetLastCellBelow(int cell) const;
	///
	int GetCellNumber(int row, int column) const;
	///
	void SetUsebox(int cell, BoxType);
	///
	BoxType GetUsebox(int cell) const;
	//
	// Long Tabular Options support functions
	///
	bool checkLTType(int row, ltType const &) const;
	///
	void SetLTHead(int row, bool flag, ltType const &, bool first);
	///
	bool GetRowOfLTHead(int row, ltType &) const;
	///
	bool GetRowOfLTFirstHead(int row, ltType &) const;
	///
	void SetLTFoot(int row, bool flag, ltType const &, bool last);
	///
	bool GetRowOfLTFoot(int row, ltType &) const;
	///
	bool GetRowOfLTLastFoot(int row, ltType &) const;
	///
	void SetLTNewPage(int row, bool what);
	///
	bool GetLTNewPage(int row) const;
	///
	bool haveLTHead() const;
	///
	bool haveLTFirstHead() const;
	///
	bool haveLTFoot() const;
	///
	bool haveLTLastFoot() const;
	///
	// end longtable support
	///
	InsetText * GetCellInset(int cell) const;
	///
	InsetText * GetCellInset(int row, int column) const;
	///
	int rows() const { return rows_; }
	///
	int columns() const { return columns_;}
	///
	InsetTabular * owner() const { return owner_; }
	///
	void Validate(LaTeXFeatures &) const;
	///
	std::vector<string> const getLabelList() const;
	///
	/// recalculate the widths/heights only!
	void reinit();
	///
	mutable int cur_cell;
private:
	///
	struct cellstruct {
		///
		cellstruct();
		///
		int cellno;
		///
		int width_of_cell;
		///
		int multicolumn;
		///
		LyXAlignment alignment;
		///
		VAlignment valignment;
		///
		bool top_line;
		///
		bool bottom_line;
		///
		bool left_line;
		///
		bool right_line;
		///
		BoxType usebox;
		///
		bool rotate;
		///
		string align_special;
		///
		LyXLength p_width; // this is only set for multicolumn!!!
		///
		InsetText inset;
	};
	///
	typedef std::vector<cellstruct> cell_vector;
	///
	typedef std::vector<cell_vector> cell_vvector;

	///
	struct rowstruct {
		///
		rowstruct();
		///
		int ascent_of_row;
		///
		int descent_of_row;
		///
		bool top_line;
		///
		bool bottom_line;
		/// This are for longtabulars only
		/// a row of endhead
		bool endhead;
		/// a row of endfirsthead
		bool endfirsthead;
		/// a row of endfoot
		bool endfoot;
		/// row of endlastfoot
		bool endlastfoot;
		/// row for a pagebreak
		bool newpage;
	};
	///
	typedef std::vector<rowstruct> row_vector;

	///
	struct columnstruct {
		///
		columnstruct();
		///
		LyXAlignment alignment;
		///
		VAlignment valignment;
		///
		bool left_line;
		///
		bool right_line;
		///
		int  width_of_column;
		///
		LyXLength p_width;
		///
		string align_special;
	};
	///
	typedef std::vector<columnstruct> column_vector;

	///
	void ReadNew(Buffer const * buf, std::istream & is,
				 LyXLex & lex, string const & l, int const version);
	///
	void ReadOld(Buffer const * buf, std::istream & is,
				 LyXLex & lex, string const & l);
	///
	int rows_;
	///
	int columns_;
	///
	int numberofcells;
	///
	std::vector<int> rowofcell;
	///
	std::vector<int> columnofcell;
	///
	row_vector row_info;
	///
	column_vector column_info;
	///
	mutable cell_vvector cell_info;
	///
	int width_of_tabular;
	///
	bool rotate;
	//
	// for long tabulars
	//
	bool is_long_tabular;
	/// endhead data
	ltType endhead;
	/// endfirsthead data
	ltType endfirsthead;
	/// endfoot data
	ltType endfoot;
	/// endlastfoot data
	ltType endlastfoot;
	//
	///
	InsetTabular * owner_;

	///
	void Init(int columns_arg, int rows_arg, LyXTabular const * lt = 0);
	///
	void Reinit(bool reset_widths = true);
	///
	void set_row_column_number_info(bool oldformat = false);
	/// Returns true if a complete update is necessary, otherwise false
	bool SetWidthOfMulticolCell(int cell, int new_width);
	///
	void recalculateMulticolCells(int cell, int new_width);
	/// Returns true if change
	bool calculate_width_of_column(int column);
	///
	bool calculate_width_of_column_NMC(int column); // no multi cells
	///
	void calculate_width_of_tabular();
	///
	cellstruct * cellinfo_of_cell(int cell) const;
	///
	void delete_column(int column);
	///
	int cells_in_multicolumn(int cell) const;
	///
	BoxType UseParbox(int cell) const;
	///
	void setHeaderFooterRows(int header, int fheader, int footer, int lfooter);
};

#endif
