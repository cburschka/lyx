%% Author: Garst R. Reese  reese@isn.net
%% This is file `broadway.cls', which provides a format for writing
%% plays.
%% Derived from report.cls. There is probably still a lot of stuff
%% that could be deleted.
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{broadway} [1999/02/09 v1.0 uses broadway.layout]
\renewcommand{\ref}[1]{#1}
\newcommand\@ptsize{}
\newif\if@restonecol
\newif\if@titlepage
\@titlepagetrue
\newif\if@openright
\DeclareOption{letterpaper}
   {\setlength\paperheight {11in}%
    \setlength\paperwidth  {8.5in}}
\DeclareOption{12pt}{\renewcommand\@ptsize{0}}
\DeclareOption{12pt}{\renewcommand\@ptsize{1}}
\DeclareOption{12pt}{\renewcommand\@ptsize{2}}
\DeclareOption{oneside}{\@twosidefalse \@mparswitchfalse}
\DeclareOption{twoside}{\@twosidetrue  \@mparswitchtrue}
\DeclareOption{draft}{\setlength\overfullrule{5pt}}
\DeclareOption{final}{\setlength\overfullrule{0pt}}
\DeclareOption{titlepage}{\@titlepagetrue}
\DeclareOption{notitlepage}{\@titlepagefalse}
\DeclareOption{openright}{\@openrighttrue}
\DeclareOption{openany}{\@openrightfalse}
\DeclareOption{onecolumn}{\@twocolumnfalse}
\ExecuteOptions{letterpaper,12pt,oneside,onecolumn,final,openany}
\ProcessOptions
\input{size1\@ptsize.clo}
\setlength\lineskip{1\p@}
\setlength\normallineskip{1\p@}
\renewcommand\baselinestretch{}
\setlength\parskip{0\p@ \@plus \p@}
\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
\setcounter{topnumber}{2}
\renewcommand\topfraction{.7}
\setcounter{bottomnumber}{1}
\renewcommand\bottomfraction{.3}
\setcounter{totalnumber}{3}
\renewcommand\textfraction{.2}
\renewcommand\floatpagefraction{.5}
\setcounter{dbltopnumber}{2}
\renewcommand\dbltopfraction{.7}
\renewcommand\dblfloatpagefraction{.5}
  \def\ps@headings{%
    \let\@oddfoot\@empty
    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
    \let\@mkboth\markboth
}
\def\ps@myheadings{%
    \let\@oddfoot\@empty\let\@evenfoot\@empty
    \def\@evenhead{\thepage\hfil\slshape\leftmark}%
    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
    \let\@mkboth\@gobbletwo
    }
  \if@titlepage
\newcommand{\address}{\renewcommand{\@address}}
\newcommand{\@address}{}
  \newcommand\maketitle{\begin{titlepage}%
  \let\footnotesize\small
  \let\footnoterule\relax
  \let \footnote \thanks
  \null\vfil
  \vskip 60\p@
  \begin{center}%
    { \underline{\@title} \par}%
    \vskip 3em%
    {\lineskip .75em%
      \begin{tabular}[t]{c}%
        \@author
      \end{tabular}\par}%
      \vskip 1.5em%
  \end{center}\par
\raggedleft
\vfill
\begin{tabular}{l}%
\ignorespaces
\@address
\end{tabular}\par%
  \@thanks
%  \vfil\null
  \end{titlepage}%
  \setcounter{footnote}{0}%
  \global\let\thanks\relax
  \global\let\maketitle\relax
% \global\let\@address\@empty
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@date\@empty
  \global\let\@title\@empty
% \global\let\address\relax
  \global\let\title\relax
  \global\let\author\relax
  \global\let\date\relax
  \global\let\and\relax
}
\fi
  \setlength\leftmargini  {2.5em}
\leftmargin  \leftmargini
\setlength  \labelsep  {.1em}
\setlength  \labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
\newenvironment{titlepage}
    {%
      \if@twocolumn
        \@restonecoltrue\onecolumn
      \else
        \@restonecolfalse\newpage
      \fi
      \thispagestyle{empty}%
      \setcounter{page}\@ne
    }%
    {\if@restonecol\twocolumn \else \newpage \fi
     \if@twoside\else
        \setcounter{page}\@ne
     \fi
    }
\setlength\tabbingsep{\labelsep}
\skip\@mpfootins = \skip\footins
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\newcommand\@pnumwidth{1.55em}
\newcommand\@tocrmarg{2.55em}
\newcommand\@dotsep{4.5}
\setcounter{tocdepth}{2}
\setlength\columnsep{10\p@}
\setlength\columnseprule{0\p@}
\pagestyle{plain}
\pagenumbering{arabic}
\if@twoside
\else
  \raggedbottom
\fi
\if@twocolumn
  \twocolumn
  \sloppy
  \flushbottom
\else
  \onecolumn
\fi
\usepackage{fancyhdr}
\usepackage{extramarks}
\usepackage{ifthen}
\usepackage{lastpage}
\pagestyle{fancy}
\lhead{\TheTitle}
\chead{\TheAuthor}
\cfoot{}
\rhead{\thepage}
% We do not want any hyphenation
  \sloppy
\newcommand{\TheAuthor}{}
\newcommand{\Author}[1]{\renewcommand{\TheAuthor}{#1}}
\newcommand{\TheTitle}{}
\newcommand{\Title}[1]{\renewcommand{\TheTitle}{#1}}

\newcommand{\dialogue}[1]%
{\raggedright
\hspace{1in}
\parbox{4in}{#1}
}

\newcommand{\narrative}[1]%
{\filbreak
\raggedright{#1}
}

\newcommand{\speaker}[1]%
{\filbreak
\vspace{12pt}
\hspace{2.25in}{#1}
\vspace{6pt}
}

\newcommand{\parenthetical}[1]%
{\raggedright
\hspace{1.75in}
\parbox{3in}{(#1)}
\vspace{6pt}}

\newcounter{actctr}
\newcounter{scenectr}[actctr]
\renewcommand{\theactctr}{\Roman{actctr}}
\renewcommand{\thescenectr}{\arabic{scenectr}}
\providecommand{\atrise}[1]{\par\raggedright AT RISE: #1 }%
\providecommand{\scenestar}[1]{\filbreak\par\centering SCENE #1 }%
\providecommand{\scene}{%
\filbreak\par\stepcounter{scenectr}\centering SCENE \thescenectr}%
\providecommand{\act}{%
\stepcounter{actctr}\filbreak\setcounter{scenectr}{0}\par\centering ACT \theactctr\par}%
\providecommand{\curtain}{\filbreak\par\centering CURTAIN \par}%

\endinput
%%
%% End of file `broadway.cls'.
