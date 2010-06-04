# Moved here by rgheck. Original author cloudy.

Format 27

#This is just a hacked version of the AMS proof environment
Style Proof
	Preamble
		\ifx\proof\undefined\
		\newenvironment{proof}[1][\protect\proofname]{\par
			\normalfont\topsep6\p@\@plus6\p@\relax
			\trivlist
			\itemindent\parindent
			\item[\hskip\labelsep
					\scshape
#1]\ignorespaces
}{%
			\endtrivlist\@endpefalse
}
		\fi
	EndPreamble
	LangPreamble
		\providecommand{\proofname}{_(Proof)}
	EndLangPreamble
	BabelPreamble
		\addto\captions$$lang{\renewcommand{\proofname}{_(Proof)}}
	EndBabelPreamble
End
