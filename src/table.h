// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */
#ifndef TABLE_H
#define TABLE_H

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
class LyXTable  {
public:
    // Are the values of these enums important? (Lgb)
    enum {
	APPEND_ROW = 0,
	APPEND_COLUMN = 1,
	DELETE_ROW = 2,
	DELETE_COLUMN = 3,
	TOGGLE_LINE_TOP = 4,
	TOGGLE_LINE_BOTTOM = 5,
	TOGGLE_LINE_LEFT = 6,
	TOGGLE_LINE_RIGHT = 7,
	ALIGN_LEFT = 8, // what are these alignment enums used for?
	ALIGN_RIGHT = 9,
	ALIGN_CENTER = 10,
	DELETE_TABLE = 11,
	MULTICOLUMN = 12,
	SET_ALL_LINES = 13,
	UNSET_ALL_LINES = 14,
	SET_LONGTABLE = 15,
	UNSET_LONGTABLE = 16,
	SET_PWIDTH = 17,
	APPEND_CONT_ROW = 18,
	SET_ROTATE_TABLE = 19,
	UNSET_ROTATE_TABLE = 20,
	SET_ROTATE_CELL = 21,
	UNSET_ROTATE_CELL = 22,
	SET_LINEBREAKS = 23,
	SET_LTHEAD = 24,
	SET_LTFIRSTHEAD = 25,
	SET_LTFOOT = 26,
	SET_LTLASTFOOT = 27,
	SET_LTNEWPAGE = 28,
	SET_SPECIAL_COLUMN = 29,
	SET_SPECIAL_MULTI = 30
    };

    enum {
	CELL_NORMAL = 0,
	CELL_BEGIN_OF_MULTICOLUMN = 1,
	CELL_PART_OF_MULTICOLUMN = 2
    };
    /* konstruktor */
    ///
    LyXTable(int columns_arg, int rows_arg, Buffer *buf = 0);
    ///
    ///
    LyXTable(LyXTable const &, Buffer *buf = 0);
    ///
    explicit
    LyXTable(LyXLex & lex, Buffer *buf = 0);
    ///
    ~LyXTable();
    ///
    LyXTable & operator=(LyXTable const &);
    ///
    LyXTable * Clone();
    
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
    int WidthOfTable();
    ///
    int AscentOfRow(int row);
    ///
    int DescentOfRow(int row);
    ///
    int HeightOfTable();
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
    bool IsFirstCell(int cell);
    ///
    bool IsLastCell(int cell);
    
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
    int rows;
    ///
    int columns;
    ///
    void SetLongTable(int what);
    ///
    bool IsLongTable();
    ///
    void SetRotateTable(int what);
    ///
    bool RotateTable();
    ///
    void SetRotateCell(int cell, int what);
    ///
    bool RotateCell(int cell);
    ///
    bool NeedRotating();
    ///
    void AppendContRow(int cell);
    ///
    bool IsContRow(int cell);
    /// returns the number of the cell which continues
    /// or -1 if no ContRow
    int CellHasContRow(int cell);
    ///
    bool RowHasContRow(int cell);
    ///
    int FirstVirtualCell(int cell);
    ///
    int NextVirtualCell(int cell);
    ///
    bool ShouldBeVeryLastCell(int cell);
    ///
    bool ShouldBeVeryLastRow(int cell);
    ///
    int GetCellAbove(int cell);
    ///
    int GetCellNumber(int column, int row);
    ///
    void SetLinebreaks(int cell, bool what);
    ///
    bool Linebreaks(int cell);
    ///
    /// Long Table Options
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
	bool has_cont_row;
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
	bool is_cont_row;
        int ascent_of_row;
        int descent_of_row;
	/// This are for longtables only
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
    int width_of_table;
    ///
    /// for long tables
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
    void calculate_width_of_table();

    ///
    int right_column_of_cell(int cell);

    ///
    cellstruct * cellinfo_of_cell(int cell);

    ///
    void delete_column(int column);

    ///
    int cells_in_multicolumn(int cell);
    ///
    int is_long_table;
    ///
    int rotate;
};

#endif
