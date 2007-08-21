# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Character Styles definition

Format 5
InsetLayout CharStyle:Filename
	LyXType charstyle
	LabelString Filename
	LatexType             Command
	LatexName             filename
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
End

InsetLayout CharStyle:Directory
	LyXType charstyle
	LabelString Directory
	LatexType             Command
	LatexName             filename
	LatexParam            "class=<directory>"
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
End

InsetLayout CharStyle:Firstname
	LyXType charstyle
	LabelString Firstname
	LatexType             Command
	LatexName             firstname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

InsetLayout CharStyle:Surname
	LyXType charstyle
	LabelString Surname
	LatexType             Command
	LatexName             surname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

InsetLayout CharStyle:Literal
	LyXType charstyle
	LabelString Literal
	LatexType             Command
	LatexName             literal
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
End

InsetLayout CharStyle:Email
	LyXType charstyle
	LabelString Email
        LatexType             Command
        LatexName             email
        Font
          Family              Typewriter
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
End

InsetLayout CharStyle:KeyCombo
	LyXType charstyle
	LabelString KeyCombo
        LatexType             Command
        LatexName             keycombo
        Font
          Family              Typewriter
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
End

InsetLayout CharStyle:KeyCap
	LyXType charstyle
	LabelString KeyCap
        LatexType             Command
        LatexName             keycap
        Font
          Family              Typewriter
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
End

InsetLayout CharStyle:GuiMenu
	LyXType charstyle
	LabelString GuiMenu
        LatexType             Command
        LatexName             guimenu
        Font
          Family              Sans
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
End

InsetLayout CharStyle:GuiMenuItem
	LyXType charstyle
	LabelString GuiMenuItem
        LatexType             Command
        LatexName             guimenuitem
        Font
          Family              Sans
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
End

InsetLayout CharStyle:GuiButton
	LyXType charstyle
	LabelString GuiButton
        LatexType             Command
        LatexName             guibutton
        Font
          Family              Sans
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
End

InsetLayout CharStyle:MenuChoice
	LyXType charstyle
	LabelString MenuChoice
        LatexType             Command
        LatexName             menuchoice
        Font
          Family              Sans
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
End
