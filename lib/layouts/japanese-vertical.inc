# Japanese standard textclass definition file.
# Author : Koji Yokota (yokota6@gmail.com)
#
# This file is included in the following Japanese standard classes:
# - tarticle
# - tbook
# - treport

InsetLayout Flex:Rensuji
	LyxType				custom
	LatexType			Command
	LatexName			rensuji
	MenuString			"Rensuji|R"
	LabelString			"Rensuji"
	ForcePlain			1
	Argument 1
		LabelString		"Pos"
		MenuString		"Alignment"
		Tooltip			"Insert 'c' for centering, 'r' for flushright and 'l' for flushleft"
	EndArgument
End

InsetLayout Flex:Rensuji*
	CopyStyle			Flex:Rensuji
	LatexName			rensuji*
	MenuString			"Rensuji with no Skip Adjustment|n"
	LabelString			"Rensuji (no skip adj.)"
End

InsetLayout Flex:Bou
	CopyStyle			Flex:Rensuji
	LatexName			bou
	MenuString			"Bouten"
	LabelString			"Bouten"
End

InsetLayout Flex:Kasen
	CopyStyle			Flex:Rensuji
	LatexName			kasen
	MenuString			"Bousen"
	LabelString			"Bousen"
End
