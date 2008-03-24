# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Character Styles definition

Format 7
InsetLayout Element:Filename
	LyXType element
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

InsetLayout Element:Directory
	LyXType element
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

InsetLayout Element:Firstname
	LyXType element
	LabelString Firstname
	LatexType             Command
	LatexName             firstname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

InsetLayout Element:Surname
	LyXType element
	LabelString Surname
	LatexType             Command
	LatexName             surname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

InsetLayout Element:Literal
	LyXType element
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

InsetLayout Element:Email
	LyXType element
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

InsetLayout Element:KeyCombo
	LyXType element
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

InsetLayout Element:KeyCap
	LyXType element
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

InsetLayout Element:GuiMenu
	LyXType element
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

InsetLayout Element:GuiMenuItem
	LyXType element
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

InsetLayout Element:GuiButton
	LyXType element
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

InsetLayout Element:MenuChoice
	LyXType element
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
