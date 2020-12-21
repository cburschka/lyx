# Moved here by rikiheck. Original author cloudy.

Format 90

#This is just a hacked version of the AMS proof environment
Style Proof
	Preamble
		\ifx\proof\undefined
		\newenvironment{proof}[1][\protect\proofname]{\par
			\normalfont\topsep6\p@\@plus6\p@\relax
			\trivlist
			\itemindent\parindent
			\item[\hskip\labelsep\scshape #1]\ignorespaces
		}{%
			\endtrivlist\@endpefalse
		}
		\providecommand{\proofname}{Proof}
		\fi
	EndPreamble
# No LangPreamble or BabelPreamble, since babel knows \proofname already.
	DocBookTag            para
	DocBookAttr           role='proof'
End
