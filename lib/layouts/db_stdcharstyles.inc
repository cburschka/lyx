# Textclass definition file for docbook.
# Author : José Abílio Oliveira Matos <jamatos@lyx.org>
# Character Styles definition

Format 84

InsetLayout Flex:Filename
	LabelString Filename
	LatexType             Command
	DocBookTag            filename
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
	ResetsFont true
End

InsetLayout Flex:Directory
	LabelString Directory
	LatexType             Command
	DocBookTag            filename
	DocBookAttr           "class='directory'"
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
	ResetsFont true
End

InsetLayout Flex:Firstname
	LabelString Firstname
	LatexType             Command
	DocBookTag            firstname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
	ResetsFont true
End

InsetLayout Flex:Surname
	LabelString Surname
	LatexType             Command
	DocBookTag            surname
	LabelFont
	  Shape               Up
	  Color               blue
	EndFont
	ResetsFont true
End

InsetLayout Flex:Literal
	LabelString Literal
	LatexType             Command
	DocBookTag            literal
	Font
	  Family              Typewriter
	EndFont
	LabelFont
	  Family              Roman
	  Color               blue
	EndFont
	ResetsFont true
End

InsetLayout Flex:Email
	LabelString Email
        LatexType             Command
        DocBookTag            email
        Font
          Family              Typewriter
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
	ResetsFont true
End

InsetLayout Flex:KeyCombo
	LabelString KeyCombo
        LatexType             Command
        DocBookTag            keycombo
        Font
          Family              Typewriter
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
	ResetsFont true
End

InsetLayout Flex:KeyCap
	LabelString KeyCap
        LatexType             Command
        DocBookTag            keycap
        Font
          Family              Typewriter
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
	ResetsFont true
End

InsetLayout Flex:GuiMenu
	LabelString GuiMenu
        LatexType             Command
        DocBookTag            guimenu
        Font
          Family              Sans
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
	ResetsFont true
End

InsetLayout Flex:GuiMenuItem
	LabelString GuiMenuItem
        LatexType             Command
        DocBookTag            guimenuitem
        Font
          Family              Sans
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
	ResetsFont true
End

InsetLayout Flex:GuiButton
	LabelString GuiButton
        LatexType             Command
        DocBookTag            guibutton
        Font
          Family              Sans
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
	ResetsFont true
End

InsetLayout Flex:MenuChoice
	LabelString MenuChoice
        LatexType             Command
        DocBookTag            menuchoice
        Font
          Family              Sans
        EndFont
        LabelFont
          Family              Roman
          Color               blue
        EndFont
	ResetsFont true
End
