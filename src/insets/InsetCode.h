// -*- C++ -*-
/**
 * \file InsetCode.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCODE_H
#define INSETCODE_H

namespace lyx {

enum InsetCode {
	///
	NO_CODE, // 0
	///
	TOC_CODE,  // do these insets really need a code? (ale)
	///
	QUOTE_CODE,
	///
	MARK_CODE,
	///
	REF_CODE,
	///
	HYPERLINK_CODE, // 5
	///
	SEPARATOR_CODE,
	///
	ENDING_CODE,
	///
	LABEL_CODE,
	///
	NOTE_CODE,
	///
	ACCENT_CODE, // 10
	///
	MATH_CODE,
	///
	INDEX_CODE,
	///
	INCLUDE_CODE,
	///
	GRAPHICS_CODE,
	///
	BIBITEM_CODE, // 15
	///
	BIBTEX_CODE,
	///
	TEXT_CODE,
	///
	ERT_CODE,
	///
	FOOT_CODE,
	///
	MARGIN_CODE,  // 20
	///
	FLOAT_CODE,
	///
	WRAP_CODE,
	///
	SPACE_CODE,
	///
	SPECIALCHAR_CODE,
	///
	TABULAR_CODE, // 25
	///
	EXTERNAL_CODE,
	///
	CAPTION_CODE,
	///
	MATHMACRO_CODE,
	///
	CITE_CODE,
	///
	FLOAT_LIST_CODE, // 30
	///
	INDEX_PRINT_CODE,
	///
	OPTARG_CODE,
	///
	CELL_CODE,
	///
	NEWLINE_CODE,
	///
	LINE_CODE, // 35
	///
	BRANCH_CODE,
	///
	BOX_CODE,
	///
	FLEX_CODE,
	///
	VSPACE_CODE,
	///
	MATHMACROARG_CODE, // 40
	///
	NOMENCL_CODE,
	///
	NOMENCL_PRINT_CODE,
	///
	NEWPAGE_CODE,
	///
	LISTINGS_CODE,
	///
	INFO_CODE, // 45
	///
	COLLAPSABLE_CODE,
};

} // namespace lyx

#endif
