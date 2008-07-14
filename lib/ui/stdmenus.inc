# -*- text -*-

# file stdmenus.inc
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Jean-Marc Lasgouttes
# author John Levon
# author Michael Gerz

# Full author contact details are available in file CREDITS.

# The interface is designed (partially) following the KDE Human Interface
# Guidelines (http://usability.kde.org/hig/)

Menuset

	Menubar
# Make the first 4 menus be as much as expected as possible
		Submenu "File|F" "file"
		Submenu "Edit|E" "edit"
		Submenu "View|V" "view"
		Submenu "Insert|I" "insert"
		Submenu "Navigate|N" "navigate"
		Submenu "Document|D" "document"
		Submenu "Tools|T" "tools"
		Submenu "Help|H" "help"
	End

#
# FILE MENU
#

	Menu "file"
		Item "New|N" "buffer-new"
		Item "New from Template...|m" "buffer-new-template"
		Item "Open...|O" "file-open"
		Submenu "Open Recent|t" "file_lastfiles"
		Separator
		Item "Close|C" "buffer-close"
		Item "Save|S" "buffer-write"
		Item "Save As...|A" "buffer-write-as"
		Item "Save All|l" "buffer-write-all"
		Item "Revert to Saved|R" "buffer-reload"
		Submenu "Version Control|V" "file_vc"
		Separator
		Submenu "Import|I" "file_import"
		Submenu "Export|E" "file_export"
		Item "Print...|P" "dialog-show print"
		OptItem "Fax...|F" "buffer-export fax"
		Separator
		Item "New Window|W" "window-new"
		Item "Close Window|d" "window-close"
		Separator
		Item "Exit|x" "lyx-quit"
	End

	Menu "file_lastfiles"
		Lastfiles
	End

	Menu "file_vc"
		OptItem "Register...|R" "vc-register"
		OptItem "Check In Changes...|I" "vc-check-in"
		OptItem "Check Out for Edit|O" "vc-check-out"
		OptItem "Revert to Last Version|L" "vc-revert"
		OptItem "Undo Last Check In|U" "vc-undo-last"
		OptItem "Show History|H" "dialog-show vclog"
	End

	Menu "file_import"
		ImportFormats
	End

	Menu "file_export"
		ExportFormats
		Item "Custom...|C" "buffer-export custom"
	End

#
# EDIT MENU
#

	Menu "edit"
		Item "Undo|U" "undo"
		Item "Redo|R" "redo"
		Separator
		Item "Cut" "cut"
		Item "Copy" "copy"
		Item "Paste" "paste"
		Submenu "Paste Recent|e" "edit_pasterecent"
		Submenu "Paste Special" "edit_paste"
		Separator
		Item "Select All" "command-sequence buffer-begin ; buffer-end-select"
		Separator
		Item "Find & Replace...|F" "dialog-show findreplace"
		Separator
		Item "Move Paragraph Up|o" "paragraph-move-up"
		Item "Move Paragraph Down|v" "paragraph-move-down"
		Separator
		Submenu "Text Style|S" "edit_textstyles"
		Item "Paragraph Settings...|P" "layout-paragraph"
		Separator
# Mathed b0rkage means these don't work properly
		OptSubmenu "Table|T" "edit_tabular"
		OptSubmenu "Math|M" "edit_math"
		OptSubmenu "Rows & Columns|C" "edit_tabular_features"
		Separator
# This is where the context-specific stuff is supposed to go.
# The idea is that everything on here can disappear and should
# only appear when relevant (getStatus). Only stuff that needs
# a right-click to bring up the dialog should appear here (it is not
# obvious what the context is for the others)
		OptItem "Increase List Depth|I" "depth-increment"
		OptItem "Decrease List Depth|D" "depth-decrement"
		OptItem "Dissolve Inset|l" "inset-dissolve"
		OptItem "TeX Code Settings...|C" "inset-settings ert"
# 'a' shortcut to match Insert entry, shouldn't clash with Table Settings
		OptItem "Float Settings...|a" "inset-settings float"
		OptItem "Text Wrap Settings...|W" "inset-settings wrap"
		OptItem "Note Settings...|N" "inset-settings note"
		OptItem "Branch Settings...|B" "inset-settings branch"
		OptItem "Box Settings...|x" "inset-settings box"
# Hey, guess what's broken ? Surprise surprise, it's tabular stuff
# This is in the Table submenu instead for now.
#	OptItem "Table Settings...|a" "inset-settings tabular"
		OptItem "Table Settings...|a" "layout-tabular"
	End

	Menu "edit_paste"
		Item "Plain Text|T" "clipboard-paste"
		Item "Plain Text, Join Lines|J" "clipboard-paste paragraph"
		Separator
		Item "Selection|S" "primary-selection-paste"
		Item "Selection, Join Lines|i" "primary-selection-paste paragraph"
		Separator
		Item "Paste As LinkBack PDF" "paste linkback"
		Item "Paste As PDF" "paste pdf"
		Item "Paste As PNG" "paste png"
		Item "Paste As JPEG" "paste jpeg"
	End

	Menu "edit_pasterecent"
		PasteRecent
	End

	Menu "edit_textstyles"
		Item "Dissolve CharStyle" "inset-dissolve charstyle"
		CharStyles
		Elements
		Separator
		Item "Customized...|C" "dialog-show character"
		Separator
		Item "Capitalize|a" "word-capitalize"
		Item "Uppercase|U" "word-upcase"
		Item "Lowercase|L" "word-lowcase"
	End

# not much we can do to help here
	Menu "edit_tabular"
		Item "Multicolumn|M" "tabular-feature multicolumn"
		Separator
		Item "Top Line|T" "tabular-feature toggle-line-top"
		Item "Bottom Line|B" "tabular-feature toggle-line-bottom"
		Item "Left Line|L" "tabular-feature toggle-line-left"
		Item "Right Line|R" "tabular-feature toggle-line-right"
		Separator
	End

	Menu "edit_tabular_features"
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
	End

	Menu "edit_math"
		Item "Number whole Formula|N" "math-number-toggle"
		Item "Number this Line|u" "math-number-line-toggle"
		Submenu "Change Limits Type|L" "edit_math_limits"
		Submenu "Macro Definition" "edit_math_macro_definition"
		Separator
		Submenu "Change Formula Type|F" "edit_math_mutate"
		Separator
		Submenu "Text Style|T" "edit_math_textstyles"
		Separator
		Submenu "Use Computer Algebra System|S" "edit_math_extern"
		Separator
		Item "Split Cell|C" "cell-split"
		Separator
		Item "Add Line Above|A" "tabular-feature add-hline-above"
		Item "Add Line Below|B" "tabular-feature add-hline-below"
		Item "Delete Line Above|D" "tabular-feature delete-hline-above"
		Item "Delete Line Below|e" "tabular-feature delete-hline-below"
		Separator
		Item "Add Line to Left" "tabular-feature add-vline-left"
		Item "Add Line to Right" "tabular-feature add-vline-right"
		Item "Delete Line to Left" "tabular-feature delete-vline-left"
		Item "Delete Line to Right" "tabular-feature delete-vline-right"
	End

	Menu "edit_math_macro_definition"
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

	Menu "edit_math_limits"
		 Item "Default|t"  "math-limits empty"
		 Item "Display|D"  "math-limits limits"
		 Item "Inline|I"  "math-limits nolimits"
	End

	Menu "edit_math_textstyles"
		Item "Math Normal Font|N" "math-insert \mathnormal"
		Separator
		Item "Math Calligraphic Family|C" "math-insert \mathcal"
		Item "Math Fraktur Family|F" "math-insert \mathfrak"
		Item "Math Roman Family|R" "math-insert \mathrm"
		Item "Math Sans Serif Family|S" "math-insert \mathsf"
		Separator
		Item "Math Bold Series|B" "math-insert \mathbf"
		Separator
		Item "Text Normal Font|T" "math-insert \textnormal"
		Separator
		Item "Text Roman Family" "math-insert \textrm"
		Item "Text Sans Serif Family" "math-insert \textsf"
		Item "Text Typewriter Family" "math-insert \texttt"
		Separator
		Item "Text Bold Series" "math-insert \textbf"
		Item "Text Medium Series" "math-insert \textmd"
		Separator
		Item "Text Italic Shape" "math-insert \textit"
		Item "Text Small Caps Shape" "math-insert \textsc"
		Item "Text Slanted Shape" "math-insert \textsl"
		Item "Text Upright Shape" "math-insert \textup"
	End

	Menu "edit_math_extern"
		Item "Octave|O" "math-extern octave"
		Item "Maxima|M" "math-extern maxima"
		Item "Mathematica|a" "math-extern mathematica"
		Separator
		Item "Maple, simplify|s" "math-extern maple simplify"
		Item "Maple, factor|f" "math-extern maple factor"
		Item "Maple, evalm|e" "math-extern maple evalm"
		Item "Maple, evalf|v" "math-extern maple evalf"
	End

	Menu "edit_math_mutate"
		Item "Inline Formula|I" "math-mutate simple"
		Item "Displayed Formula|D" "math-mutate equation"
		Item "Eqnarray Environment|E" "math-mutate eqnarray"
		Item "AMS align Environment|a" "math-mutate align"
		Item "AMS alignat Environment|t" "math-mutate alignat"
		Item "AMS flalign Environment|f" "math-mutate flalign"
		Item "AMS gather Environment|g" "math-mutate gather"
		Item "AMS multline Environment|m" "math-mutate multline"
	End

#
# VIEW MENU
#

	Menu "view"
		Item "Open All Insets|O" "all-insets-toggle open"
		Item "Close All Insets|C" "all-insets-toggle close"
		Separator
		Item "Unfold Math Macro" "math-macro-unfold"
		Item "Fold Math Macro" "math-macro-fold"
		Separator
		Item "View Source|S" "dialog-toggle view-source"
		Submenu "Update|U" "view_update"
		ViewFormats
		Separator
		Item "Split View Into Left And Right Half|i" "split-view horizontal"
		Item "Split View Into Upper And Lower Half|V" "split-view vertical"
		Item "Close Tab Group|G" "close-tab-group"
		Item "Fullscreen|l" "ui-toggle fullscreen"
		Submenu "Toolbars|b" "toolbars"
		Separator
	    Documents
	End

	Menu "view_update"
		UpdateFormats
	End

	Menu "toolbars"
		Toolbars
	End
#
# INSERT MENU
#
	Menu "insert"
# Try and group things a little better, and percolate the most
# commonly used to good positions (exception being mathed, but
# most people using mathed learn key shortcuts quite quickly anyway)
		Submenu "Math|h" "insert_math"
		Submenu "Special Character|p" "insert_special"
		Submenu "Formatting|o" "insert_formatting"
		Submenu "List / TOC|i" "insert_toc"
		Submenu "Float|a" "insert_float"
		Submenu "Note|N" "insert_note"
		Submenu "Branch|B" "insert_branches"
		Submenu "Custom insets" "insert_custom"
		Submenu "File|e" "insert_file"
		Item "Box[[Menu]]" "box-insert Frameless"
		Separator
		Item "Citation...|C" "dialog-show-new-inset citation"
		Item "Cross-Reference...|R" "dialog-show-new-inset ref"
		Item "Label...|L" "label-insert"
		Item "Caption" "caption-insert"
		Item "Index Entry|d" "index-insert"
		Item "Nomenclature Entry...|y" "nomencl-insert"
		Separator
		Item "Table...|T" "tabular-insert"
		Item "Graphics...|G" "dialog-show-new-inset graphics"
		Item "URL" "flex-insert URL"
		Item "Hyperlink|k" "href-insert"
		Item "Footnote|F" "footnote-insert"
		Item "Marginal Note|M" "marginalnote-insert"
		Item "Short Title|S" "optional-insert"
		Item "TeX Code|X" "ert-insert"
		Item "Program Listing[[Menu]]" "listing-insert"
		Item "Date" "date-insert"
	End

	Menu "insert_special"
		Item "Symbols...|b" "dialog-show symbols"
		Item "Ellipsis|i" "specialchar-insert dots"
		Item "End of Sentence|E" "specialchar-insert end-of-sentence"
		Item "Ordinary Quote|Q" "self-insert \""
		Item "Single Quote|S" "quote-insert single"
		Item "Protected Hyphen|y" "specialchar-insert nobreakdash"
		Item "Breakable Slash|a" "specialchar-insert slash"
		Item "Menu Separator|M" "specialchar-insert menu-separator"
		Item "Phonetic Symbols|P" "command-sequence math-mode on; math-insert \text\textipa ; char-forward ;"
	End

	Menu "insert_formatting"
		Item "Superscript|S" "command-sequence math-mode on; math-superscript; math-insert \text;"
		Item "Subscript|u" "command-sequence math-mode on; math-subscript; math-insert \text;"
		Separator
		Item "Protected Space|P" "space-insert protected"
		Item "Inter-word Space|w" "space-insert normal"
		Item "Thin Space|T" "space-insert thin"
		Item "Horizontal Space...|o" "dialog-show-new-inset space"
		Item "Horizontal Line|L" "line-insert"
		Item "Vertical Space...|V" "dialog-show-new-inset vspace"
		Separator
		Item "Hyphenation Point|H" "specialchar-insert hyphenation"
		Item "Ligature Break|k" "specialchar-insert ligature-break"
		Item "Ragged Line Break|R" "newline-insert newline"
		Item "Justified Line Break|J" "newline-insert linebreak"
		Separator
		Item "New Page|N" "newpage-insert newpage"
		Item "Page Break|a" "newpage-insert pagebreak"
		Item "Clear Page|C" "newpage-insert clearpage"
		Item "Clear Double Page|D" "newpage-insert cleardoublepage"
	End

	Menu "insert_math"
		Item "Inline Formula|I" "math-mode on"
		Item "Display Formula|D" "math-display"
		Item "Numbered Formula|N" "command-sequence math-display; math-number-toggle;"
		Item "Eqnarray Environment|E" "command-sequence math-mode on; math-mutate eqnarray;"
		Item "AMS align Environment|a" "command-sequence math-mode on; math-mutate align;"
		Item "AMS alignat Environment|t" "command-sequence math-mode on; math-mutate alignat;"
		Item "AMS flalign Environment|f" "command-sequence math-mode on; math-mutate flalign;"
		Item "AMS gather Environment|g" "command-sequence math-mode on; math-mutate gather;"
		Item "AMS multline Environment|m" "command-sequence math-mode on; math-mutate multline;"
		Separator
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

	Menu "insert_float"
		FloatInsert
# This has got to be better than "Wrap", anyway
		Item "Figure Wrap Float|F" "wrap-insert figure"
		Item "Table Wrap Float|T" "wrap-insert table"
	End

	Menu "insert_toc"
		Item "Table of Contents|C" "toc-insert"
		FloatListInsert
		Item "Index List|I" "index-print"
		Item "Nomenclature|N" "nomencl-print"
		Item "BibTeX Bibliography...|B" "dialog-show-new-inset bibtex"
	End

	Menu "insert_file"
		Item "LyX Document...|X" "file-insert"
		Item "Plain Text...|T" "file-insert-plaintext"
		Item "Plain Text, Join Lines...|J" "file-insert-plaintext-para"
		Separator
		Item "External Material...|M" "dialog-show-new-inset external"
		Item "Child Document...|d" "dialog-show-new-inset include"
	End

	Menu "insert_note"
		Item "LyX Note|N" "note-insert Note"
		Item "Comment|C" "note-insert Comment"
		Item "Greyed Out|G" "note-insert Greyedout"
	End

	Menu "insert_branches"
		Branches
	End

	Menu "insert_custom"
		Custom
	End

#
# DOCUMENT MENU
#

	Menu "document"
		Submenu "Change Tracking|C" "document_change"
		OptItem "Build Program|B" "build-program"
		Item "LaTeX Log|L" "dialog-show latexlog"
		Item "Outline|O" "dialog-toggle toc"
		Item "Start Appendix Here|A" "appendix"
		Separator
		Item "Save in Bundled Format|F" "buffer-toggle-embedding"
		Item "Compressed|m" "buffer-toggle-compression"
		Item "Settings...|S" "dialog-show document"
	End

	Menu "document_change"
		Item "Track Changes|T" "changes-track"
		Item "Merge Changes...|M" "changes-merge"
		Item "Accept Change|A" "change-accept"
		Item "Reject Change|R" "change-reject"
		Item "Accept All Changes|c" "all-changes-accept"
		Item "Reject All Changes|e" "all-changes-reject"
		Item "Show Changes in Output|S" "changes-output"
	End

#
# NAVIGATE MENU
#
	Menu "navigate"
		Submenu "Bookmarks|B" "navigate_bookmarks"
		Item "Next Note|N" "note-next"
		Item "Next Change|C" "change-next"
		Item "Next Cross-Reference|R" "reference-next"
		Item "Go to Label|L" "label-goto"
		Separator
		Toc
	End

	Menu "navigate_bookmarks"
		Item "Save Bookmark 1|S" "bookmark-save 1"
		Item "Save Bookmark 2" "bookmark-save 2"
		Item "Save Bookmark 3" "bookmark-save 3"
		Item "Save Bookmark 4" "bookmark-save 4"
		Item "Save Bookmark 5" "bookmark-save 5"
		Item "Clear Bookmarks|C" "bookmark-clear"
		Separator
		Bookmarks
	End

#
# TOOLS MENU
#

	Menu "tools"
		Item "Spellchecker...|S" "dialog-show spellchecker"
		OptItem "Thesaurus...|T" "thesaurus-entry"
		Item "Statistics...|a" "statistics"
		OptItem "Check TeX|h" "buffer-chktex"
		Item "TeX Information|I" "dialog-show texinfo"
		Separator
# A LOT of applications have Tools->Prefs. Remember this
# should be rarely used - Edit menu is not a good place to
# have it.
		Item "Reconfigure|R" "reconfigure"
		Item "Preferences...|P" "dialog-show prefs"
	End

#
# HELP MENU
#
	Menu "help"
		Item "Introduction|I" "help-open Intro"
		Item "Tutorial|T" "help-open Tutorial"
		Item "User's Guide|U" "help-open UserGuide"
		Item "Extended Features|E" "help-open Extended"
		Item "Embedded Objects|m" "help-open EmbeddedObjects"
		Item "Math|h" "help-open Math"
		Item "Customization|C" "help-open Customization"
		Item "Shortcuts|S" "help-open Shortcuts"
		Item "FAQ|F" "help-open FAQ"
		Item "Table of Contents|a" "help-open TOC"
		Item "LaTeX Configuration|L" "help-open LaTeXConfig"
		Separator
		Item "About LyX|X" "dialog-show aboutlyx"
	End

End

