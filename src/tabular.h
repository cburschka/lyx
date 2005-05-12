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

#include <boost/shared_ptr.hpp>

#include <iosfwd>
#include <vector>

class InsetTabular;
class OutputParams;

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

	class ltType {
	public:
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

	/// type for row numbers
	typedef size_t row_type;
	/// type for column numbers
	typedef size_t col_type;
	/// type for cell indices
	typedef size_t idx_type;
	/// index indicating an invalid position
	static const idx_type npos = static_cast<idx_type>(-1);

	/// constructor
	LyXTabular(BufferParams const &, col_type columns_arg,
	           row_type rows_arg);

	/// Returns true if there is a topline, returns false if not
	bool topLine(idx_type cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool bottomLine(idx_type cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool leftLine(idx_type cell, bool onlycolumn = false) const;
	/// Returns true if there is a topline, returns false if not
	bool rightLine(idx_type cell, bool onlycolumn = false) const;

	///
	bool topAlreadyDrawn(idx_type cell) const;
	///
	bool leftAlreadyDrawn(idx_type cell) const;
	///
	bool isLastRow(idx_type cell) const;

	///
	int getAdditionalHeight(row_type row) const;
	///
	int getAdditionalWidth(idx_type cell) const;

	/* returns the maximum over all rows */
	///
	int getWidthOfColumn(idx_type cell) const;
	///
	int getWidthOfTabular() const;
	///
	int getAscentOfRow(row_type row) const;
	///
	int getDescentOfRow(row_type row) const;
	///
	int getHeightOfTabular() const;
	///
	void setAscentOfRow(row_type row, int height);
	///
	void setDescentOfRow(row_type row, int height);
	///
	void setWidthOfCell(idx_type cell, int new_width);
	///
	void setAllLines(idx_type cell, bool line);
	///
	void setTopLine(idx_type cell, bool line, bool onlycolumn = false);
	///
	void setBottomLine(idx_type cell, bool line, bool onlycolumn = false);
	///
	void setLeftLine(idx_type cell, bool line, bool onlycolumn = false);
	///
	void setRightLine(idx_type cell, bool line, bool onlycolumn = false);
	///
	void setAlignment(idx_type cell, LyXAlignment align,
			  bool onlycolumn = false);
	///
	void setVAlignment(idx_type cell, VAlignment align,
			   bool onlycolumn = false);
	///
	void setColumnPWidth(idx_type cell, LyXLength const & width);
	///
	bool setMColumnPWidth(idx_type cell, LyXLength const & width);
	///
	void setAlignSpecial(idx_type cell, std::string const & special,
	                     Feature what);
	///
	LyXAlignment getAlignment(idx_type cell,
	                          bool onlycolumn = false) const;
	///
	VAlignment getVAlignment(idx_type cell,
	                         bool onlycolumn = false) const;
	///
	LyXLength const getPWidth(idx_type cell) const;
	///
	LyXLength const getColumnPWidth(idx_type cell) const;
	///
	LyXLength const getMColumnPWidth(idx_type cell) const;
	///
	std::string const getAlignSpecial(idx_type cell, int what) const;
	///
	int getWidthOfCell(idx_type cell) const;
	///
	int getBeginningOfTextInCell(idx_type cell) const;
	///
	void appendRow(BufferParams const &, idx_type cell);
	///
	void deleteRow(row_type row);
	///
	void appendColumn(BufferParams const &, idx_type cell);
	///
	void deleteColumn(col_type column);
	///
	bool isFirstCellInRow(idx_type cell) const;
	///
	idx_type getFirstCellInRow(row_type row) const;
	///
	bool isLastCellInRow(idx_type cell) const;
	///
	idx_type getLastCellInRow(row_type row) const;
	///
	idx_type getNumberOfCells() const;
	///
	idx_type numberOfCellsInRow(idx_type cell) const;
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, LyXLex &);
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	//
	int linuxdoc(Buffer const & buf, std::ostream & os,
		     OutputParams const &) const;
	///
	int docbook(Buffer const & buf, std::ostream & os,
		    OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const & runparams,
		  int const depth,
		  bool onlydata, unsigned char delim) const;
	///
	bool isMultiColumn(idx_type cell) const;
	///
	bool isMultiColumnReal(idx_type cell) const;
	///
	void setMultiColumn(Buffer *, idx_type cell, idx_type number);
	///
	idx_type unsetMultiColumn(idx_type cell); // returns number of new cells
	///
	bool isPartOfMultiColumn(row_type row, col_type column) const;
	///
	row_type row_of_cell(idx_type cell) const;
	///
	col_type column_of_cell(idx_type cell) const;
	///
	col_type right_column_of_cell(idx_type cell) const;
	///
	void setLongTabular(bool);
	///
	bool isLongTabular() const;
	///
	void setRotateTabular(bool);
	///
	bool getRotateTabular() const;
	///
	void setRotateCell(idx_type cell, bool);
	///
	bool getRotateCell(idx_type cell) const;
	///
	bool needRotating() const;
	///
	bool isLastCell(idx_type cell) const;
	///
	idx_type getCellAbove(idx_type cell) const;
	///
	idx_type getCellBelow(idx_type cell) const;
	///
	idx_type getLastCellAbove(idx_type cell) const;
	///
	idx_type getLastCellBelow(idx_type cell) const;
	///
	idx_type getCellNumber(row_type row, col_type column) const;
	///
	void setUsebox(idx_type cell, BoxType);
	///
	BoxType getUsebox(idx_type cell) const;
	//
	// Long Tabular Options support functions
	///
	bool checkLTType(row_type row, ltType const &) const;
	///
	void setLTHead(row_type row, bool flag, ltType const &, bool first);
	///
	bool getRowOfLTHead(row_type row, ltType &) const;
	///
	bool getRowOfLTFirstHead(row_type row, ltType &) const;
	///
	void setLTFoot(row_type row, bool flag, ltType const &, bool last);
	///
	bool getRowOfLTFoot(row_type row, ltType &) const;
	///
	bool getRowOfLTLastFoot(row_type row, ltType &) const;
	///
	void setLTNewPage(row_type row, bool what);
	///
	bool getLTNewPage(row_type row) const;
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
	boost::shared_ptr<InsetText> getCellInset(idx_type cell) const;
	///
	boost::shared_ptr<InsetText> getCellInset(row_type row,
	                                          col_type column) const;
	///
	void setCellInset(row_type row, col_type column, 
			  boost::shared_ptr<InsetText>) const;
	/// Search for \param inset in the tabular, with the
	///
	idx_type getCellFromInset(InsetBase const * inset) const;
	///
	row_type rows() const { return rows_; }
	///
	col_type columns() const { return columns_;}
	///
	void validate(LaTeXFeatures &) const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	///
//private:
	///
	class cellstruct {
	public:
		///
		cellstruct(BufferParams const &);
		///
		cellstruct(cellstruct const &);
		///
		cellstruct & operator=(cellstruct);
		///
		void swap(cellstruct & rhs);
		///
		idx_type cellno;
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
		boost::shared_ptr<InsetText> inset;
	};
	cellstruct & cellinfo_of_cell(idx_type cell) const;
	///
	typedef std::vector<cellstruct> cell_vector;
	///
	typedef std::vector<cell_vector> cell_vvector;

	///
	class rowstruct {
	public:
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
	class columnstruct {
		public:
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
	row_type rows_;
	///
	col_type columns_;
	///
	idx_type numberofcells;
	///
	std::vector<row_type> rowofcell;
	///
	std::vector<col_type> columnofcell;
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
	void init(BufferParams const &, row_type rows_arg,
	          col_type columns_arg);
	///
	void set_row_column_number_info();
	/// Returns true if a complete update is necessary, otherwise false
	bool setWidthOfMulticolCell(idx_type cell, int new_width);
	///
	void recalculateMulticolumnsOfColumn(col_type column);
	/// Returns true if change
	void calculate_width_of_column(col_type column);
	///
	bool calculate_width_of_column_NMC(col_type column); // no multi cells
	///
	void calculate_width_of_tabular();
	///
	void delete_column(col_type column);
	///
	idx_type cells_in_multicolumn(idx_type cell) const;
	///
	BoxType useParbox(idx_type cell) const;
	///
	// helper function for Latex returns number of newlines
	///
	int TeXTopHLine(std::ostream &, row_type row) const;
	///
	int TeXBottomHLine(std::ostream &, row_type row) const;
	///
	int TeXCellPreamble(std::ostream &, idx_type cell) const;
	///
	int TeXCellPostamble(std::ostream &, idx_type cell) const;
	///
	int TeXLongtableHeaderFooter(std::ostream &, Buffer const & buf,
				     OutputParams const &) const;
	///
	bool isValidRow(row_type const row) const;
	///
	int TeXRow(std::ostream &, row_type const row, Buffer const & buf,
		   OutputParams const &) const;
	///
	// helper function for ASCII returns number of newlines
	///
	int asciiTopHLine(std::ostream &, row_type row,
			  std::vector<unsigned int> const &) const;
	///
	int asciiBottomHLine(std::ostream &, row_type row,
			     std::vector<unsigned int> const &) const;
	///
	int asciiPrintCell(Buffer const &, std::ostream &,
			   OutputParams const &,
			   idx_type cell, row_type row, col_type column,
			   std::vector<unsigned int> const &,
					   bool onlydata) const;
	/// auxiliary function for docbook
	int docbookRow(Buffer const & buf, std::ostream & os, row_type,
		       OutputParams const &) const;

private:
	/// renumber cells after structural changes
	void fixCellNums();
};

#endif
