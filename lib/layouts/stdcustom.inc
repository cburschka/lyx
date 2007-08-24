# Textclass definition file for LaTeX.
# Author : Martin vermeer <amrtin.vermeer@tkk.fi>
# Custom Inset layouts definition

Format 5

InsetLayout Custom:Endnote
   LyXType	custom
   LatexName	endnote
   LatexType	command
   Font
     Size	Small
   EndFont
   LabelString	foot
   Preamble
     \usepackage{endnote}
   EndPreamble
End

