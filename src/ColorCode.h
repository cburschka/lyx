// -*- C++ -*-
/**
 * \file ColorCode.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef COLOR_CODE_H
#define COLOR_CODE_H

namespace lyx {

/// Names of colors, including all logical colors
enum ColorCode
{
	/// No particular color---clear or default
	Color_none,
	/// The different text colors
	Color_black,
	///
	Color_white,
	///
	Color_red,
	///
	Color_green,
	///
	Color_blue,
	///
	Color_cyan,
	///
	Color_magenta,
	///
	Color_yellow,

	// Needed interface colors

	/// Cursor color
	Color_cursor,
	/// Background color
	Color_background,
	/// Foreground color
	Color_foreground,
	/// Background color of selected text
	Color_selection,
	/// Foreground color of selected text
	Color_selectiontext,
	/// Text color in LaTeX mode
	Color_latex,
	/// The color used for previews
	Color_preview,
	/// Inline completion color
	Color_inlinecompletion,
	/// Inline completion color for the non-unique part
	Color_nonunique_inlinecompletion,

	/// Label color for notes
	Color_notelabel,
	/// Background color of notes
	Color_notebg,
	/// Label color for comments
	Color_commentlabel,
	/// Background color of comments
	Color_commentbg,
	/// Label color for greyedout insets
	Color_greyedoutlabel,
	/// Background color of greyedout inset
	Color_greyedoutbg,
	/// Shaded box background
	Color_shadedbg,

	/// Label color for branches
	Color_branchlabel,
	/// Label color for footnotes
	Color_footlabel,
	/// Label color for index insets
	Color_indexlabel,
	/// Label color for margin notes
	Color_marginlabel,
	/// Label color for URL insets
	Color_urllabel,

	/// Color for URL inset text
	Color_urltext,

	/// Color for the depth bars in the margin
	Color_depthbar,
	/// Color for marking foreign language words
	Color_language,

	/// Text color for command insets
	Color_command,
	/// Background color for command insets
	Color_commandbg,
	/// Frame color for command insets
	Color_commandframe,

	/// Special chars text color
	Color_special,

	/// Graphics inset background color
	Color_graphicsbg,
	/// Math inset text color
	Color_math,
	/// Math inset background color
	Color_mathbg,
	/// Macro math inset background color
	Color_mathmacrobg,
	/// Macro math inset background color hovered
	Color_mathmacrohoverbg,
	/// Macro math label color
	Color_mathmacrolabel,
	/// Macro math frame color
	Color_mathmacroframe,
	/// Macro math blended color 
	Color_mathmacroblend,
	/// Macro template color for old parameters 
	Color_mathmacrooldarg,
	/// Macro template color for new parameters 
	Color_mathmacronewarg,
	/// Math inset frame color under focus
	Color_mathframe,
	/// Math inset frame color not under focus
	Color_mathcorners,
	/// Math line color
	Color_mathline,

	/// caption frame color
	Color_captionframe,

	/// collapsable insets text
	Color_collapsable,
	/// collapsable insets frame
	Color_collapsableframe,

	/// Inset marker background color
	Color_insetbg,
	/// Inset marker frame color
	Color_insetframe,

	/// Error box text color
	Color_error,
	/// EOL marker color
	Color_eolmarker,
	/// Added space colour
	Color_added_space,
	/// Appendix marker color
	Color_appendix,
	/// changebar color
	Color_changebar,
	/// deleted text color
	Color_deletedtext,
	/// added text color
	Color_addedtext,
	/// Top and bottom line color
	Color_topline,
	/// Table line color
	Color_tabularline,
	/// Table line color
	Color_tabularonoffline,
	/// Bottom area color
	Color_bottomarea,
	/// New page color
	Color_newpage,
	/// Page break color
	Color_pagebreak,

	// FIXME: why are the next four separate ??
	/// Color used for button frame
	Color_buttonframe,
	/// Color used for bottom background
	Color_buttonbg,
	/// Color used for buttom under focus
	Color_buttonhoverbg,

	// Logical attributes

	/// Color is inherited
	Color_inherit,
	/// For ignoring updates of a color
	Color_ignore
};


struct RGBColor {
	unsigned int r;
	unsigned int g;
	unsigned int b;
	RGBColor() : r(0), g(0), b(0) {}
	RGBColor(unsigned int red, unsigned int green, unsigned int blue)
		: r(red), g(green), b(blue) {}
};

inline bool operator==(RGBColor const & c1, RGBColor const & c2)
{
	return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}


inline bool operator!=(RGBColor const & c1, RGBColor const & c2)
{
	return !(c1 == c2);
}

} // namespace lyx

#endif
