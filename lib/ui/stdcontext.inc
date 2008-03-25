# -*- text -*-

# file stdcontext.inc
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author The LyX Team

# Full author contact details are available in file CREDITS.

# The interface is designed (partially) following the KDE Human Interface
# Guidelines (http://usability.kde.org/hig/)

Menuset

#
# InsetMath context menu
#
	Menu "context-math"
		Item "Equation Label|L" "label-insert"
		Item "Toggle Labeling/Numbering|T" "math-number-toggle"
	End


#
# InsetRef context menu
#
	Menu "context-ref"
		Item "Next Cross-Reference|N" "reference-next"
		Item "Go to Label|G" "label-goto"
		Separator
		Item "<reference>|r" "next-inset-modify changetype ref"
		Item "(<reference>)|e" "next-inset-modify changetype eqref"
		Item "<page>|p" "next-inset-modify changetype pageref"
		Item "on page <page>|o" "next-inset-modify changetype vpageref"
		Item "<reference> on page <page>|f" "next-inset-modify changetype vref"
		Item "Formatted reference|t" "next-inset-modify changetype prettyref"
		Separator
		Item "Properties...|P" "next-inset-toggle"
	End

	Menu "edit_reftype"
		
	End

#
# InsetLabel context menu
#
	Menu "context-label"
		Item "Next Cross-Reference|N" "reference-next"
		Item "Go back to Reference|G" "bookmark-goto 0"
		Separator
		Item "Properties...|P" "next-inset-toggle"
	End

#
# InsetCollapsable context menu
#
	Menu "context-collapsable"
		OptItem "Open Inset|O" "next-inset-toggle open"
		OptItem "Close Inset|C" "next-inset-toggle close"
		Separator
		OptItem "Properties...|P" "inset-settings"
	End

	Menu "context-conglomerate"
		Item "Toggle Label|L" "next-inset-toggle"
		Item "Dissolve Inset|D" "inset-dissolve charstyle"
		Separator
		OptItem "Properties...|P" "inset-settings"
	End

#
# InsetBox context menu
#

	Menu "context-box"
		Item "Frameless|l" "next-inset-modify changetype Frameless"
		Item "Simple frame|f" "next-inset-modify changetype Boxed"
		Item "Simple frame, page breaks|p" "next-inset-modify changetype Framed"
		Item "Oval, thin|O" "next-inset-modify changetype ovalbox"
		Item "Oval, thick|v" "next-inset-modify changetype Ovalbox"
		Item "Drop Shadow|w" "next-inset-modify changetype Shadowbox"
		Item "Shaded background|b" "next-inset-modify changetype Shaded"
		Item "Double frame|D" "next-inset-modify changetype Doublebox"
		Separator
		Item "Properties...|P" "inset-settings box"
	End

#
# InsetNote context menu
#

	Menu "context-note"
		Item "LyX Note|N" "next-inset-modify note Note Note"
		Item "Comment|C" "next-inset-modify note Note Comment"
		Item "Greyed Out|G" "next-inset-modify note Note Greyedout"
	End


#
# InsetSpace context menu
#
	Menu "context-space"
		Item "Interword Space|w" "next-inset-modify space \space{}"
		Item "Protected Space|o" "next-inset-modify space ~"
		Item "Thin Space|T" "next-inset-modify space \thinspace{}"
		Item "Quad Space|Q" "next-inset-modify space \quad{}"
		Item "QQuad Space|u" "next-inset-modify space \qquad{}"
		Item "Enspace|E" "next-inset-modify space \enspace{}"
		Item "Enskip|k" "next-inset-modify space \enskip{}"
		Item "Negative Thin Space|N" "next-inset-modify space \negthinspace{}"
		Item "Horizontal Fill|F" "next-inset-modify space \hfill{}"
		Item "Protected Horizontal Fill|i" "next-inset-modify space \hspace*{\fill}"
		Item "Horizontal Fill (Dots)|D" "next-inset-modify space \dotfill{}"
		Item "Horizontal Fill (Rule)|R" "next-inset-modify space \hrulefill{}"
		Item "Custom Length|C" "command-sequence next-inset-modify space \hspace{} \length 1in; next-inset-toggle"
		Separator
		Item "Properties...|P" "next-inset-toggle"
	End

#
# InsetVSpace context menu
#
	Menu "context-vspace"
		Item "DefSkip|D" "next-inset-modify vspace defskip"
		Item "SmallSkip|S" "next-inset-modify vspace smallskip"
		Item "MedSkip|M" "next-inset-modify vspace medskip"
		Item "BigSkip|B" "next-inset-modify vspace bigskip"
		Item "VFill|F" "next-inset-modify vspace vfill"
		Item "Custom|C" "command-sequence next-inset-modify vspace 1in; next-inset-toggle"
		Separator
		Item "Properties...|P" "next-inset-toggle"
	End

	Menu "edit_vspacetype"
		
	End

#
# InsetNewpage context menu
#
	Menu "context-newpage"
		Item "New Page|N" "next-inset-modify newpage newpage"
		Item "Page Break|a" "next-inset-modify newpage pagebreak"
		Item "Clear Page|C" "next-inset-modify newpage clearpage"
		Item "Clear Double Page|D" "next-inset-modify newpage cleardoublepage"
	End

#
# Edit context menu
#
	Menu "context-edit"
		Item "Cut" "cut"
		Item "Copy" "copy"
		Item "Paste" "paste"
		Submenu "Paste Recent|e" "edit_pasterecent"
		Separator
		Item "Jump to Saved Bookmark|B" "bookmark-goto 0"
		Separator
		Item "Move Paragraph Up|o" "paragraph-move-up"
		Item "Move Paragraph Down|v" "paragraph-move-down"
		Separator
		Item "Apply Last Text Style|A" "font-free-apply"
		Submenu "Text Style|S" "edit_textstyles"
		Item "Paragraph Settings...|P" "layout-paragraph"
		Separator
		Item "Fullscreen Mode" "ui-toggle fullscreen"
	End

#
# Math Macro context menu
#

	Menu "context-math-macro-definition"
		Item "Append Parameter" "math-macro-add-param"
		Item "Remove Last Parameter" "math-macro-remove-param"
                Separator
		Item "Make First Non-Optional into Optional Parameter" "math-macro-make-optional"
		Item "Make Last Optional into Non-Optional Parameter" "math-macro-make-nonoptional"
                Item "Insert Optional Parameter" "math-macro-add-optional-param"
                Item "Remove Optional Parameter" "math-macro-remove-optional-param"
		Separator
		Item "Append Parameter Eating From The Right" "math-macro-append-greedy-param"
                Item "Append Optional Parameter Eating From The Right" "math-macro-add-greedy-optional-param"
		Item "Remove Last Parameter Spitting Out To The Right" "math-macro-remove-greedy-param"
	End

#
# InsetListing context menu
#

	Menu "context-listings"
		Item "Cut" "cut"
		Item "Copy" "copy"
		Item "Paste" "paste"
		Submenu "Paste Recent|e" "edit_pasterecent"
		Separator
		Item "Properties...|P" "inset-settings listings"
	End

#
# InsetGraphics context menu
#

	Menu "context-graphics"
		Item "Properties...|P" "next-inset-toggle"
		Separator
		Item "Edit externally...|x" "graphics-edit"
	End

End