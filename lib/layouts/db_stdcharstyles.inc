# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Character Styles definition

Format 35
InsetLayout Flex:Filename
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

InsetLayout Flex:Directory
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

InsetLayout Flex:Firstname
	LyXType element
	LabelString Firstname
	LatexType             Command
	LatexName             firstname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

InsetLayout Flex:Surname
	LyXType element
	LabelString Surname
	LatexType             Command
	LatexName             surname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
End

InsetLayout Flex:Literal
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

InsetLayout Flex:Email
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

InsetLayout Flex:KeyCombo
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

InsetLayout Flex:KeyCap
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

InsetLayout Flex:GuiMenu
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

InsetLayout Flex:GuiMenuItem
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

InsetLayout Flex:GuiButton
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

InsetLayout Flex:MenuChoice
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
