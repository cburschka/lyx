%%%% This is the cv document class, intended to provide a simple way
%%%% to write your curriculum vitaes (resume)
%%%% Author: Jean-Marc Lasgouttes (Jean-Marc.Lasgouttes@inria.fr) 
%%%%   (with lot of help from Amir Karger <karger@bead.aecom.yu.edu>,
%%%%                      Reuben Thomas <rrt@dcs.gla.ac.uk>
%%%%                      and Dekel Tsur <dekelts@tau.ac.il>)
%%%% WARNING: this document class is really simple. Don't expect too much.
%%%% You can do what you want with this code.
%%
%%% Basic usage:
%% \leftheader{text} : defines what should appear in the upper
%%      left of the first page. `Text' may contain \\ to break lines.
%% \rightheader{text} : like \leftheader, but for the upper right of
%%      the first page
%% \title{text} : defines a title, will will appear centered below the
%%                headers (or above, if the `titleabove' option is used)
%% \maketitle: actually typesets the header. 
%%
%% \section{text} : gives a title for a new topic of the CV. 
%%
%% `topic' environment: begins an itemize-like environment where the
%%   argument of \item[] is typeset in font \itemfont. A line break is
%%   automatically inserted if the label is too long to fit in the
%%   margin (this can be controlled by option `notopicbreak'). 
%%
%% The cv document class also has some support for bibliography. 
%% You can use the `thebibliography' environment as usual, in
%% particular wih BibTeX . The output is similar to the `topic' 
%% environment. If you separate your bibliography into several
%% sections, you may want to use the `contbibnum' document class
%% option. 
%%
%% Note that this class also has support for right-to-left languages,
%% such as hebrew (courtesy Dekel Tsur).
%%
%% The document class accepts some options (along with the usual
%% article.cls options): 
%%   sf            (default) produce title and headers in sans serif fonts
%%   plain         produce all output in roman fonts. 
%%   notopicbreak  do not add a line break after longtopic labels.
%%   contbibnum    let the numbering of bibliography items be
%%                 continuous when there are several thebibliography
%%                 environments 
%%   titleabove    output the title above the left and right headers,
%%                 not below
%%
%% You can also modify directly the fonts used in the document be
%% using the following macros. They take one parameter which is the font
%% changing command.
%% \headerfont: the font used in both headers.
%%		Defaults to sans serif.
%% \titlefont:  the font used for the title.
%%		Defaults to \LARGE sans-serif semi bold condensed.
%% \sectionfont: the font used by \section when beginning a new topic.
%%              Defaults to sans-serif semi bold condensed.
%% \subsectionfont: the font used by \subsection when beginning a new
%%              topic. 
%%              Defaults to sans-serif semi bold condensed.
%% \itemfont:   the font used in descriptions of items.
%%              Defaults to sans-serif slanted.
%%
%% You can modify the following parameters using \renewcommand:
%% \topicmargin: the left margin inside topics.
%%               Defaults to 20% of the text width (0.20\textwidth).

%%% CHANGES:
%% - 1.0 1998/03/23: First public release.
%% - 1.1 1998/11/06: Better documentation, in order to release it for
%%   LyX ,added \refname, disabled all sectionning commands other that
%%   \section, disabled numbering of sections.
%% - 1.2 1998/12/01: 
%%      * Refined the algorithm to display the headers. In
%%        particular, \leftheader and \rightheader do not exist
%%        anymore. 
%%	* Changed the justification of items labels 
%%   [Thanks to Amir Karger <karger@bead.aecom.yu.edu> for the two
%%    changes above]
%%	* Added command \title
%%      * Changed the semantics of \topicfont and \itemfont, and added
%%        \titlefont and \headerfont
%%
%% - 1.3 1999/02/09:
%%      * The thebibliography environment does not add a section by
%%        itself anymore. This means that you can/should add your own
%%        and that \refname does not exist anymore.
%%        [Thanks to Reuben Thomas for the idea]
%%      * Added support for subsections (with associated font command
%%        \subsectionfont).
%%      * Added class options `sf' and `plain'. 
%% - 1.4 2001/05/04
%%      * Added `notopicbreak' class option.
%% - 1.5 2001/06/18
%%	* Added hebrew support (from Dekel Tsur).
%%	* \topicmargin is now a macro and defaults to 20% of text width.
%%      * Added `contbibnum' class option.
%%      * Added `titleabove' class option.
%%      * some variables renaming and cleanups.


%% Basic definition to have a real LaTeX document class
\NeedsTeXFormat{LaTeX2e} 
\ProvidesClass{cv}[2001/06/18 Curriculum vitae version 1.5] 

%% The fonts used in the layout
\newcommand{\sectionfont}[1]{\def\cv@sec@fnt{#1}}
\newcommand{\subsectionfont}[1]{\def\cv@ssec@fnt{#1}}
\newcommand{\itemfont}[1]{\def\cv@it@fnt{#1}}
\newcommand{\headerfont}[1]{\def\cv@hd@fnt{#1}}
\newcommand{\titlefont}[1]{\def\cv@tit@fnt{#1}}

% Some document class options.
\DeclareOption{sf}{
  \sectionfont{\sffamily\fontseries{sbc}\selectfont}
  \subsectionfont{\sffamily\fontseries{sbc}\selectfont}
  \itemfont{\sffamily\slshape}
  \headerfont{\sffamily}
  \titlefont{\sffamily\fontseries{sbc}\selectfont\LARGE}
}
\DeclareOption{plain}{
  \sectionfont{\bfseries}
  \subsectionfont{\bfseries}
  \itemfont{\itshape}
  \headerfont{}
  \titlefont{\bfseries\LARGE}
}
\newif\if@cv@topic@break \@cv@topic@breaktrue
\DeclareOption{notopicbreak}{
  \@cv@topic@breakfalse
}
\newif\if@cv@cont@bib@num \@cv@cont@bib@numfalse
\DeclareOption{contbibnum}{
  \@cv@cont@bib@numtrue
  \newcounter{cv@save@bib@num}
}
\newif\if@cv@title@above \@cv@title@abovefalse
\DeclareOption{titleabove}{
  \@cv@title@abovetrue
}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ExecuteOptions{sf,letterpaper,10pt,oneside,onecolumn,final}
\ProcessOptions
\LoadClass{article}

% Support for RTL (Hebrew). This will be defined and set by babel for
% RtL languages, but we define it here for the benefit of others.
\newif\if@rl

%% stuff needed for the header
\newcommand{\leftheader}[1]{\def\cv@lh{#1}}
\newcommand{\rightheader}[1]{\def\cv@rh{#1}}
\renewcommand{\title}[1]{\def\cv@tit{#1}}
% Default to empty.
\leftheader{}\rightheader{}\title{}
% And now a command to actually show the headers
\newcommand{\cv@header}[1]{%
          \if@rl%
	    \begin{tabular}[t]{@{\cv@hd@fnt\beginR}l@{\endR}}%
          \else%
	    \begin{tabular}[t]{@{\cv@hd@fnt}l@{}}%
          \fi%
              #1%
          \end{tabular}}
\newcommand{\cv@do@title}{\par\bigskip
	\begin{center} \cv@tit@fnt\cv@tit\end{center}}
\renewcommand{\maketitle}{%
	\if@cv@title@above\cv@do@title\fi
	\par\noindent%
	\begin{tabular*}{\textwidth}{@{}l@{\extracolsep{\fill}}l@{}}
	\cv@header{\cv@lh}&\cv@header{\cv@rh}
	\end{tabular*}
	\par
	\if@cv@title@above\else\cv@do@title\fi
	\bigskip\par}

%% Redefine \section to use \cv@sec@fnt
\renewcommand{\section}{
  \@startsection{section}{1}{\z@}
       {-3.5ex \@plus -1ex \@minus -.2ex}
       {2.3ex \@plus .2ex}{\cv@sec@fnt}}
\renewcommand{\subsection}{
  \@startsection{subsection}{1}{3em}
       {-3.5ex \@plus -1ex \@minus -.2ex}
       {2.3ex \@plus .2ex}{\cv@ssec@fnt}}
% the other ones do not exist.
\let\subsubsection=\relax 
\let\paragraph=\relax  \let\subparagraph=\relax
% we do not want any numbering
\setcounter{secnumdepth}{0}

%% Define the topic environment
% The left margin for topics
\newcommand{\topicmargin}{0.20\textwidth}
\newlength{\t@picmargin}
% The label stuff
\newcommand{\@topic@makelabel}[1]{\cv@it@fnt #1\hfill}
\let\@topic@old@item=\@item
\newlength{\topic@label@length}
\def\@topic@item[#1]{%
	\if@cv@topic@break%
		\settowidth{\topic@label@length}{#1}%
		\@topic@old@item[#1]%
		\ifdim\topic@label@length>\labelwidth\mbox{}\\*\fi%
	\else%
		\@topic@old@item[#1]%
	\fi}
\newcommand{\@topic@setup}{%
	\setlength{\t@picmargin}{\topicmargin}
	\if@rl%
	  \setlength{\leftmargin}{0cm}%
	  \setlength{\rightmargin}{\t@picmargin}%
	\else%
	  \setlength{\leftmargin}{\t@picmargin}%
	  \setlength{\rightmargin}{0cm}%
	\fi%
	\setlength{\labelwidth}{0.8\t@picmargin}%
	\let\makelabel=\@topic@makelabel}
\newenvironment{topic}
	{\list{}
	      { \@topic@setup%
	  	\let\@item=\@topic@item}}
	{\endlist}

%% Redefine the thebibliography environment to look like the topic
%% environment. The argument of thebibliography is ignored
\renewenvironment{thebibliography}[1]
 {\list{\@biblabel{\theenumiv}}{%
	\@topic@setup%
	\@openbib@code%
	\usecounter{enumiv}%
	\let\p@enumiv\@empty%
	\renewcommand\theenumiv{\@arabic\c@enumiv}}%
	\if@cv@cont@bib@num%
	  \setcounter{enumiv}{\value{cv@save@bib@num}}%
	\fi%
  \sloppy \clubpenalty 4000 \widowpenalty 4000 \sfcode`\.=\@m}%
 {\endlist%
  \if@cv@cont@bib@num%
    \setcounter{cv@save@bib@num}{\value{enumiv}}%
  \fi}
