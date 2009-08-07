# -*- text -*-

# file stdcontext.inc
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author The LyX Team

# Full author contact details are available in file CREDITS.

# The interface is designed (partially) following the KDE Human Interface
# Guidelines (http://usability.kde.org/hig/)

# Casing Rules:
# Capitalize all words in the element, with the following exceptions: 
# * Articles: a, an, the. 
# * Conjunctions: and, but, for, not, so, yet ...  
# * Prepositions of three or fewer letters: at, for, by, in, to ...
#   (except when the preposition is part of a verb phrase, such as "Check In")
# (http://library.gnome.org/devel/hig-book/stable/design-text-labels.html.en)

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
		Item "Delimiters...|r" "dialog-show mathdelimiter"
		Item "Matrix...|x" "dialog-show mathmatrix"
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
		Submenu "AMS Environment|A" "ams_environment"
		Separator
		OptItem "Equation Label|L" "label-insert"
		OptItem "Copy as Reference|R" "copy-label-as-reference"
		OptItem "Number Whole Formula|N" "math-number-toggle"
		OptItem "Number This Line|u" "math-number-line-toggle"
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
		Item "Show Math Toolbar" "toolbar-toggle math toggle"
		Item "Show Math-Panels Toolbar" "toolbar-toggle math_panels toggle"
		Item "Show Table Toolbar" "toolbar-toggle table toggle"
	End


#
# InsetRef context menu
#
	Menu "context-ref"
		Item "Next Cross-Reference|N" "reference-next"
		Item "Go to Label|G" "label-goto"
		Separator
		Item "<Reference>|R" "next-inset-modify changetype ref"
		Item "(<Reference>)|e" "next-inset-modify changetype eqref"
		Item "<Page>|P" "next-inset-modify changetype pageref"
		Item "On Page <Page>|O" "next-inset-modify changetype vpageref"
		Item "<Reference> on Page <Page>|f" "next-inset-modify changetype vref"
		Item "Formatted Reference|t" "next-inset-modify changetype prettyref"
		Separator
		Item "Settings...|S" "next-inset-toggle"
	End

#
# InsetLabel context menu
#
	Menu "context-label"
		Item "Next Cross-Reference|N" "reference-next"
		Item "Go Back|G" "bookmark-goto 0"
		Separator
		Item "Copy as Reference|C" "copy-label-as-reference"
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
		Item "Edit Database(s) Externally...|x" "inset-edit"
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
		Item "Show Label|L" "next-inset-toggle"
		Item "Dissolve Inset|D" "inset-dissolve charstyle"
		Separator
		OptItem "Settings...|S" "inset-settings"
	End

#
# InsetBox context menu
#

	Menu "context-box"
		Item "Frameless|l" "next-inset-modify changetype Frameless"
		Item "Simple Frame|F" "next-inset-modify changetype Boxed"
		Item "Simple Frame, Page Breaks|P" "next-inset-modify changetype Framed"
		Item "Oval, Thin|a" "next-inset-modify changetype ovalbox"
		Item "Oval, Thick|v" "next-inset-modify changetype Ovalbox"
		Item "Drop Shadow|w" "next-inset-modify changetype Shadowbox"
		Item "Shaded Background|B" "next-inset-modify changetype Shaded"
		Item "Double Frame|u" "next-inset-modify changetype Doublebox"
		Separator
		OptItem "Open Inset|O" "next-inset-toggle open"
		OptItem "Close Inset|C" "next-inset-toggle close"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
		Item "Settings...|S" "inset-settings box"
	End

#
# InsetNote context menu
#

	Menu "context-note"
		Item "LyX Note|N" "next-inset-modify note Note Note"
		Item "Comment|m" "next-inset-modify note Note Comment"
		Item "Greyed Out|G" "next-inset-modify note Note Greyedout"
		Separator
		OptItem "Open Inset|O" "next-inset-toggle open"
		OptItem "Close Inset|C" "next-inset-toggle close"
		Separator
		Item "Open All Notes|A" "all-insets-toggle open note"
		Item "Close All Notes|l" "all-insets-toggle close note"
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
# InsetMathSpace context menu
#
	Menu "context-mathspace"
		Item "Thin Space|T" "next-inset-modify mathspace \thinspace{}"
		Item "Medium Space|M" "next-inset-modify mathspace \medspace{}"
		Item "Thick Space|h" "next-inset-modify mathspace \thickspace{}"
		Item "Negative Thin Space|N" "next-inset-modify mathspace \negthinspace{}"
		Item "Negative Medium Space|u" "next-inset-modify mathspace \negmedspace{}"
		Item "Negative Thick Space|i" "next-inset-modify mathspace \negthickspace{}"
		Item "Half Quad Space (Enskip)|k" "next-inset-modify mathspace \enskip{}"
		Item "Quad Space|Q" "next-inset-modify mathspace \quad{}"
		Item "Double Quad Space|u" "next-inset-modify mathspace \qquad{}"
		Item "Custom Length|C" "command-sequence next-inset-modify mathspace \hspace{} \length 1in; next-inset-toggle"
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
		Item "Edit Included File...|E" "inset-edit"
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
		Item "Jump Back to Saved Bookmark|B" "bookmark-goto 0"
		Separator
		Item "Move Paragraph Up|o" "paragraph-move-up"
		Item "Move Paragraph Down|v" "paragraph-move-down"
		Separator
		OptItem "Promote Section|r" "outline-out"
		OptItem "Demote Section|m" "outline-in"
		OptItem "Move Section Down|D" "outline-down"
		OptItem "Move Section Up|U" "outline-up"
		OptItem "Insert Short Title|T" "optional-insert"
		Separator
		Item "Apply Last Text Style|A" "textstyle-apply"
		Submenu "Text Style|S" "edit_textstyles"
		Item "Paragraph Settings...|P" "layout-paragraph"
		Separator
		Item "Fullscreen Mode" "ui-toggle fullscreen"
	End

#
# Math Macro context menu
#

	Menu "context-math-macro-definition"
		Item "Append Argument" "math-macro-add-param"
		Item "Remove Last Argument" "math-macro-remove-param"
		Separator
		Item "Make First Non-Optional Into Optional Argument" "math-macro-make-optional"
		Item "Make Last Optional Into Non-Optional Argument" "math-macro-make-nonoptional"
		Item "Insert Optional Argument" "math-macro-add-optional-param"
		Item "Remove Optional Argument" "math-macro-remove-optional-param"
		Separator
		Item "Append Argument Eating From the Right" "math-macro-append-greedy-param"
		Item "Append Optional Argument Eating From the Right" "math-macro-add-greedy-optional-param"
		Item "Remove Last Argument Spitting Out to the Right" "math-macro-remove-greedy-param"
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
		OptItem "Open Inset|O" "next-inset-toggle open"
		OptItem "Close Inset|C" "next-inset-toggle close"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
		Item "Settings...|S" "inset-settings listings"
	End

#
# InsetGraphics context menu
#

	Menu "context-graphics"
		Item "Settings...|S" "next-inset-toggle"
		Separator
		Item "Edit Externally...|x" "inset-edit"
		Separator
		GraphicsGroups
	End

#
# InsetExternal context menu
#

	Menu "context-external"
		Item "Settings...|S" "next-inset-toggle"
		Separator
		Item "Edit Externally...|x" "inset-edit"
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
		Item "Settings...|S" "inset-settings info"
	End

End
