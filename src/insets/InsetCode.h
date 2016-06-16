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
	ARG_CODE,
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
	MATH_MACROARG_CODE, // 40
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
	///
	PHANTOM_CODE,
	///
	MATH_AMSARRAY_CODE,
	///
	MATH_ARRAY_CODE,
	///
	MATH_BIG_CODE, // 50
	///
	MATH_BOLDSYMBOL_CODE,
	///
	MATH_BOX_CODE,
	///
	MATH_BRACE_CODE,
	///
	MATH_CANCEL_CODE,
	///
	MATH_CANCELTO_CODE, // 55
	///
	MATH_CASES_CODE,
	///
	MATH_CHAR_CODE, 
	///
	MATH_COLOR_CODE,
	///
	MATH_COMMENT_CODE,
	///
	MATH_DECORATION_CODE, // 60
	///
	MATH_DELIM_CODE,
	///
	MATH_DIFF_CODE,
	///
	MATH_DOTS_CODE,
	///
	MATH_ENSUREMATH_CODE,
	///
	MATH_ENV_CODE, // 65
	///
	MATH_EXFUNC_CODE,
	///
	MATH_EXINT_CODE,
	///
	MATH_FONT_CODE,
	///
	MATH_FONTOLD_CODE,
	///
	MATH_FRAC_CODE, // 70
	///
	MATH_GRID_CODE,
	///
	MATH_HULL_CODE,
	///
	MATH_KERN_CODE,
	///
	MATH_LEFTEQN_CODE,
	///
	MATH_LIM_CODE, // 75
	///
	MATH_MATRIX_CODE,
	///
	MATH_MBOX_CODE,
	///
	MATH_NEST_CODE,
	///
	MATH_NUMBER_CODE,
	///
	MATH_OVERSET_CODE, // 80
	///
	MATH_PAR_CODE,
	///
	MATH_PHANTOM_CODE,
	///
	MATH_REF_CODE,
	///
	MATH_ROOT_CODE,
	///
	MATH_SCRIPT_CODE, // 85
	///
	MATH_SIZE_CODE,
	///
	MATH_SPACE_CODE,
	///
	MATH_SPECIALCHAR_CODE,
	///
	MATH_SPLIT_CODE,
	///
	MATH_SQRT_CODE, // 90
	///
	MATH_STACKREL_CODE,
	///
	MATH_STRING_CODE,
	///
	MATH_SUBSTACK_CODE,
	///
	MATH_SYMBOL_CODE,
	///
	MATH_TABULAR_CODE, // 95
	///
	MATH_UNDERSET_CODE,
	///
	MATH_UNKNOWN_CODE,
	///
	MATH_XARROW_CODE,
	///
	MATH_XYMATRIX_CODE,
	///
	MATH_MACRO_CODE, // 100
	///
	ARGUMENT_PROXY_CODE,
	///
	PREVIEW_CODE,
	///
	MATH_DIAGRAM_CODE, 
	///
	SCRIPT_CODE,
	///
	IPA_CODE, // 105
	///
	IPACHAR_CODE,
	///
	IPADECO_CODE,
	///
	INSET_CODE_SIZE
};

} // namespace lyx

#endif
