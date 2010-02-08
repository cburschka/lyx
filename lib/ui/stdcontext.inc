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
		OptItem "Number Whole Formula|N" "math-number-toggle"
		OptItem "Number This Line|u" "math-number-line-toggle"
		OptItem "Equation Label|L" "label-insert"
		OptItem "Copy as Reference|R" "copy-label-as-reference"
		Separator
		Item "Split Cell|C" "cell-split"
		Separator
		Submenu "Insert|s" "insert_math_context"
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
		Separator
		Submenu "Use Computer Algebra System|m" "edit_math_extern"
	End


#
# InsetRef context menu
#
	Menu "context-ref"
		Item "Next Cross-Reference|N" "reference-next"
		Item "Go to Label|G" "label-goto"
		Separator
		Item "<Reference>|R" "inset-modify changetype ref"
		Item "(<Reference>)|e" "inset-modify changetype eqref"
		Item "<Page>|P" "inset-modify changetype pageref"
		Item "On Page <Page>|O" "inset-modify changetype vpageref"
		Item "<Reference> on Page <Page>|f" "inset-modify changetype vref"
		Item "Formatted Reference|t" "inset-modify changetype prettyref"
		Separator
		Item "Settings...|S" "inset-settings"
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
		Item "Settings...|S" "inset-settings"
	End


#
# InsetCitation context menu
#
	Menu "context-citation"
		CiteStyles
		Separator
		Item "Settings...|S" "inset-settings"
	End


#
# InsetBibtex context menu
#
	Menu "context-bibtex"
		Item "Settings...|S" "inset-settings"
		Separator
		Item "Edit Database(s) Externally...|x" "inset-edit"
	End


#
# InsetCollapsable context menu
#
	Menu "context-collapsable"
		OptItem "Open Inset|O" "inset-toggle open"
		OptItem "Close Inset|C" "inset-toggle close"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
		OptItem "Settings...|S" "inset-settings"
	End

	Menu "context-conglomerate"
		Item "Show Label|L" "inset-toggle"
		Item "Dissolve Inset|D" "inset-dissolve charstyle"
		Separator
		OptItem "Settings...|S" "inset-settings"
	End

#
# InsetBox context menu
#

	Menu "context-box"
		Item "Frameless|l" "inset-modify changetype Frameless"
		Item "Simple Frame|F" "inset-modify changetype Boxed"
		Item "Simple Frame, Page Breaks|P" "inset-modify changetype Framed"
		Item "Oval, Thin|a" "inset-modify changetype ovalbox"
		Item "Oval, Thick|v" "inset-modify changetype Ovalbox"
		Item "Drop Shadow|w" "inset-modify changetype Shadowbox"
		Item "Shaded Background|B" "inset-modify changetype Shaded"
		Item "Double Frame|u" "inset-modify changetype Doublebox"
		Separator
		OptItem "Open Inset|O" "inset-toggle open"
		OptItem "Close Inset|C" "inset-toggle close"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
		Item "Settings...|S" "inset-settings box"
	End

#
# InsetNote context menu
#

	Menu "context-note"
		Item "LyX Note|N" "inset-modify note Note Note"
		Item "Comment|m" "inset-modify note Note Comment"
		Item "Greyed Out|G" "inset-modify note Note Greyedout"
		Separator
		OptItem "Open Inset|O" "inset-toggle open"
		OptItem "Close Inset|C" "inset-toggle close"
		Separator
		Item "Open All Notes|A" "all-insets-toggle open note"
		Item "Close All Notes|l" "all-insets-toggle close note"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
	End

#
# InsetPhantom context menu
#

	Menu "context-phantom"
		Item "Phantom" "inset-modify phantom Phantom Phantom"
		Item "Horiz. Phantom" "inset-modify phantom Phantom HPhantom"
		Item "Vert. Phantom" "inset-modify phantom Phantom VPhantom"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
	End

#
# InsetSpace context menu
#
	Menu "context-space"
		Item "Interword Space|w" "inset-modify space \space{}"
		Item "Protected Space|o" "inset-modify space ~"
		Item "Thin Space|T" "inset-modify space \thinspace{}"
		Item "Negative Thin Space|N" "inset-modify space \negthinspace{}"
		Item "Half Quad Space (Enskip)|k" "inset-modify space \enskip{}"
		Item "Protected Half Quad Space (Enspace)|E" "inset-modify space \enspace{}"
		Item "Quad Space|Q" "inset-modify space \quad{}"
		Item "Double Quad Space|u" "inset-modify space \qquad{}"
		Item "Horizontal Fill|F" "inset-modify space \hfill{}"
		Item "Protected Horizontal Fill|i" "inset-modify space \hspace*{\fill}"
		Item "Horizontal Fill (Dots)|D" "inset-modify space \dotfill{}"
		Item "Horizontal Fill (Rule)|R" "inset-modify space \hrulefill{}"
		Item "Horizontal Fill (Left Arrow)|L" "inset-modify space \leftarrowfill{}"
		Item "Horizontal Fill (Right Arrow)|g" "inset-modify space \rightarrowfill{}"
		Item "Horizontal Fill (Up Brace)|p" "inset-modify space \upbracefill{}"
		Item "Horizontal Fill (Down Brace)|B" "inset-modify space \downbracefill{}"
		Item "Custom Length|C" "command-sequence inset-modify space \hspace{} \length 1in; inset-settings"
		Separator
		Item "Settings...|S" "inset-settings"
	End

#
# InsetMathSpace context menu
#
	Menu "context-mathspace"
		Item "Thin Space|T" "inset-modify mathspace \thinspace{}"
		Item "Medium Space|M" "inset-modify mathspace \medspace{}"
		Item "Thick Space|h" "inset-modify mathspace \thickspace{}"
		Item "Negative Thin Space|N" "inset-modify mathspace \negthinspace{}"
		Item "Negative Medium Space|u" "inset-modify mathspace \negmedspace{}"
		Item "Negative Thick Space|i" "inset-modify mathspace \negthickspace{}"
		Item "Half Quad Space (Enskip)|k" "inset-modify mathspace \enskip{}"
		Item "Quad Space|Q" "inset-modify mathspace \quad{}"
		Item "Double Quad Space|u" "inset-modify mathspace \qquad{}"
		Item "Custom Length|C" "command-sequence inset-modify mathspace \hspace{} \length 1in; inset-settings"
		Separator
		Item "Settings...|S" "inset-settings"
	End

#
# InsetVSpace context menu
#
	Menu "context-vspace"
		Item "DefSkip|D" "inset-modify vspace defskip"
		Item "SmallSkip|S" "inset-modify vspace smallskip"
		Item "MedSkip|M" "inset-modify vspace medskip"
		Item "BigSkip|B" "inset-modify vspace bigskip"
		Item "VFill|F" "inset-modify vspace vfill"
		Item "Custom|C" "command-sequence inset-modify vspace 1in; inset-settings"
		Separator
		Item "Settings...|e" "inset-settings"
	End

#
# InsetInclude context menu
#
	Menu "context-include"
		Item "Include|c" "inset-modify changetype include"
		Item "Input|p" "inset-modify changetype input"
		Item "Verbatim|V" "inset-modify changetype verbatiminput"
		Item "Verbatim (marked blanks)|b" "inset-modify changetype verbatiminput*"
		Item "Listing|L" "inset-modify changetype lstinputlisting"
		Separator
		Item "Settings...|S" "inset-settings"
		Separator
		Item "Edit Included File...|E" "inset-edit"
	End

#
# InsetNewpage context menu
#
	Menu "context-newpage"
		Item "New Page|N" "inset-modify newpage newpage"
		Item "Page Break|a" "inset-modify newpage pagebreak"
		Item "Clear Page|C" "inset-modify newpage clearpage"
		Item "Clear Double Page|D" "inset-modify newpage cleardoublepage"
	End

#
# InsetNewline context menu
#
	Menu "context-newline"
		Item "Ragged Line Break|R" "inset-modify newline newline"
		Item "Justified Line Break|J" "inset-modify newline linebreak"
	End

#
# Edit context menu
#
	Menu "context-edit"
		spellingsuggestions
		Separator
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
		OptItem "Accept Change|c" "change-accept"
		OptItem "Reject Change|j" "change-reject"
		Separator
		Item "Apply Last Text Style|A" "textstyle-apply"
		Submenu "Text Style|S" "edit_textstyles"
		Item "Paragraph Settings...|P" "layout-paragraph"
		LanguageSelector
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
		OptItem "Open Inset|O" "inset-toggle open"
		OptItem "Close Inset|C" "inset-toggle close"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
		Item "Settings...|S" "inset-settings listings"
	End

#
# InsetGraphics context menu
#

	Menu "context-graphics"
		Item "Settings...|S" "inset-settings"
		Item "Reload|R" "graphics-reload"
		Separator
		Item "Edit Externally...|x" "inset-edit"
		Separator
		GraphicsGroups
	End

#
# InsetExternal context menu
#

	Menu "context-external"
		Item "Settings...|S" "inset-settings"
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

#
# InsetBranch context menu
#

	Menu "context-branch"
		OptItem "Open Inset|O" "inset-toggle open"
		OptItem "Close Inset|C" "inset-toggle close"
		Separator
		OptItem "Activate Branch|A" "branch-activate"
		OptItem "Deactivate Branch|e" "branch-deactivate"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
		OptItem "Settings...|S" "inset-settings"
	End

#
# Toc Labels and References context menu
#

	Menu "context-toc-label"
		OptItem "Copy as Reference|C" "copy-label-as-reference"
		OptItem "Insert Reference at Cursor Position|I" "label-insert-as-reference"
		Separator
		Item "Settings...|S" "inset-settings"
	End

#
# Toc Branches context menu
#

	Menu "context-toc-branch"
		OptItem "Activate Branch|A" "branch-activate"
		OptItem "Deactivate Branch|e" "branch-deactivate"
		Separator
		Item "Settings...|S" "inset-settings"
	End

#
# Toc Graphics context menu
#

	Menu "context-toc-graphics"
		Item "Settings...|S" "inset-settings"
		Separator
		Item "Edit Externally...|x" "inset-edit"
	End

#
# Toc Citation context menu
#

	Menu "context-toc-citation"
		Item "Settings...|S" "inset-settings"
	End

#
# Toc Figures context menu
#

	Menu "context-toc-figure"
		Item "Settings...|S" "inset-settings"
	End

#
# Toc Listings context menu
#

	Menu "context-toc-listing"
		Item "Settings...|S" "inset-settings"
	End

#
# Toc Tables context menu
#

	Menu "context-toc-table"
		Item "Settings...|S" "inset-settings"
	End

#
# Toc Childs context menu
#
	Menu "context-toc-child"
		Item "Include|c" "inset-modify changetype include"
		Item "Input|p" "inset-modify changetype input"
		Item "Verbatim|V" "inset-modify changetype verbatiminput"
		Item "Verbatim (marked blanks)|b" "inset-modify changetype verbatiminput*"
		Item "Listing|L" "inset-modify changetype lstinputlisting"
		Separator
		Item "Settings...|S" "inset-settings"
		Separator
		Item "Edit Included File...|E" "inset-edit"
	End

#
# Toc Indices context menu
#

	Menu "context-toc-index"
		OptItem "Settings...|S" "inset-settings"
	End

#
# Index context menu
#

	Menu "context-index"
		IndicesContext
		Separator
		OptItem "Open Inset|O" "inset-toggle open"
		OptItem "Close Inset|C" "inset-toggle close"
		Separator
		Item "Dissolve Inset|D" "inset-dissolve"
		Separator
		OptItem "Settings...|S" "inset-settings"
	End

#
# Index Lists context menu
#

	Menu "context-indexprint"
		Item "All Indexes|A" "inset-modify check-printindex*"
		IndicesListsContext
		Separator
		Item "Subindex|b" "inset-modify toggle-subindex"
		Separator
		OptItem "Settings...|S" "inset-settings"
	End

#
# Nomencl List context menu
#

	Menu "context-nomenclprint"
		OptItem "Settings...|S" "inset-settings"
	End


#
# Toc Changes context menu
#

	Menu "context-toc-change"
		Item "Accept Change|c" "change-accept"
		Item "Reject Change|R" "change-reject"
	End	
	
#
# Toc Table of Context context menu
#

	Menu "context-toc-tableofcontents"
		Item "Promote Section|P" "outline-out"
		Item "Demote Section|D" "outline-in"
		Item "Move Section Up|U" "outline-up"
		Item "Move Section Down|w" "outline-down"
		Separator
		Item "Select Section|S" "section-select"
	End	

End
