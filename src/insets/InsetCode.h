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

/** This is not quite the correct place for this enum. I think
    the correct would be to let each subclass of Inset declare
    its own enum code. Actually the notion of an InsetCode
    should be avoided, but I am not sure how this could be done
    in a cleaner way. */
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
	URL_CODE, // 5
	///
	HTMLURL_CODE,
	///
	SEPARATOR_CODE,
	///
	ENDING_CODE,
	///
	LABEL_CODE,
	///
	NOTE_CODE, // 10
	///
	ACCENT_CODE,
	///
	MATH_CODE,
	///
	INDEX_CODE,
	///
	INCLUDE_CODE,
	///
	GRAPHICS_CODE, // 15
	///
	BIBITEM_CODE,
	///
	BIBTEX_CODE,
	///
	TEXT_CODE,
	///
	ERT_CODE,
	///
	FOOT_CODE, // 20
	///
	MARGIN_CODE,
	///
	FLOAT_CODE,
	///
	WRAP_CODE,
	///
	SPACE_CODE, // 25
	///
	SPECIALCHAR_CODE,
	///
	TABULAR_CODE,
	///
	EXTERNAL_CODE,
#if 0
	///
	THEOREM_CODE,
#endif
	///
	CAPTION_CODE,
	///
	MATHMACRO_CODE, // 30
	///
	CITE_CODE,
	///
	FLOAT_LIST_CODE,
	///
	INDEX_PRINT_CODE,
	///
	OPTARG_CODE, // 35
	///
	ENVIRONMENT_CODE,
	///
	HFILL_CODE,
	///
	NEWLINE_CODE,
	///
	LINE_CODE,
	///
	BRANCH_CODE, // 40
	///
	BOX_CODE,
	///
	FLEX_CODE,
	///
	VSPACE_CODE,
	///
	MATHMACROARG_CODE,
	///
	NOMENCL_CODE, // 45
	///
	NOMENCL_PRINT_CODE,
	///
	PAGEBREAK_CODE,
	///
	LISTINGS_CODE,
	///
	INFO_CODE,
};

} // namespace lyx

#endif
