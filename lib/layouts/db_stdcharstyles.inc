# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Character Styles definition

Format 4
CharStyle Filename
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

CharStyle Directory
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

CharStyle Firstname
	LatexType             Command
	LatexName             firstname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

CharStyle Surname
	LatexType             Command
	LatexName             surname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

CharStyle Literal
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

CharStyle Email
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

CharStyle KeyCombo
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

CharStyle KeyCap
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

CharStyle GuiMenu
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

CharStyle GuiMenuItem
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

CharStyle GuiButton
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

CharStyle MenuChoice
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
