// -*- C++ -*-
/**
 * \file tabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TABULAR_H
#define TABULAR_H

#include "lyxlength.h"
#include "insets/insettext.h"

#include <iosfwd>
#include <vector>

class InsetTabular;
class LatexRunParams;

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
		ALIGN_BLOCK,
		///
		VALIGN_TOP,
		///
		VALIGN_BOTTOM,
		///
		VALIGN_MIDDLE,
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
		M_VALIGN_MIDDLE,
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
		LYX_VALIGN_MIDDLE = 2
	};

	enum BoxType {
		///
		BOX_NONE = 0,
		///
		BOX_PARBOX = 1,
		///
		BOX_MINIPAGE = 2
	};

	struct ltType {
		// constructor
		ltType();
		// we have this header type (is set in the getLT... functions)
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

	/// constructor
	LyXTabular(BufferParams const &, int columns_arg, int rows_arg);
	///
	void setOwner(InsetTabular * inset);

	/// Returns true if there is a topline, returns false if not
	bool topLine(int cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool bottomLine(int cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool leftLine(int cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool rightLine(int cell, bool onlycolumn = false) const;

	///
	bool topAlreadyDrawn(int cell) const;
	///
	bool leftAlreadyDrawn(int cell) const;
	///
	bool isLastRow(int cell) const;

	///
	int getAdditionalHeight(int row) const;
	///
	int getAdditionalWidth(int cell) const;

	/* returns the maximum over all rows */
	///
	int getWidthOfColumn(int cell) const;
	///
	int getWidthOfTabular() const;
	///
	int getAscentOfRow(int row) const;
	///
	int getDescentOfRow(int row) const;
	///
	int getHeightOfTabular() const;
	/// Returns true if a complete update is necessary, otherwise false
	void setAscentOfRow(int row, int height);
	/// Returns true if a complete update is necessary, otherwise false
	void setDescentOfRow(int row, int height);
	/// Returns true if a complete update is necessary, otherwise false
	void setWidthOfCell(int cell, int new_width);
	///
	void setAllLines(int cell, bool line);
	///
	void setTopLine(int cell, bool line, bool onlycolumn = false);
	///
	void setBottomLine(int cell, bool line, bool onlycolumn = false);
	///
	void setLeftLine(int cell, bool line, bool onlycolumn = false);
	///
	void setRightLine(int cell, bool line, bool onlycolumn = false);
	///
	void setAlignment(int cell, LyXAlignment align,
			  bool onlycolumn = false);
	///
	void setVAlignment(int cell, VAlignment align,
			   bool onlycolumn = false);
	///
	void setColumnPWidth(int cell, LyXLength const & width);
	///
	bool setMColumnPWidth(int cell, LyXLength const & width);
	///
	void setAlignSpecial(int cell, std::string const & special, Feature what);
	///
	LyXAlignment getAlignment(int cell, bool onlycolumn = false) const;
	///
	VAlignment getVAlignment(int cell, bool onlycolumn = false) const;
	///
	LyXLength const getPWidth(int cell) const;
	///
	LyXLength const getColumnPWidth(int cell) const;
	///
	LyXLength const getMColumnPWidth(int cell) const;
	///
	std::string const getAlignSpecial(int cell, int what) const;
	///
	int getWidthOfCell(int cell) const;
	///
	int getBeginningOfTextInCell(int cell) const;
	///
	void appendRow(BufferParams const &, int cell);
	///
	void deleteRow(int row);
	///
	void appendColumn(BufferParams const &, int cell);
	///
	void deleteColumn(int column);
	///
	bool isFirstCellInRow(int cell) const;
	///
	int getFirstCellInRow(int row) const;
	///
	bool isLastCellInRow(int cell) const;
	///
	int getLastCellInRow(int row) const;
	///
	int getNumberOfCells() const;
	///
	int numberOfCellsInRow(int cell) const;
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, LyXLex &);
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	//
	int linuxdoc(Buffer const & buf, std::ostream & os,
		     LatexRunParams const &) const;
	///
	int docbook(Buffer const & buf, std::ostream & os,
		    LatexRunParams const &) const;
	///
	int ascii(Buffer const &, std::ostream &,
		  LatexRunParams const & runparams,
		  int const depth,
		  bool onlydata, unsigned char delim) const;
	///
	bool isMultiColumn(int cell) const;
	///
	bool isMultiColumnReal(int cell) const;
	///
	void setMultiColumn(Buffer *, int cell, int number);
	///
	int unsetMultiColumn(int cell); // returns number of new cells
	///
	bool isPartOfMultiColumn(int row, int column) const;
	///
	int row_of_cell(int cell) const;
	///
	int column_of_cell(int cell) const;
	///
	int right_column_of_cell(int cell) const;
	///
	void setLongTabular(bool);
	///
	bool isLongTabular() const;
	///
	void setRotateTabular(bool);
	///
	bool getRotateTabular() const;
	///
	void setRotateCell(int cell, bool);
	///
	bool getRotateCell(int cell) const;
	///
	bool needRotating() const;
	///
	bool isLastCell(int cell) const;
	///
	int getCellAbove(int cell) const;
	///
	int getCellBelow(int cell) const;
	///
	int getLastCellAbove(int cell) const;
	///
	int getLastCellBelow(int cell) const;
	///
	int getCellNumber(int row, int column) const;
	///
	void setUsebox(int cell, BoxType);
	///
	BoxType getUsebox(int cell) const;
	//
	// Long Tabular Options support functions
	///
	bool checkLTType(int row, ltType const &) const;
	///
	void setLTHead(int row, bool flag, ltType const &, bool first);
	///
	bool getRowOfLTHead(int row, ltType &) const;
	///
	bool getRowOfLTFirstHead(int row, ltType &) const;
	///
	void setLTFoot(int row, bool flag, ltType const &, bool last);
	///
	bool getRowOfLTFoot(int row, ltType &) const;
	///
	bool getRowOfLTLastFoot(int row, ltType &) const;
	///
	void setLTNewPage(int row, bool what);
	///
	bool getLTNewPage(int row) const;
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
	InsetText & getCellInset(int cell) const;
	///
	InsetText & getCellInset(int row, int column) const;
	/// Search for \param inset in the tabular, with the
	///
	int getCellFromInset(InsetOld const * inset) const;
	///
	int rows() const { return rows_; }
	///
	int columns() const { return columns_;}
	///
	void validate(LaTeXFeatures &) const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	///
//private:
	///
	struct cellstruct {
		///
		cellstruct(BufferParams const &);
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
		std::string align_special;
		///
		LyXLength p_width; // this is only set for multicolumn!!!
		///
		InsetText inset;
	};
	cellstruct & cellinfo_of_cell(int cell) const;
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
		std::string align_special;
	};
	///
	typedef std::vector<columnstruct> column_vector;

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

	///
	void init(BufferParams const &, int rows_arg, int columns_arg);
	///
	void set_row_column_number_info();
	/// Returns true if a complete update is necessary, otherwise false
	bool setWidthOfMulticolCell(int cell, int new_width);
	///
	void recalculateMulticolumnsOfColumn(int column);
	/// Returns true if change
	void calculate_width_of_column(int column);
	///
	bool calculate_width_of_column_NMC(int column); // no multi cells
	///
	void calculate_width_of_tabular();
	///
	void delete_column(int column);
	///
	int cells_in_multicolumn(int cell) const;
	///
	BoxType useParbox(int cell) const;
	///
	void setHeaderFooterRows(int header, int fheader, int footer, int lfooter);
	///
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
	int TeXLongtableHeaderFooter(std::ostream &, Buffer const & buf,
				     LatexRunParams const &) const;
	///
	bool isValidRow(int const row) const;
	///
	int TeXRow(std::ostream &, int const row, Buffer const & buf,
		   LatexRunParams const &) const;
	///
	// helper function for ASCII returns number of newlines
	///
	int asciiTopHLine(std::ostream &, int row,
			  std::vector<unsigned int> const &) const;
	///
	int asciiBottomHLine(std::ostream &, int row,
			     std::vector<unsigned int> const &) const;
	///
	int asciiPrintCell(Buffer const &, std::ostream &,
			   LatexRunParams const &,
			   int cell, int row, int column,
			   std::vector<unsigned int> const &,
					   bool onlydata) const;
	/// auxiliary function for docbook
	int docbookRow(Buffer const & buf, std::ostream & os, int,
		       LatexRunParams const &) const;

private:
	/// renumber cells after structural changes
	void fixCellNums();
};

#endif
