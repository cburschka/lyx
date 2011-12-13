# Original Author : Paul Rubin <rubin@msu.edu>
# Modified by Richard Heck to use enumitem

# Definition of case environment as an enumeration

# Define Case as an enumeration environment
# with its own counters

Format 35

Counter casei
        LabelString "Case \arabic{casei}."
End
Counter caseii
        Within casei
        LabelString "Case \roman{caseii}."
End
Counter caseiii
        Within caseii
        LabelString "Case \alph{caseiii}."
End
Counter caseiv
        Within caseiii
        LabelString "Case \arabic{caseiv}."
End

Style Case
  Category              Theorem
  CopyStyle             Enumerate
  LatexName             casenv
  LabelType             Enumerate
  LeftMargin            "Case ###."
  LabelCounter          case
  LabelFont
    Series              Medium
    Shape               Italic
  EndFont
	Requires enumitem
	# It'd be easier to use \setlist[caseenv] here, but that causes problems
	# if you nest an ordinary list inside a case environment. See discussion
	# at bug #7611.
  Preamble
    \newlist{casenv}{enumerate}{4}
    \setlist[casenv]{leftmargin=*,align=left,widest={iiii}}
    \setlist[casenv,1]{label={{\itshape\ \casename} \arabic*.},ref=\arabic*}
    \setlist[casenv,2]{label={{\itshape\ \casename} \roman*.},ref=\roman*}
    \setlist[casenv,3]{label={{\itshape\ \casename\ \alph*.}},ref=\alph*}
    \setlist[casenv,4]{label={{\itshape\ \casename} \arabic*.},ref=\arabic*}
  EndPreamble
  LangPreamble
    \providecommand{\casename}{_(Case)}
  EndLangPreamble
  BabelPreamble
    \addto\captions$$lang{\renewcommand{\casename}{_(Case)}}
  EndBabelPreamble
  RefPrefix OFF
End
