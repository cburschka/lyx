# Textclass definition file for LaTeX.
# Author : Martin vermeer <martin.vermeer@tkk.fi>
# Custom Inset layouts definition

Format 5

InsetLayout Custom:Endnote
   LyXType	custom
   LatexName	endnote
   LatexType	command
   Decoration	classic
   Font
     Size	Small
   EndFont
   LabelString	endnote
   Preamble
     \usepackage{endnotes}
   EndPreamble
End

