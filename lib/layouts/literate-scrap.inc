#
# Suggested style to write your code:
# Within same scrap, lines are separated by new-lines (Ctrl-return), use:
#  ItemSep               0.4
#     . disavantage: lyx doesn't allow space on first column, use proteced space
#                    must type ctrl-return every single line
#     . advantage:   looks better (IMHO)
#                    resembles more closelly the produced paper doc. (more WYSIWYG)
#

OutputType              literate

Style Scrap
  Margin                First_Dynamic
  LatexType             Paragraph
  LatexName             dummy
  NewLine               0
  LeftMargin            MMM
  ParSep                0.4
  TopSep                0.4
  BottomSep             0.4
  ItemSep               0.4
  Align                 Left
  AlignPossible         Block,Left
  FreeSpacing           1
  LabelType             Static

  LabelFont
    Color               magenta
  EndFont
  TextFont
    Latex               Latex
    Family              Typewriter
  EndFont

End
