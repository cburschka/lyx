%% Author: Garst R. Reese  reese@isn.net
%% This is file `hollywood.cls', which provides the current
%% preferred format for submission of "spec" scripts
%% It is a pretty dull and uninteresting format, but it sells
%% Derived from report.cls. There is probably still a lot of stuff
%% that could be deleted. 
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{hollywood} [1998/10/27 v1.0 uses hollywood.layout]
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
% \renewcommand{\headrulewidth}{0pt}
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
\setlength  \labelsep  {.5em}
\setlength  \labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
\renewcommand\theenumi{\@arabic\c@enumi}
\newcommand\labelenumi{\theenumi.}
\renewcommand\p@enumii{\theenumi}
\newcommand\labelitemi{\textbullet}
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
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\sl}{\normalfont\ttfamily}{\mathtt}
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
% This stuff is for a shooting script
% \lhead{\ifthenelse{ \not \value{page} = 1}{CONTINUED:}{}}
% \rfoot{\ifthenelse{ \not \value{page}=\pageref{LastPage}}{(CONT'D)}
%  {\ifthenelse{\value{page}=\pageref{LastPage}}{(END)}{}}}


% This is what we want for a spec script
\rhead{\thepage.}
\cfoot{}
% \renewcommand{\headrulewitdh}{0pt}
  % This one is needed to get real adobe courier, but you can use
  % cmtt which is also correct
  \renewcommand{\familydefault}{pcr}
  % This is to negate the effect of the ~ that LyX adds everywhere
 \let~=\space
  % this protects you against the fact that TeX does not want to
  % hyphenate in typewriter font. It is possible to enable
  % hyphenation, but I do not remember how.
  % That's good because we -do not- want any hyphenation.
  \sloppy
\newcommand{\fadein}{\filbreak\raggedright\par FADE IN: }%
\newcommand{\fadeout}{\filbreak\raggedright\par FADE OUT }%
\newcommand{\interiorshot}{\filbreak\raggedright\par INT. }%
\newcommand{\exteriorshot}{\filbreak\raggedright\par EXT. }%
\newenvironment{dialogue}%
{\raggedright
\begin{list}{}{%
	\setlength{\leftmargin}{1.2in}
	\setlength{\rightmargin}{1.2in}
\setlength{\parsep}{0mm}
\setlength{\topsep}{0mm}}
\item[] \ignorespaces}
{\unskip \end{list}}

\newenvironment{narrative}%
{\filbreak
\raggedright
\begin{list}{}{%
\leftmargin \rightmargin
\setlength{\parsep}{2mm}
\setlength{\topsep}{2mm}}
\item[] \ignorespaces}
{\unskip \end{list}}

\newenvironment{speaker}%
{\filbreak
\begin{list}{}{%
	\setlength{\leftmargin}{2.5in}
\setlength{\partopsep}{4mm}
\setlength{\parsep}{0mm}
\setlength{\topsep}{0mm}}
\item[] \ignorespaces}
{\unskip \end{list}}

\newenvironment{parenthetical}%
{\begin{list}{}{%
	\setlength{\leftmargin}{2in}
\setlength{\parsep}{0mm}
\setlength{\topsep}{0mm}}
	\item[](\ignorespaces}
{\unskip)\end{list}}




\endinput
%%
%% End of file `hollywood.cls'.
