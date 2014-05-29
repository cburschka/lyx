// -*- C++ -*-
/**
 * \file Layout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LAYOUTENUMS_H
#define LAYOUTENUMS_H

// Do not include anything here

namespace lyx {

/// The different output types
enum OutputType {
	///
	LATEX = 1,
	///
	DOCBOOK,
	///
	LITERATE
};


/// The different margin types
enum MarginType {
	///
	MARGIN_MANUAL = 1,
	///
	MARGIN_FIRST_DYNAMIC,
	///
	MARGIN_DYNAMIC,
	///
	MARGIN_STATIC,
	///
	MARGIN_RIGHT_ADDRESS_BOX
};


///
enum LyXAlignment {
	///
	LYX_ALIGN_NONE = 0,
	///
	LYX_ALIGN_BLOCK = 1,
	///
	LYX_ALIGN_LEFT = 2,
	///
	LYX_ALIGN_RIGHT = 4,
	///
	LYX_ALIGN_CENTER = 8,
	///
	LYX_ALIGN_LAYOUT = 16,
	///
	LYX_ALIGN_SPECIAL = 32,
	///
	LYX_ALIGN_DECIMAL = 64
};


/// Paragraph indendation control
enum ToggleIndentation {
	/// Indentation can be toggled if document uses "indent" paragraph style
	ITOGGLE_DOCUMENT_DEFAULT = 1,
	/// Indentation cannot be toggled
	ITOGGLE_NEVER,
	/// Indentation can always be toggled
	ITOGGLE_ALWAYS
};

///
inline void operator|=(LyXAlignment & la1, LyXAlignment la2)
{
	la1 = static_cast<LyXAlignment>(la1 | la2);
}


///
inline LyXAlignment operator|(LyXAlignment la1, LyXAlignment la2)
{
	return static_cast<LyXAlignment>(int(la1) | int(la2));
}


/// The different LaTeX-Types
enum LatexType {
	///
	LATEX_PARAGRAPH = 1,
	///
	LATEX_COMMAND,
	///
	LATEX_ENVIRONMENT,
	///
	LATEX_ITEM_ENVIRONMENT,
	///
	LATEX_BIB_ENVIRONMENT,
	///
	LATEX_LIST_ENVIRONMENT
};


/// The different title types
enum TitleLatexType {
	///
	TITLE_COMMAND_AFTER = 1,
	///
	TITLE_ENVIRONMENT
};


/// The different label types
enum LabelType {
	///
	LABEL_NO_LABEL,
	///
	LABEL_MANUAL,
	///
	LABEL_BIBLIO,
	///
	LABEL_ABOVE,
	///
	LABEL_CENTERED,
	///
	LABEL_STATIC,
	///
	LABEL_SENSITIVE,
	///
	LABEL_ENUMERATE,
	///
	LABEL_ITEMIZE
};


///
enum EndLabelType {
	///
	END_LABEL_NO_LABEL,
	///
	END_LABEL_BOX,
	///
	END_LABEL_FILLED_BOX,
	///
	END_LABEL_STATIC
};

} // namespace lyx

#endif
