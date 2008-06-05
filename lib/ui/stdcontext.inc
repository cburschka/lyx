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
	Menu "insert_math_context"
		Item "Array Environment|y" "math-matrix 2 2"
		Item "Cases Environment|C" "command-sequence math-mode on; math-insert \cases"
		Item "Aligned Environment|l" "command-sequence math-mode on; math-insert \aligned; tabular-feature append-column"
		Item "AlignedAt Environment|v" "command-sequence math-mode on; math-insert \alignedat; tabular-feature append-column"
		Item "Gathered Environment|h" "command-sequence math-mode on; math-insert \gathered"
		Item "Split Environment|S" "command-sequence math-mode on; math-insert \split; tabular-feature append-column"
		Separator
		Item "Delimiters|r" "dialog-show mathdelimiter"
		Item "Matrix|x" "dialog-show mathmatrix"
		Item "Macro|o" "math-macro newmacroname newcommand"
	End

	Menu "ams_environment"
		Item "AMS align Environment|a" "math-mutate align"
		Item "AMS alignat Environment|t" "math-mutate alignat"
		Item "AMS flalign Environment|f" "math-mutate flalign"
		Item "AMS gather Environment|g" "math-mutate gather"
		Item "AMS multline Environment|m" "math-mutate multline"
	End
	
	Menu "context-math"
		Item "Inline Formula|I" "math-mutate simple"
		Item "Displayed Formula|D" "math-mutate equation"
		Item "Eqnarray Environment|E" "math-mutate eqnarray"
		SubMenu "AMS Environment|A" "ams_environment"
		Separator		
		OptItem "Equation Label|L" "label-insert"
		OptItem "Toggle Labeling/Numbering|T" "math-number-toggle"
		Separator
		Item "Split Cell|C" "cell-split"
		Separator
		Submenu "Insert|n" "insert_math_context"
		Separator
		OptItem "Add Line Above|o" "tabular-feature add-hline-above"
		OptItem "Add Line Below|B" "tabular-feature add-hline-below"
		OptItem "Delete Line Above|D" "tabular-feature delete-hline-above"
		OptItem "Delete Line Below|e" "tabular-feature delete-hline-below"
		Separator
		OptItem "Add Line to Left" "tabular-feature add-vline-left"
		OptItem "Add Line to Right" "tabular-feature add-vline-right"
		OptItem "Delete Line to Left" "tabular-feature delete-vline-left"
		OptItem "Delete Line to Right" "tabular-feature delete-vline-right"
		Separator
		Item "Toggle Math Toolbar" "toolbar-toggle math toggle"
		Item "Toggle Math-Panels Toolbar" "toolbar-toggle math_panels toggle"
		Item "Toggle Table Toolbar" "toolbar-toggle table toggle"
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
		Item "Settings...|S" "next-inset-toggle"
	End

#
# InsetLabel context menu
#
	Menu "context-label"
		Item "Next Cross-Reference|N" "reference-next"
		Item "Go back to Reference|G" "bookmark-goto 0"
		Separator
		Item "Settings...|S" "next-inset-toggle"
	End


#
# InsetCitation context menu
#
	Menu "context-citation"
		CiteStyles
		Separator
		Item "Settings...|S" "next-inset-toggle"
	End


#
# InsetBibtex context menu
#
	Menu "context-bibtex"
		Item "Settings...|S" "next-inset-toggle"
		Separator
		Item "Edit Database(s) externally...|x" "inset-edit"
	End


#
# InsetCollapsable context menu
#
	Menu "context-collapsable"
		OptItem "Open Inset|O" "next-inset-toggle open"
		OptItem "Close Inset|C" "next-inset-toggle close"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
		OptItem "Settings...|S" "inset-settings"
	End

	Menu "context-conglomerate"
		Item "Toggle Label|L" "next-inset-toggle"
		Item "Dissolve Inset|D" "inset-dissolve charstyle"
		Separator
		OptItem "Settings...|S" "inset-settings"
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
		Item "Dissolve Inset|D" "inset-dissolve"
		Item "Settings...|S" "inset-settings box"
	End

#
# InsetNote context menu
#

	Menu "context-note"
		Item "LyX Note|N" "next-inset-modify note Note Note"
		Item "Comment|C" "next-inset-modify note Note Comment"
		Item "Greyed Out|G" "next-inset-modify note Note Greyedout"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
	End


#
# InsetSpace context menu
#
	Menu "context-space"
		Item "Interword Space|w" "next-inset-modify space \space{}"
		Item "Protected Space|o" "next-inset-modify space ~"
		Item "Thin Space|T" "next-inset-modify space \thinspace{}"
		Item "Negative Thin Space|N" "next-inset-modify space \negthinspace{}"
		Item "Half Quad Space (Enskip)|k" "next-inset-modify space \enskip{}"
		Item "Protected Half Quad Space (Enspace)|E" "next-inset-modify space \enspace{}"
		Item "Quad Space|Q" "next-inset-modify space \quad{}"
		Item "Double Quad Space|u" "next-inset-modify space \qquad{}"
		Item "Horizontal Fill|F" "next-inset-modify space \hfill{}"
		Item "Protected Horizontal Fill|i" "next-inset-modify space \hspace*{\fill}"
		Item "Horizontal Fill (Dots)|D" "next-inset-modify space \dotfill{}"
		Item "Horizontal Fill (Rule)|R" "next-inset-modify space \hrulefill{}"
		Item "Horizontal Fill (Left Arrow)|L" "next-inset-modify space \leftarrowfill{}"
		Item "Horizontal Fill (Right Arrow)|g" "next-inset-modify space \rightarrowfill{}"
		Item "Horizontal Fill (Up Brace)|p" "next-inset-modify space \upbracefill{}"
		Item "Horizontal Fill (Down Brace)|B" "next-inset-modify space \downbracefill{}"
		Item "Custom Length|C" "command-sequence next-inset-modify space \hspace{} \length 1in; next-inset-toggle"
		Separator
		Item "Settings...|S" "next-inset-toggle"
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
		Item "Settings...|e" "next-inset-toggle"
	End

#
# InsetInclude context menu
#
	Menu "context-include"
		Item "Include|c" "next-inset-modify changetype include"
		Item "Input|p" "next-inset-modify changetype input"
		Item "Verbatim|V" "next-inset-modify changetype verbatiminput"
		Item "Verbatim (marked blanks)|b" "next-inset-modify changetype verbatiminput*"
		Item "Listing|L" "next-inset-modify changetype lstinputlisting"
		Separator
		Item "Settings...|S" "next-inset-toggle"
		Separator
		Item "Edit included file...|E" "inset-edit"
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
# InsetNewline context menu
#
	Menu "context-newline"
		Item "Ragged Line Break|R" "next-inset-modify newline newline"
		Item "Justified Line Break|J" "next-inset-modify newline linebreak"
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
		OptItem "Promote Section|r" "outline-out"
		OptItem "Demote Section|m" "outline-in"
		OptItem "Move Section down|d" "outline-down"
		OptItem "Move Section up|u" "outline-up"
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
		Item "Settings...|S" "inset-settings listings"
	End

#
# InsetGraphics context menu
#

	Menu "context-graphics"
		Item "Settings...|S" "next-inset-toggle"
		Separator
		Item "Edit externally...|x" "inset-edit"
		Separator
		GraphicsGroups
	End

#
# InsetExternal context menu
#

	Menu "context-external"
		Item "Settings...|S" "next-inset-toggle"
		Separator
		Item "Edit externally...|x" "inset-edit"
	End

#
# InsetTabular context menu
#

	Menu "context-tabular"
		Item "Multicolumn|M" "tabular-feature multicolumn"
		Separator
		Item "Top Line|T" "tabular-feature toggle-line-top"
		Item "Bottom Line|B" "tabular-feature toggle-line-bottom"
		Item "Left Line|L" "tabular-feature toggle-line-left"
		Item "Right Line|R" "tabular-feature toggle-line-right"
		Separator
		Item "Left|L" "tabular-feature align-left"
		Item "Center|C" "tabular-feature align-center"
		Item "Right|R" "tabular-feature align-right"
		Separator
		Item "Top|T" "tabular-feature valign-top"
		Item "Middle|M" "tabular-feature valign-middle"
		Item "Bottom|B" "tabular-feature valign-bottom"
		Separator
		Item "Add Row|A" "tabular-feature append-row"
		Item "Delete Row|D" "tabular-feature delete-row"
		Item "Copy Row|o" "tabular-feature copy-row"
		# Item "Swap Rows|S" "tabular-feature swap-row" # currently broken
		Separator
		Item "Add Column|u" "tabular-feature append-column"
		Item "Delete Column|e" "tabular-feature delete-column"
		Item "Copy Column|p" "tabular-feature copy-column"
		# Item "Swap Columns|w" "tabular-feature swap-column" # currently broken
		Separator
		Item "Settings...|S" "inset-settings tabular"
	End


#
# InsetInfo context menu
#

	Menu "context-info"
		Item "Settings...|S" "inset-settings"
	End

End
