// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000 The LyX Team.
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
#include "LString.h"

class InsetTabular;
class InsetText;

/* The features the text class offers for tables */ 

///
class LyXTabular  {
public:
    // Are the values of these enums important? (Lgb)
    enum {
	APPEND_ROW = 0,
	APPEND_COLUMN,
	DELETE_ROW,
	DELETE_COLUMN,
	TOGGLE_LINE_TOP,
	TOGGLE_LINE_BOTTOM,
	TOGGLE_LINE_LEFT,
	TOGGLE_LINE_RIGHT,
	ALIGN_LEFT, // what are these alignment enums used for?
	ALIGN_RIGHT,
	ALIGN_CENTER,
	DELETE_TABULAR,
	MULTICOLUMN,
	SET_ALL_LINES,
	UNSET_ALL_LINES,
	SET_LONGTABULAR,
	UNSET_LONGTABULAR,
	SET_PWIDTH,
	SET_ROTATE_TABULAR,
	UNSET_ROTATE_TABULAR,
	SET_ROTATE_CELL,
	UNSET_ROTATE_CELL,
	SET_LINEBREAKS,
	SET_LTHEAD,
	SET_LTFIRSTHEAD,
	SET_LTFOOT,
	SET_LTLASTFOOT,
	SET_LTNEWPAGE,
	SET_SPECIAL_COLUMN,
	SET_SPECIAL_MULTI
    };

    enum {
	CELL_NORMAL = 0,
	CELL_BEGIN_OF_MULTICOLUMN,
	CELL_PART_OF_MULTICOLUMN
    };
    /* konstruktor */
    ///
    LyXTabular(InsetTabular *, int columns_arg, int rows_arg);
    ///
    ///
    LyXTabular(InsetTabular *, LyXTabular const &);
    ///
    explicit
    LyXTabular(InsetTabular *, LyXLex & lex);
    ///
    ~LyXTabular();
    ///
    LyXTabular & operator=(LyXTabular const &);
    ///
    LyXTabular * Clone(InsetTabular *);
    
    /// Returns true if there is a topline, returns false if not
    bool TopLine(int cell) const;
    /// Returns true if there is a topline, returns false if not
    bool BottomLine(int cell) const;
    /// Returns true if there is a topline, returns false if not
    bool LeftLine(int cell) const;
    /// Returns true if there is a topline, returns false if not
    bool RightLine(int cell) const;
    
    ///
    bool TopAlreadyDrawed(int cell) const;
    ///
    bool LeftAlreadyDrawed(int cell) const;
    ///
    bool IsLastRow(int cell) const;
    
    ///
    int GetAdditionalHeight(int cell) const;
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
    ///
    void SetAscentOfRow(int row, int height);
    ///
    void SetDescentOfRow(int row, int height);
    /// Returns true if a complete update is necessary, otherwise false
    bool SetWidthOfCell(int cell, int new_width);
    /// Returns true if a complete update is necessary, otherwise false
    bool SetAllLines(int cell, bool line);
    /// Returns true if a complete update is necessary, otherwise false
    bool SetTopLine(int cell, bool line);
    /// Returns true if a complete update is necessary, otherwise false
    bool SetBottomLine(int cell, bool line);
    /// Returns true if a complete update is necessary, otherwise false
    bool SetLeftLine(int cell, bool line);
    /// Returns true if a complete update is necessary, otherwise false
    bool SetRightLine(int cell, bool line);
    /// Returns true if a complete update is necessary, otherwise false
    bool SetAlignment(int cell, char align);
    ///
    bool SetPWidth(int cell, string width);
    ///
    bool SetAlignSpecial(int cell, string special, int what);
    ///
    char GetAlignment(int cell) const; // add approp. signedness
    ///
    string GetPWidth(int cell) const;
    ///
    string GetAlignSpecial(int cell, int what) const;
    ///
    int GetWidthOfCell(int cell) const;
    ///
    int GetBeginningOfTextInCell(int cell) const;
    ///
    void AppendRow(int cell);
    ///
    void DeleteRow(int cell);
    ///
    void AppendColumn(int cell);
    ///
    void DeleteColumn(int cell);
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
    int AppendCellAfterCell(int append_cell, int question_cell);
    ///
    int DeleteCellIfColumnIsDeleted(int cell, int delete_column_cell);
    ///
    int NumberOfCellsInRow(int cell) const;
    ///
    void Write(std::ostream &) const;
    ///
    void Read(LyXLex &);
    ///
    void OldFormatRead(std::istream &, string);
    ///
    /// helper function for Latex returns number of newlines
    ///
    int TeXTopHLine(std::ostream &, int row) const;
    int TeXBottomHLine(std::ostream &, int row) const;
    int TeXCellPreamble(std::ostream &, int cell) const;
    int TeXCellPostamble(std::ostream &, int cell) const;
    ///
    int Latex(std::ostream &, bool, bool) const;
    ///
    int DocBookEndOfCell(std::ostream &, int cell, int & depth) const;
#if 0
    ///
    int RoffEndOfCell(std::ostream &, int cell);
#endif
    ///
    char const * GetDocBookAlign(int cell, bool isColumn = false) const;

    ///
    bool IsMultiColumn(int cell) const;
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
    void SetLongTabular(int what);
    ///
    bool IsLongTabular() const;
    ///
    void SetRotateTabular(int what);
    ///
    bool GetRotateTabular() const;
    ///
    void SetRotateCell(int cell, int what);
    ///
    bool GetRotateCell(int cell) const;
    ///
    bool NeedRotating() const;
    ///
    bool IsLastCell(int cell) const;
    ///
    int GetCellAbove(int cell) const;
    ///
    int GetCellNumber(int row, int column) const;
    ///
    void SetLinebreaks(int cell, bool what);
    ///
    bool GetLinebreaks(int cell) const;
    ///
    /// Long Tabular Options
    ///
    void SetLTHead(int cell, bool first);
    ///
    bool GetRowOfLTHead(int cell) const;
    ///
    bool GetRowOfLTFirstHead(int cell) const;
    ///
    void SetLTFoot(int cell, bool last);
    ///
    bool GetRowOfLTFoot(int cell) const;
    ///
    bool GetRowOfLTLastFoot(int cell) const;
    ///
    void SetLTNewPage(int cell, bool what);
    ///
    bool GetLTNewPage(int cell) const;
    ///
    InsetText * GetCellInset(int cell) const;
    ///
    int rows() const { return rows_; }
    ///
    int columns() const { return columns_;}
    ///
    InsetTabular * owner() const { return owner_; }

private: //////////////////////////////////////////////////////////////////
    ///
    struct cellstruct {
	///
        cellstruct();
	///
	~cellstruct();
	///
        cellstruct(cellstruct const &);
	///
	cellstruct & operator=(cellstruct const &);
	///
	int cellno;
	///
	int width_of_cell;
	///
	int  multicolumn; // add approp. signedness
	///
	int alignment; // add approp. signedness
	///
	bool top_line;
	///
	bool bottom_line;
	///
	bool linebreaks;
	///
	int rotate;
	///
	string align_special;
	///
	string p_width; // this is only set for multicolumn!!!
	///
	InsetText * inset;
    };
    ///
    struct rowstruct {
	///
        rowstruct();
	///
	    //~rowstruct();
	///
	    // rowstruct & operator=(rowstruct const &);
	///
	bool top_line;
	bool bottom_line;
        int ascent_of_row;
        int descent_of_row;
	/// This are for longtabulars only
	bool newpage;
    };
    ///
    struct columnstruct {
	///
        columnstruct();
	///
	    //~columnstruct();
	///
	    //columnstruct & operator=(columnstruct const &);
	///
	int alignment; // add approp. signedness
	bool left_line;
	bool right_line;
	int  width_of_column;
	string p_width;
	string align_special;
    };
    ///
    int rows_;
    ///
    int columns_;
    ///
    int numberofcells;
    ///
    int * rowofcell;
    ///
    int * columnofcell;
    ///
    std::vector<rowstruct> row_info;
    ///
    std::vector<columnstruct> column_info;
    ///
    mutable std::vector< std::vector<cellstruct> > cell_info;
    ///
    int width_of_tabular;
    ///
    /// for long tabulars
    ///
    int endhead; // row of endhead
    int endfirsthead; // row of endfirsthead
    int endfoot; // row of endfoot
    int endlastfoot; // row of endlastfoot
    ///
    InsetTabular * owner_;
   
    ///
    void Init(int columns_arg, int rows_arg);
    ///
    void Reinit();
    ///
    void set_row_column_number_info();
    /// Returns true if a complete update is necessary, otherwise false
    bool SetWidthOfMulticolCell(int cell, int new_width);
    void recalculateMulticolCells(int cell, int new_width);
    /// Returns true if change
    bool calculate_width_of_column(int column);
    bool calculate_width_of_column_NMC(int column); // no multi cells
    ///
    void calculate_width_of_tabular();

    ///
    int right_column_of_cell(int cell) const;

    ///
    cellstruct * cellinfo_of_cell(int cell) const;

    ///
    void delete_column(int column);

    ///
    int cells_in_multicolumn(int cell) const;
    ///
    int is_long_tabular;
    ///
    int rotate;
};

#endif
