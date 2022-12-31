# theorem environments for the extended theorem modules

Style Criterion
	CopyStyle             Theorem
	DependsOn	          Theorem
	LatexName             criterion
	LabelString           "Criterion \thetheorem."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem{criterion}{\protect\criterionname}
	EndPreamble
	LangPreamble
	    \providecommand{\criterionname}{_(Criterion)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\criterionname}{_(Criterion)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='criterion'
	DocBookTag            para
	DocBookGenerateTitle  true
End

Style Criterion*
	CopyStyle             Theorem*
	LatexName             criterion*
	LabelString           "Criterion."
	Preamble
	  \theoremstyle{plain}
	  \lyx@newtheorem*{criterion*}{\protect\criterionname}
	EndPreamble
	LangPreamble
	  \providecommand{\criterionname}{_(Criterion)}
	EndLangPreamble
	BabelPreamble
	  \addto\captions$$lang{\renewcommand{\criterionname}{_(Criterion)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='criterion'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Algorithm
	CopyStyle             Theorem
	DependsOn	          Theorem
	LatexName             lyxalgorithm
	LabelString           "Algorithm \thetheorem."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem{lyxalgorithm}{\protect\algorithmname}
	EndPreamble
	LangPreamble
	    \providecommand{\algorithmname}{_(Algorithm)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\algorithmname}{_(Algorithm)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='algorithm'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Algorithm*
	CopyStyle             Theorem*
	LatexName             lyxalgorithm*
	LabelString           "Algorithm."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{lyxalgorithm*}{\protect\algorithmname}
	EndPreamble
	LangPreamble
	    \providecommand{\algorithmname}{_(Algorithm)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\algorithmname}{_(Algorithm)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='algorithm'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Axiom
	CopyStyle             Theorem
	DependsOn	            Theorem
	LatexName             ax
	LabelString           "Axiom \thetheorem."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem{ax}{\protect\axiomname}
	EndPreamble
	LangPreamble
	    \providecommand{\axiomname}{_(Axiom)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\axiomname}{_(Axiom)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='axiom'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Axiom*
	CopyStyle             Theorem*
	LatexName             ax*
	LabelString           "Axiom."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{ax*}{\protect\axiomname}
	EndPreamble
	LangPreamble
	    \providecommand{\axiomname}{_(Axiom)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\axiomname}{_(Axiom)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='axiom'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Condition
	CopyStyle             Definition
	LatexName             condition
	LabelString           "Condition \thetheorem."
	Preamble
	    \theoremstyle{definition}
	    \lyx@newtheorem{condition}{\protect\conditionname}
	EndPreamble
	LangPreamble
	    \providecommand{\conditionname}{_(Condition)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\conditionname}{_(Condition)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='condition'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Condition*
	CopyStyle             Definition*
	LatexName             condition*
	LabelString           "Condition."
	Preamble
	    \theoremstyle{definition}
	    \lyx@newtheorem*{condition*}{\protect\conditionname}
	EndPreamble
	LangPreamble
	    \providecommand{\conditionname}{_(Condition)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\conditionname}{_(Condition)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='condition'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Note
	CopyStyle             Remark
	LatexName             note
	LabelString           "Note \thetheorem."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem{note}{\protect\notename}
	EndPreamble
	LangPreamble
	    \providecommand{\notename}{_(Note)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\notename}{_(Note)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='note'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Note*
	CopyStyle             Remark*
	LatexName             note*
	LabelString           "Note."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem*{note*}{\protect\notename}
	EndPreamble
	LangPreamble
	    \providecommand{\notename}{_(Note)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\notename}{_(Note)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='note'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Notation
	CopyStyle             Remark
	LatexName             notation
	LabelString           "Notation \thetheorem."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem{notation}{\protect\notationname}
	EndPreamble
	LangPreamble
	    \providecommand{\notationname}{_(Notation)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\notationname}{_(Notation)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='notation'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Notation*
	CopyStyle             Remark*
	LatexName             notation*
	LabelString           "Notation."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem*{notation*}{\protect\notationname}
	EndPreamble
	LangPreamble
	    \providecommand{\notationname}{_(Notation)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\notationname}{_(Notation)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='notation'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Summary
	CopyStyle             Remark
	LatexName             summary
	LabelString           "Summary \thetheorem."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem{summary}{\protect\summaryname}
	EndPreamble
	LangPreamble
	    \providecommand{\summaryname}{_(Summary)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\summaryname}{_(Summary)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='summary'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Summary*
	CopyStyle             Remark*
	LatexName             summary*
	LabelString           "Summary."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem*{summary*}{\protect\summaryname}
	EndPreamble
	LangPreamble
	    \providecommand{\summaryname}{_(Summary)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\summaryname}{_(Summary)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='summary'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Acknowledgement
	CopyStyle             Remark
	LatexName             acknowledgement
	LabelString           "Acknowledgement \thetheorem."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem{acknowledgement}{\protect\acknowledgementname}
	EndPreamble
	LangPreamble
	    \providecommand{\acknowledgementname}{_(Acknowledgement)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\acknowledgementname}{_(Acknowledgement)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='acknowledgement'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Acknowledgement*
	CopyStyle             Remark*
	LatexName             acknowledgement*
	LabelString           "Acknowledgement."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem*{acknowledgement*}{\protect\acknowledgementname}
	EndPreamble
	LangPreamble
	    \providecommand{\acknowledgementname}{_(Acknowledgement)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\acknowledgementname}{_(Acknowledgement)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='acknowledgement'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Conclusion
	CopyStyle             Remark
	LatexName             conclusion
	LabelString           "Conclusion \thetheorem."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem{conclusion}{\protect\conclusionname}
	EndPreamble
	LangPreamble
	    \providecommand{\conclusionname}{_(Conclusion)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\conclusionname}{_(Conclusion)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='conclusion'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Conclusion*
	CopyStyle             Remark*
	LatexName             conclusion*
	LabelString           "Conclusion."
	Preamble
	    \theoremstyle{remark}
	    \lyx@newtheorem*{conclusion*}{\protect\conclusionname}
	EndPreamble
	LangPreamble
	    \providecommand{\conclusionname}{_(Conclusion)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\conclusionname}{_(Conclusion)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='conclusion'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Assumption
	CopyStyle             Theorem
	DependsOn	          Theorem
	LatexName             assumption
	LabelString           "Assumption \thetheorem."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem{assumption}{\protect\assumptionname}
	EndPreamble
	LangPreamble
	    \providecommand{\assumptionname}{_(Assumption)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\assumptionname}{_(Assumption)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='assumption'
	DocBookTag            para
	DocBookGenerateTitle  true
End


Style Assumption*
	CopyStyle             Theorem*
	LatexName             assumption*
	LabelString           "Assumption."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem*{assumption*}{\protect\assumptionname}
	EndPreamble
	LangPreamble
	    \providecommand{\assumptionname}{_(Assumption)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\assumptionname}{_(Assumption)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='assumption'
	DocBookTag            para
	DocBookGenerateTitle  true
End

Style Question
	CopyStyle             Theorem
	DependsOn             Theorem
	LatexName             question
	LabelString           "Question \thetheorem."
	Preamble
	    \theoremstyle{plain}
	    \lyx@newtheorem{question}{\protect\questionname}
	EndPreamble
	LangPreamble
	    \providecommand{\questionname}{_(Question)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\questionname}{_(Question)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='question'
	DocBookTag            para
	DocBookGenerateTitle  true
End

Style Question*
	CopyStyle             Theorem*
	LatexName             question*
	LabelString           "Question."
	Preamble
	    \theoremstyle{plain}
	   \lyx@newtheorem*{question*}{\protect\questionname}
	EndPreamble
	LangPreamble
	    \providecommand{\questionname}{_(Question)}
	EndLangPreamble
	BabelPreamble
	    \addto\captions$$lang{\renewcommand{\questionname}{_(Question)}}
	EndBabelPreamble
	DocBookWrapperTag     figure
	DocBookWrapperAttr    role='question'
	DocBookTag            para
	DocBookGenerateTitle  true
End

Input theorems-refprefix.inc
