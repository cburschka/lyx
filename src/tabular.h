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

#include "lyxlex.h"
#include "LString.h"

class InsetText;
class Buffer;

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
    LyXTabular(int columns_arg, int rows_arg, Buffer *buf = 0);
    ///
    ///
    LyXTabular(LyXTabular const &, Buffer *buf = 0);
    ///
    explicit
    LyXTabular(LyXLex & lex, Buffer *buf = 0);
    ///
    ~LyXTabular();
    ///
    LyXTabular & operator=(LyXTabular const &);
    ///
    LyXTabular * Clone();
    
    /// Returns true if there is a topline, returns false if not
    bool TopLine(int cell);
    /// Returns true if there is a topline, returns false if not
    bool BottomLine(int cell);
    /// Returns true if there is a topline, returns false if not
    bool LeftLine(int cell);
    /// Returns true if there is a topline, returns false if not
    bool RightLine(int cell);
    
    ///
    bool TopAlreadyDrawed(int cell);
    ///
    bool VeryLastRow(int cell);
    
    ///
    int AdditionalHeight(int cell);
    ///
    int AdditionalWidth(int cell);
    
    /* returns the maximum over all rows */
    ///
    int WidthOfColumn(int cell);
    ///
    int WidthOfTabular();
    ///
    int AscentOfRow(int row);
    ///
    int DescentOfRow(int row);
    ///
    int HeightOfTabular();
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
    char GetAlignment(int cell); // add approp. signedness
    ///
    string GetPWidth(int cell);
    ///
    string GetAlignSpecial(int cell, int what);
    ///
    int GetWidthOfCell(int cell);
    ///
    int GetBeginningOfTextInCell(int cell);
    ///
    void AppendRow(int cell);
    ///
    void DeleteRow(int cell);
    ///
    void AppendColumn(int cell);
    ///
    void DeleteColumn(int cell);
    ///
    bool IsFirstCellInRow(int cell);
    ///
    bool IsLastCellInRow(int cell);
    ///
    int GetNumberOfCells();
    ///
    int AppendCellAfterCell(int append_cell, int question_cell);
    ///
    int DeleteCellIfColumnIsDeleted(int cell, int delete_column_cell);
    ///
    int NumberOfCellsInRow(int cell);
    ///
    void Reinit();
    ///
    void Init(int columns_arg, int rows_arg);
    ///
    void Write(std::ostream &, bool old_format=true);
    ///
    void Read(std::istream &);
    ///
    int Latex(std::ostream &);

    // cell <0 will tex the preamble
    // returns the number of printed newlines
    ///
    int TexEndOfCell(std::ostream &, int cell);
    ///
    int DocBookEndOfCell(std::ostream &, int cell, int & depth);
#if 0
    ///
    int RoffEndOfCell(std::ostream &, int cell);
#endif
    ///
    char const * getDocBookAlign(int cell, bool isColumn = false);

    ///
    bool IsMultiColumn(int cell);
    ///
    void SetMultiColumn(int cell, int number);
    ///
    int UnsetMultiColumn(int cell); // returns number of new cells
    ///
    bool IsPartOfMultiColumn(int row, int column);
    ///
    int row_of_cell(int cell) const;
    ///
    int column_of_cell(int cell) const;
    ///
    void SetLongTabular(int what);
    ///
    bool IsLongTabular();
    ///
    void SetRotateTabular(int what);
    ///
    bool RotateTabular();
    ///
    void SetRotateCell(int cell, int what);
    ///
    bool RotateCell(int cell);
    ///
    bool NeedRotating();
    ///
    bool ShouldBeVeryLastCell(int cell);
    ///
    bool IsLastRow(int cell);
    ///
    int GetCellAbove(int cell);
    ///
    int GetCellNumber(int column, int row);
    ///
    void SetLinebreaks(int cell, bool what);
    ///
    bool Linebreaks(int cell);
    ///
    /// Long Tabular Options
    ///
    void SetLTHead(int cell, bool first);
    ///
    bool RowOfLTHead(int cell);
    ///
    bool RowOfLTFirstHead(int cell);
    ///
    void SetLTFoot(int cell, bool last);
    ///
    bool RowOfLTFoot(int cell);
    ///
    bool RowOfLTLastFoot(int cell);
    ///
    void SetLTNewPage(int cell, bool what);
    ///
    bool LTNewPage(int cell);
    ///
    InsetText * GetCellInset(int cell) const;
    ///
    int rows() const { return rows_; }
    ///
    int columns() const { return columns_;}

private: //////////////////////////////////////////////////////////////////
    ///
    struct cellstruct {
	///
        cellstruct(Buffer * buf = 0);
	///
	~cellstruct();
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
	InsetText *inset;
    };
    ///
    struct rowstruct {
	///
        rowstruct();
	///
	~rowstruct();
	///
        rowstruct & operator=(rowstruct const &);
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
	~columnstruct();
	///
        columnstruct & operator=(columnstruct const &);
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
    rowstruct * row_info;
    ///
    columnstruct * column_info;
    ///
    cellstruct ** cell_info;
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
    Buffer *buffer;
   
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
    int right_column_of_cell(int cell);

    ///
    cellstruct * cellinfo_of_cell(int cell);

    ///
    void delete_column(int column);

    ///
    int cells_in_multicolumn(int cell);
    ///
    int is_long_tabular;
    ///
    int rotate;
};

#endif
