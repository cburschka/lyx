%
%-------------------------------- NEW ---------------------------------
% ejour2.cls   
% ==========
%
% class definition file for LaTeX2e
% two-column journal articles
%
% Martin Vermeer, June 1997
% =========================
% Lyx 10 version
%
% MV and Jean-Marc Lasgouttes, March 1998
% =======================================
% Lyx 12 version, fixes for reference list and author/year ref format
%
% Call either with 
%   \documentstyle (2.09 compatibility mode)
% or with
%   \documentclass (2e native mode, preferred)
% 
% To be used with LaTeX2e and NFSS.
%
% No promises, no lies (from the GPL, freely interpreted)
% mail address for possible help:
%
%    mv@fgi.fi
%
%--------------------------------- OLD -------------------------------
% LaTeX package LJour2 1.0: main style file
% Springer-Verlag HD
%
% This file has been modified from ljour2.mac, 
% version 1.0 as of 6-May-94
% from Springer-Verlag
%
% LaTeX support of two column journals
%
%% It is for use with: LATEX VERSION 2.09 <25 March 1992>
%% and the NFSS from F. Mittelbach and R. Schoepf
%
% Please report all errors via e-mail to SPRINGER@vax.ntp.springer.de
% or to the address mentioned on page 2 of the documentation
%---------------------------------------------------------------------
%
\NeedsTeXFormat{LaTeX2e}[1994/06/01]
\ProvidesClass{ejour2}
          [1997/06/17 (Martin Vermeer)]
%
% All this is new, 2e-like (MV)
\newif\if@draft
\newif\if@bibay
\newif\if@numart
\newif\if@english
\newif\if@francais
\newif\if@deutsch
\newif\if@footinfo
\newif\if@JG           % Is this Journal of Geodesy?

% Assume english  (MV)
\@englishtrue
\@francaisfalse
\@deutschfalse

%
% All new... -- MV June 1997
%
\DeclareOption{amssymb}{\relax}
\DeclareOption{draft}{\@drafttrue}
\DeclareOption{makeidx}{\relax}
\DeclareOption{script}{\relax}
\DeclareOption{footinfo}{\@footinfotrue}
\DeclareOption{twoside}{\relax}
\DeclareOption{referee}{\@drafttrue}
\DeclareOption{vecmath}{\relax}
\DeclareOption{vecphys}{\relax}
\DeclareOption{numart}{\@numarttrue}
\DeclareOption{fleqn}{\relax}
\DeclareOption{leqno}{\relax}
\DeclareOption{array}{\relax}
\DeclareOption{bibay}{\@bibaytrue}
%
\DeclareOption{deutsch}{\@deutschtrue\@englishfalse}
\DeclareOption{francais}{\@francaistrue\@englishfalse}
\DeclareOption{german}{\@deutschtrue\@englishfalse}
%
\DeclareOption{cropmark}{\relax}
\DeclareOption{psfig}{\relax}
\DeclareOption{JG}{\@JGtrue\@bibaytrue
  \@englishtrue\@francaisfalse\@deutschfalse}

%% 
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ProcessOptions
\LoadClass[fleqn]{article}
%%

%  This was added to make equations always flush left
% Martin Vermeer 27/06/1997
%
\AtBeginDocument{\mathindent\z@}
%
%
\if@draft\@twocolumnfalse\@twosidefalse
\else\@twocolumntrue\@twosidetrue
\fi

%
% This is sprbegch.tex
% changes to LaTeX for Springer styles begin herewith
%
\makeatletter
%
% Added (cannibalized from compatibility mode code):
% Martin Vermeer 19/06/1997
%
\if@compatibility\else
% We are in native mode:
% Make 2e understand these old constructs:
  \def\@footnotemark{%
    \leavevmode
    \ifhmode\edef\@x@sf{\the\spacefactor}\fi
    \@makefnmark
    \ifhmode\spacefactor\@x@sf\fi
     \relax}
  \def\@textsuperscript#1{$\m@th{\@gobble#1}$}
  \def\@makefnmark{\hbox{$^{\@thefnmark}\m@th$}}
\fi
%
\def\@optiondef#1{\expandafter\def\csname sp@#1\endcsname{}}
%
\def\@springererr#1#2{%
\edef\@tempc{#2}\expandafter\errhelp\expandafter{\@tempc}%
\typeout{Springer Style error.  \space See documentation for
 explanation.^^J \space\@spaces\@spaces\@spaces Type \space H <return>
 \space for immediate help.}\errmessage{#1}}
%
\def\@springerwarn#1#2{%
\edef\@tempc{#2}\expandafter\errhelp\expandafter{\@tempc}%
\typeout{Springer Style Warning. \space See documentation for
 explanation.^^J \space\@spaces\@spaces\@spaces Type \space H <return>
 \space for immediate help.}\errmessage{#1}}
\def\springerstylefile{ejour2}
%
% This is document.tex
% checks whether style specified in \documentstyle and style used as
% formatfile are the same
\def\@documentstyle[#1]#2{\makeatletter
  \def\@tempx{#2}
  \def\@optionlist{#1}\gdef\@optionfiles{}%
  \@ifundefined{springerstylefile}{\input \@tempx.sty\relax}%
    {\ifx\@tempx\springerstylefile\relax
     \typeout{This is \springerstylefile.}%
    \else
    \@springererr{Style '\springerstylefile' is preloaded}{Please
     check your input.}\fi
     \@options}
    \let\@elt\input \@optionfiles \let\@elt\relax
    \def\normalsize{\@normalsize}%
    \normalsize\rm
    \mark{{}{}}}
%
% Checks whether options specified to \documentstyle are allowed with
% current style (e.g. are listed in the dataset options.tex)
\def\@options{\let\@elt\relax
  \@for\@tempa:=\@optionlist\do{
     \@ifundefined{sp@\@tempa}%
         {\@springerwarn{Option '\@tempa' not tested with
                        \springerstylefile, this may not work}%
                       {Press return to continue, but keep your
                        fingers crossed.}}%
     {}%
     \@ifundefined{ds@\@tempa}{\xdef\@optionfiles{\@optionfiles
     \@elt \@tempa.sty\relax}}{\csname ds@\@tempa\endcsname}%
  }%
}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is misc.xxx
%
% It defines various commands not available in "plain LaTeX"
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\newcommand{\ts}{\thinspace{}}
\newcommand{\sq}{\hbox{\rlap{$\sqcap$}$\sqcup$}}
\newcommand{\qed}{\ifmmode\sq\else{\unskip\nobreak\hfil
  \penalty50\hskip1em\null\nobreak\hfil\sq
  \parfillskip=0pt\finalhyphendemerits=0\endgraf}\fi{}}
\def\D{{\rm d}}
\def\E{{\rm e}}
\let\eul=\E
\def\I{{\rm i}}
\let\imag=\I
\def\strich{\vskip0.5cm\hrule\vskip3ptplus12pt\null}
% Frame for paste-in figures or tables
\def\mpicplace#1#2{%#1 = width   #2 = height
\vbox{\@tempdima=#2\advance\@tempdima by-2\fboxrule
\hrule\@height \fboxrule\@width #1
\hbox to #1{\vrule\@width \fboxrule\@height\@tempdima\hfil
\vrule\@width \fboxrule\@height\@tempdima}\hrule\@height
\fboxrule\@width #1}}
\def\picplace#1{\mpicplace{\hsize}{#1}}
% Ragged bottom for the actual page
\def\thisbottomragged{\def\@textbottom{\vskip\z@ plus.0001fil
\global\let\@textbottom\relax}}
\flushbottom
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is referee.tex
%
% It defines the style option "referee" = "draft"
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\if@draft
 \typeout{A referee's copy will be produced.}%
 \def\baselinestretch{2}\small
 \normalsize\rm
 \newbox\refereebox
 \setbox\refereebox=\vbox to0pt{\vskip0.5cm%
   \hbox to\textwidth{\normalsize\tt\hrulefill\lower0.5ex
         \hbox{\kern5pt referee's copy\kern5pt}\hrulefill}\vss}%
 \def\@oddfoot{\copy\refereebox}\let\@evenfoot=\@oddfoot
\fi
%
% This is ucgreek
% the definition of versal greek characters
\mathchardef\Gamma="0100
\mathchardef\Delta="0101
\mathchardef\Theta="0102
\mathchardef\Lambda="0103
\mathchardef\Xi="0104
\mathchardef\Pi="0105
\mathchardef\Sigma="0106
\mathchardef\Upsilon="0107
\mathchardef\Phi="0108
\mathchardef\Psi="0109
\mathchardef\Omega="010A
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is vector.tex
%
% It redefines the plain TeX \vec command
% and adds a \tens command for tensors
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\vec@style{\bf}  % hook to change style of vector
                     % default yields mathstyle i.e. boldface upright

\def\vec#1{\relax\ifmmode\mathchoice
{\mbox{\boldmath$\vec@style\displaystyle#1$}}
{\mbox{\boldmath$\vec@style\textstyle#1$}}
{\mbox{\boldmath$\vec@style\scriptstyle#1$}}
{\mbox{\boldmath$\vec@style\scriptscriptstyle#1$}}\else
\hbox{\boldmath$\vec@style\textstyle#1$}\fi}

\def\tens#1{\relax\ifmmode\mathchoice{\mbox{$\sf\displaystyle#1$}}
{\mbox{$\sf\textstyle#1$}}
{\mbox{$\sf\scriptstyle#1$}}
{\mbox{$\sf\scriptscriptstyle#1$}}\else
\hbox{$\sf\textstyle#1$}\fi}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is layout.lj2
%
% It defines various sizes and settings for journals
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\hoffset=-1cm
\voffset=-1.5cm  % changed from -2.5 cm MV
%
\topmargin=0cm
\textwidth=39.5cc
\textheight=640dd
\columnsep=1.5cc
\oddsidemargin=0cm
\evensidemargin=0cm
\headsep=16.74dd
\newdimen\headerboxheight
\headerboxheight=202.23pt % 20 10.5dd-lines - 2\baselineskip
%
\baselineskip=10.5dd
\parindent=15pt
\parskip=0pt plus 1pt
\hfuzz=2pt
\frenchspacing
%
\renewcommand{\topfraction}{.9}
\renewcommand{\bottomfraction}{.9}
\renewcommand{\textfraction}{.1}
\renewcommand{\floatpagefraction}{.9}
\renewcommand{\dbltopfraction}{.9}
\renewcommand{\dblfloatpagefraction}{.9}
%
\tolerance=500
%
\abovedisplayskip=3 mm plus6pt minus 4pt
\belowdisplayskip=3 mm plus6pt minus 4pt
\abovedisplayshortskip=0.0 mm plus6pt
\belowdisplayshortskip=2 mm plus4pt minus 4pt
%
\predisplaypenalty=0
\clubpenalty=20000
\widowpenalty=20000
%
\arraycolsep=1.5pt
%
\newdimen\betweenumberspace          % dimension for space between
\betweenumberspace=3.33pt            % number and text of titles.
\newdimen\aftertext                  % dimension for space after
\aftertext=5pt                       % text of title.
\newdimen\headlineindent             % dimension for space between
\headlineindent=1.166cm              % number and text of headings.
%
%
% Figures and tables are processed in small print
%
\if@compatibility
  % in 2.09 compat mode:   (mv 27/06/97 )
  \def\figure{\small\rm\@float{figure}}
  \@namedef{figure*}{\small\rm\@dblfloat{figure}}
  \def\table{\small\rm\@float{table}}
  \@namedef{table*}{\small\rm\@dblfloat{table}}
\else
  % 2e native mode; I added this (MV)
  \renewenvironment{figure}
                 {\@float{figure}\small}
                 {\end@float}
  \renewenvironment{figure*}
                 {\@dblfloat{figure}\small}
                 {\end@dblfloat}
  \renewenvironment{table}
                 {\@float{table}\small}
                 {\end@float}
  \renewenvironment{table*}
                 {\@dblfloat{table}\small}
                 {\end@dblfloat}
\fi
%
\def\fps@figure{htbp}
\def\fps@table{htbp}
%
\labelsep=5pt                        % measures for lists
\leftmargini=\parindent
\labelwidth\leftmargini\advance\labelwidth-\labelsep
\leftmarginii=\parindent
\parsep=\parskip
%
\def\@listI{\leftmargin\leftmargini
        \parsep=\parskip
        \topsep=\medskipamount
        \itemsep=\parskip \advance\itemsep by -\parsep}
\let\@listi\@listI
\@listi
%
\def\@listii{\leftmargin\leftmarginii
        \labelwidth\leftmarginii\advance\labelwidth by -\labelsep
        \parsep=\parskip
        \topsep=\z@
        \itemsep=\parskip \advance\itemsep by -\parsep}
%
\def\@normalsize{\@setsize\normalsize{10.5dd}\xpt\@xpt
\abovedisplayskip=3 mm plus6pt minus 4pt
\belowdisplayskip=3 mm plus6pt minus 4pt
\abovedisplayshortskip=0.0 mm plus6pt
\belowdisplayshortskip=2 mm plus4pt minus 4pt
\let\@listi\@listI}
%
\def\small{\@setsize\small{9dd}\viiipt\@viiipt
\abovedisplayskip 8.5pt plus 3pt minus 4pt%
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus2pt%
\belowdisplayshortskip 4pt plus2pt minus 2pt
\def\@listi{\leftmargin\leftmargini
\topsep 4pt plus 2pt minus 2pt\parsep\parskip
\itemsep\parskip}}
%
\def\petit{\par\addvspace{6pt}\small}
\def\endpetit{\par\addvspace{6pt}}
%
\def\footnotesize{\@setsize\footnotesize{9dd}\viiipt\@viiipt
\abovedisplayskip 6pt plus 2pt minus 4pt%
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus 1pt%
\belowdisplayshortskip 3pt plus 1pt minus 2pt
\def\@listi{\leftmargin\leftmargini
\topsep 3pt plus 1pt minus 1pt\parsep\parskip
\itemsep\parskip}}
%
\def\scriptsize{\@setsize\scriptsize{8pt}\viipt\@viipt}
\def\tiny{\@setsize\tiny{6pt}\vpt\@vpt}
\def\large{\@setsize\large{13dd}\xiipt\@xiipt}
\def\Large{\@setsize\Large{17dd}\xviipt\@xviipt}
\def\LARGE{\@setsize\LARGE{17dd}\xviipt\@xviipt}
\def\huge{\@setsize\huge{25pt}\xxpt\@xxpt}
\def\Huge{\@setsize\Huge{30pt}\xxvpt\@xxvpt}
% This is texte.tex
% it defines various texts and their translations
% called up with documentclass options
\if@english
\typeout{In English.}
\def\abstractname{Abstract.}
\def\ackname{Acknowledgements.}
\def\andname{and}
\def\lastandname{, and}
\def\appendixname{Appendix}
\def\chaptername{Chapter}
\def\claimname{Claim}
\def\conjecturename{Conjecture}
\def\contentsname{Table of Contents}
\def\corollaryname{Corollary}
\def\definitionname{Definition}
\def\examplename{Example}
\def\exercisename{Exercise}
\def\figurename{Fig.}
\def\keywordname{{\bf Key words:}}
\def\indexname{Index}
\def\lemmaname{Lemma}
\def\contriblistname{List of Contributors}
\def\listfigurename{List of Figures}
\def\listtablename{List of Tables}
\def\mailname{{\it Correspondence to\/}:}
\def\noteaddname{Note added in proof}
\def\notename{Note}
\def\partname{Part}
\def\problemname{Problem}
\def\proofname{Proof}
\def\propertyname{Property}
\def\propositionname{Proposition}
\def\questionname{Question}
\def\refname{References}
\def\remarkname{Remark}
\def\seename{see}
\def\solutionname{Solution}
\def\subclassname{{\it Subject Classifications\/}:}
\def\tablename{Table}
\def\theoremname{Theorem}
\fi
% Names of theorem like environments are already defined
% but must be translated if another language is chosen
%
% French section
%
\if@francais
 \typeout{On parle francais.}%
 \def\abstractname{R\'esum\'e.}%
 \def\ackname{Remerciements.}%
 \def\andname{et}%
 \def\lastandname{ et}%
 \def\appendixname{Appendice}
 \def\chaptername{Chapitre}%
 \def\claimname{Pr\'etention}%
 \def\conjecturename{Hypoth\`ese}%
 \def\contentsname{Table des mati\`eres}%
 \def\corollaryname{Corrolaire}%
 \def\definitionname{D\'efinition}%
 \def\examplename{Exemple}%
 \def\exercisename{Exercice}%
 \def\figurename{Fig.}%
 \def\keywordname{{\bf Mots-cl\'e:}}
 \def\indexname{Index}
 \def\lemmaname{Lemme}%
 \def\contriblistname{Liste des contributeurs}
 \def\listfigurename{Liste des figures}%
 \def\listtablename{Liste des tables}%
 \def\mailname{{\it Correspondence to\/}:}
 \def\noteaddname{Note ajout\'ee \`a l'\'epreuve}%
 \def\notename{Remarque}%
 \def\partname{Partie}%
 \def\problemname{Probl\`eme}%
 \def\proofname{\'Epreuve}%
 \def\propertyname{Caract\'eristique}%
%\def\propositionname{Proposition}%
 \def\questionname{Question}%
 \def\refname{R\'ef\'erences}
 \def\remarkname{Remarque}%
 \def\seename{see}
 \def\solutionname{Solution}%
 \def\subclassname{{\it Subject Classifications\/}:}
 \def\tablename{Tableau}%
 \def\theoremname{Th\'eor\`eme}%
\fi
%
% German section
\if@deutsch
 \typeout{Man spricht Deutsch.}
 \def\abstractname{Zusammenfassung.}%
 \def\ackname{Danksagung.}%
 \def\andname{und}%
 \def\lastandname{ und}%
 \def\appendixname{Anhang}%
 \def\chaptername{Kapitel}%
 \def\claimname{Behauptung}%
 \def\conjecturename{Hypothese}%
 \def\contentsname{Inhaltsverzeichnis}%
 \def\corollaryname{Korollar}%
%\def\definitionname{Definition}%
 \def\examplename{Beispiel}%
 \def\exercisename{\"Ubung}%
 \def\figurename{Abb.}%
 \def\keywordname{{\bf Schl\"usselw\"orter:}}
 \def\indexname{Index}
%\def\lemmaname{Lemma}%
 \def\contriblistname{Mitarbeiter}
 \def\listfigurename{Abbildungsverzeichnis}%
 \def\listtablename{Tabellenverzeichnis}%
 \def\mailname{{\it Correspondence to\/}:}
 \def\noteaddname{Nachtrag}%
 \def\notename{Anmerkung}%
 \def\partname{Teil}%
%\def\problemname{Problem}%
 \def\proofname{Beweis}%
 \def\propertyname{Eigenschaft}%
%\def\propositionname{Proposition}%
 \def\questionname{Frage}%
 \def\refname{Literatur} % Meine zufugung (MV)
 \def\remarkname{Anmerkung}%
 \def\seename{see}
 \def\solutionname{L\"osung}%
 \def\subclassname{{\it Subject Classifications\/}:}
 \def\tablename{Tab.}%
%\def\theoremname{Theorem}%
\fi
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is headjour.tex
%
% Titles for journals are redefined
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\setcounter{secnumdepth}{3}          % depth (3) of the highest-level
                                      % sectioning command
\def\thesection{\arabic{section}}
\def\thesubsection{\thesection.\arabic{subsection}}
\def\thesubsubsection{\thesubsection.\arabic{subsubsection}}
%
%  definition of \chapter
%  not allowed with this springer style
\def\chapter#1{\@springererr{Command \string\chapter \space
                        is not allowed with this Style File}%
                       {Please remove command \string\chapter.}%
                       \relax}
%
\def\sectcounterend{.}
\def\sec@hangfrom#1{\setbox\@tempboxa\hbox{#1}%
      \hangindent \z@\noindent\box\@tempboxa}
%
\def\@sect#1#2#3#4#5#6[#7]#8{\ifnum #2>\c@secnumdepth
   \let\@svsec\@empty\else
   \refstepcounter{#1}\edef\@svsec{\csname the#1%
   \endcsname\sectcounterend\hskip\betweenumberspace\ignorespaces}\fi
   \@tempskipa #5\relax
    \ifdim \@tempskipa>\z@
      \begingroup #6\relax
        \sec@hangfrom{\hskip #3\relax\@svsec}{%
        \raggedright
        \hyphenpenalty \@M%
        \interlinepenalty \@M #8\par}%
      \endgroup
     \csname #1mark\endcsname{#7}
     \addcontentsline
       {toc}{#1}{\ifnum #2>\c@secnumdepth \else
                    \protect\numberline{\csname the#1\endcsname}\fi
                  #7}
% debugging - MV
%                 \typeout{added a line to TOC:}
%                 \typeout{toc}
%                 \typeout{\protect\numberline{\csname the#1\endcsname}}
%                 \typeout{#1}
     \else
     \def\@svsechd{#6\hskip #3\relax
                \@svsec #8\hskip\aftertext\ignorespaces
                    \csname #1mark\endcsname
                    {#7}
                    \addcontentsline
                         {toc}{#1}{\ifnum #2>\c@secnumdepth \else
                           \protect\numberline{\csname the#1\endcsname}\fi
                     #7}
%                   \typeout{added another line to TOC}
                                  }
     \fi
   \@xsect{#5}}
%
%
% measures and setting of sections
%
\def\section{\@startsection{section}{1}{\z@}%
    {-21dd plus-4pt minus-4pt}{10.5dd plus 4pt
     minus4pt}{\normalsize\bf\boldmath}}
\def\subsection{\@startsection{subsection}{2}{\z@}%
    {-21dd plus-4pt minus-4pt}{10.5dd plus 4pt
     minus4pt}{\normalsize\it}}
\def\subsubsection{\@startsection{subsubsection}{3}{\z@}%
    {-21dd plus-4pt minus-4pt}{10.5dd plus 4pt
     minus4pt}{\normalsize}}
\def\paragraph{\@startsection{subsubsection}{4}{\z@}%
    {-21dd plus-4pt minus-4pt}{-1pt}{\normalsize\it}}
%  definition of \subparagraph
%  not allowed with springer styles
\def\subsubparagraph#1{\@springererr{Command \string\subparagraph \space
                        is not allowed with this Style File}%
                       {Please remove command \string\subparagraph.}%
                       \relax}
%  definition of \part
%  not allowed with springer styles
\def\part#1{\@springererr{Command \string\part \space
                        is not allowed with this Style File}%
                       {Please remove command \string\part.}%
                       \relax}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is toc.xxx
%
% it modifies the appearence of the table of contents
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\tableofcontents{
 \@restonecolfalse\if@twocolumn\@restonecoltrue\onecolumn\fi
%  \Chapter*{\contentsname \@mkboth{{\contentsname}}{{\contentsname}}}
 \section*{\contentsname \@mkboth{{\contentsname}}{{\contentsname}}}
 \@starttoc{toc}
 \if@restonecol\twocolumn\fi
                    }

\setcounter{tocdepth}{3}

\def\l@part#1#2{\addpenalty{\@secpenalty}%
   \addvspace{2em plus\p@}%  % space above part line
   \begingroup
     \parindent \z@
     \rightskip \z@ plus 5em
     \hrule\vskip5pt
     \bf\boldmath        % set line in boldface
     \leavevmode          % TeX command to enter horizontal mode.
     #1\par
     \vskip5pt
     \hrule
     \vskip1pt
     \nobreak             % Never break after part entry
   \endgroup}

\def\@dotsep{2}

\def\l@chapter#1#2{\addpenalty{-\@highpenalty}
 \vskip 1.0em plus 1pt \@tempdima \tocchpnum \begingroup
 \parindent \z@ \rightskip \@pnumwidth
 \parfillskip -\@pnumwidth
 \leavevmode \advance\leftskip\@tempdima \hskip -\leftskip
 {\bf\boldmath#1}\nobreak
 \leaders\hbox{$\m@th \mkern \@dotsep mu.\mkern
 \@dotsep mu$}\hfill
 \nobreak\hbox to\@pnumwidth{\hss #2}\par
 \penalty\@highpenalty \endgroup}

\newdimen\tocchpnum
\newdimen\tocsecnum
\newdimen\tocsectotal
\newdimen\tocsubsecnum
\newdimen\tocsubsectotal
\newdimen\tocsubsubsecnum
\newdimen\tocsubsubsectotal
\newdimen\tocparanum
\newdimen\tocparatotal
\newdimen\tocsubparanum
\tocchpnum=20\p@            % chapter {\bf 88.} plus 5.3pt
\tocsecnum=22.5\p@          % section 88.8. plus 4.722pt
\tocsubsecnum=30.5\p@       % subsection 88.8.8 plus 4.944pt
\tocsubsubsecnum=38\p@      % subsubsection 88.8.8.8 plus 4.666pt
\tocparanum=45\p@           % paragraph 88.8.8.8.8 plus 3.888pt
\tocsubparanum=53\p@        % subparagraph 88.8.8.8.8.8 plus 4.11pt
\def\calctocindent{%
\tocsectotal=\tocchpnum
\advance\tocsectotal by\tocsecnum
\tocsubsectotal=\tocsectotal
\advance\tocsubsectotal by\tocsubsecnum
\tocsubsubsectotal=\tocsubsectotal
\advance\tocsubsubsectotal by\tocsubsubsecnum
\tocparatotal=\tocsubsubsectotal
\advance\tocparatotal by\tocparanum}
\calctocindent

\def\l@section{\@dottedtocline{1}{\tocchpnum}{\tocsecnum}}
\def\l@subsection{\@dottedtocline{2}{\tocsectotal}{\tocsubsecnum}}
\def\l@subsubsection{\@dottedtocline{3}{\tocsubsectotal}{\tocsubsubsecnum}}
\def\l@paragraph{\@dottedtocline{4}{\tocsubsubsectotal}{\tocparanum}}
\def\l@subparagraph{\@dottedtocline{5}{\tocparatotal}{\tocsubparanum}}

\def\listoffigures{\@restonecolfalse\if@twocolumn\@restonecoltrue\onecolumn
 \fi\chapter*{\listfigurename\@mkboth{{\listfigurename}}{{\listfigurename}}}
 \@starttoc{lof}\if@restonecol\twocolumn\fi}
\def\l@figure{\@dottedtocline{1}{0em}{\tocsecnum}}

\def\listoftables{\@restonecolfalse\if@twocolumn\@restonecoltrue\onecolumn
 \fi\chapter*{\listtablename\@mkboth{{\listtablename}}{{\listtablename}}}
 \@starttoc{lot}\if@restonecol\twocolumn\fi}
\let\l@table\l@figure

%
%
% This is kopf2.xxx
% it does the whole head of a contribution for two-column journals
% extending the \maketitle command.
% LaTeX does not provide a command to enter the authors institute
% addresses. The \institute command is defined here.

\newcounter{inst}
\newcounter{auth}
\newdimen\instindent
\newbox\authrun
\newtoks\authorrunning
\newbox\titrun
\newtoks\titlerunning

\def\journalname#1{\gdef\@journalname{#1}}

\def\headnote#1{\gdef\@headnote{#1}}
\def\@headnote{}

\def\dedication#1{\gdef\@dedic{#1}}
\def\@dedic{}

\def\institute#1{\gdef\@institute{#1}}

\def\institutename{\par
 \begingroup
 \parindent=0pt
 \parskip=0pt
 \setcounter{inst}{1}%
 \def\and{\par\stepcounter{inst}%
 \noindent
 \hbox to\instindent{\hss$^{\theinst}$\enspace}\ignorespaces}%
 \setbox0=\vbox{\def\thanks##1{}\@institute}  % !!!
 \ifnum\value{inst}>9\relax\setbox0=\hbox{$^{88}$\enspace}%
                 \else\setbox0=\hbox{$^{8}$\enspace}\fi
 \instindent=\wd0\relax
 \ifnum\value{inst}=1\relax
 \else
   \setcounter{inst}{1}%
   \noindent
   \hbox to\instindent{\hss$^{\theinst}$\enspace}\ignorespaces
 \fi
 \small
 \ignorespaces
 \@institute\par
 \endgroup}

%\def\mail#1{\begingroup
%\def\protect{\noexpand\protect\noexpand}\xdef\@thanks{%
%\protect\footnotetext[0]{\kern-\parindent\mailname\
%\ignorespaces#1}\@thanks}\endgroup\ignorespaces}
\def\mail#1{\gdef\@mail{#1}}
\def\@mail{}

\def\@thanks{}

\def\@fnsymbol#1{\ifcase#1\or\star\or{\star\star}\or{\star\star\star}%
   \or \dagger\or \ddagger\or
   \mathchar "278\or \mathchar "27B\or \|\or **\or \dagger\dagger
   \or \ddagger\ddagger \else\@ctrerr\fi\relax}

\def\inst#1{\unskip$^{#1}$}
\def\fnmsep{\unskip$^,$}

\def\subtitle#1{\gdef\@subtitle{#1}}
\def\@subtitle{}

%\def\thesaurus#1{\gdef\@thesaurus{#1}}

\def\ch@ckobl#1#2{\@ifundefined{@#1}
       {\typeout{Missing \expandafter\string\csname#1\endcsname}%
        \csname #1\endcsname{#2}}
       {}}

\newif\if@normhead \@normheadtrue
\def\hugehead{\@normheadfalse
\onecolumn
\let\thanks=\footnote
\long\def\@makefntext##1{\parindent 15pt
     \columnwidth\textwidth\advance\columnwidth -\columnsep
     \divide\columnwidth\tw@\hsize\columnwidth \linewidth\columnwidth
     \noindent\hbox to15pt{\hss$\m@th^{\@thefnmark}$ }
     ##1}%
}

\def\maketitle{\par
%\ch@ckobl{thesaurus}{missing; you have not inserted them}
%
% Added for Journal of Geodesy (Martin Vermeer, June 1997)
%
\if@JG\journalname{Journal of Geodesy}\fi
%
%
\ch@ckobl{journalname}{Noname}
\ch@ckobl{date}{the date of receipt and acceptance should be inserted
later}
\ch@ckobl{title}{A title should be given}
\ch@ckobl{author}{Name(s) and initial(s) of author(s) should be given}
\ch@ckobl{institute}{Address(es) of author(s) should be given}
 \begingroup
 \def\thefootnote{\fnsymbol{footnote}}
 \def\lastand{\ifnum\value{inst}=2\relax
                 \unskip{} \andname\
              \else
                 \unskip, \andname\
              \fi}%
 \def\and{\stepcounter{auth}\relax
          \ifnum\value{auth}=\value{inst}%
             \lastand
          \else
             \unskip,
          \fi}%
 \thispagestyle{empty}
 \if@twocolumn
   \twocolumn[\@maketitle]
 \else
   \newpage \@maketitle
 \fi
 \global\@topnum\z@
 \if@normhead
    \@thanks
 \else
    \if@draft
      \onecolumn[\vspace{\@tempdima}]  % !!!
    \else
      \twocolumn[\vspace{\@tempdima}] 
    \fi
    \addtocounter{page}{-1}
    \if!\the\authorrunning!
       \authorrunning{ }%
    \fi
 \fi
 \if!\@mail!\else
    \footnotetext[0]{\kern-\parindent\mailname\
    \ignorespaces\@mail}%
 \fi
 \def\\{ }\def\inst##1{\unskip}\def\thanks##1{\unskip}%
 \def\fnmsep{\unskip}%
 \if!\the\authorrunning!
   \value{inst}=\value{auth}%
   \setcounter{auth}{1}%
 \else
   \edef\@author{\the\authorrunning}%
 \fi
 \global\setbox\authrun=\hbox{\small\rm\unboldmath\@author\unskip}%
 \instindent=\textwidth
 \advance\instindent by-\headlineindent
 \ifdim\wd\authrun>\instindent
    \typeout{Names of authors too long for running head. Please supply}%
    \typeout{a shorter form with \string\authorrunning\space prior to
             \string\maketitle}%
    \global\setbox\authrun=\hbox{\small\rm
    Authors suppressed due to excessive length}%
 \fi
 \xdef\@author{\copy\authrun}%
 \if!\the\titlerunning!\else
   \edef\@title{\the\titlerunning}%
 \fi
 \global\setbox\titrun=\hbox{\small\rm\unboldmath\ignorespaces\@title}%
 \ifdim\wd\titrun>\instindent
    \typeout{Title too long for running head. Please supply}%
    \typeout{a shorter form with \string\titlerunning\space prior to
             \string\maketitle}%
    \global\setbox\titrun=\hbox{\small\rm
    Title suppressed due to excessive length}%
 \fi
 \xdef\@title{\copy\titrun}%
 \markboth{\@author}{\@title}
 \endgroup
 \setcounter{footnote}{0}
 \let\maketitle\relax
 \let\@maketitle\relax
 \gdef\@thanks{}\gdef\@author{}\gdef\@title{}\gdef\@subtitle{}%
 \let\thanks\relax}

\def\makeheadbox{{%
\hbox to0pt{\vbox{\baselineskip=10dd\hrule\hbox
to\hsize{\vrule\kern3pt\vbox{\kern3pt
\hbox{\bf\@journalname\ Manuscript-Nr.}
\hbox{(will be inserted by hand later)}
\kern3pt}\hfil\kern3pt\vrule}\hrule}%
\hss}}}
\def\@maketitle{\newpage
 \rm
 \vbox to0pt{\vskip-40pt
 \nointerlineskip
 \makeheadbox
 \vss}\nointerlineskip
 \if@normhead
    \setbox\authrun=\vbox\bgroup
 \fi
 {\Large \bf\boldmath
  \pretolerance=10000
  \rightskip=0pt plus 4cm
 \if!\@headnote!\else
   \noindent
   {\em\ignorespaces\@headnote\par}\vskip 3.5mm
 \fi
  \noindent\ignorespaces
  \@title \par}\vskip 11.24pt\relax
 \bf
 \if!\@subtitle!\else
   {\large\bf\boldmath
   \pretolerance=10000
   \rightskip=0pt plus 3cm
   \noindent\ignorespaces\@subtitle \par}\vskip 11.24pt
 \fi
 \lineskip .5em
 \setbox0=\vbox{\setcounter{auth}{1}\def\and{\stepcounter{auth} }%
                \hfuzz=2\textwidth\def\thanks##1{}\@author}%
 \value{inst}=\value{auth}%
 \setcounter{auth}{1}%
 \rightskip=0pt plus 2cm
 \noindent\ignorespaces\@author\vskip7.23pt
 \rightskip=0pt\relax
 \small\rm
 \institutename
 \vskip 12.85pt \noindent\@date
 \if!\@dedic!\else
    \par
    \normalsize\it
    \addvspace\baselineskip
    \noindent\@dedic
 \fi
 \if@normhead
    \egroup % end of header box
    \@tempdima=\headerboxheight
    \advance\@tempdima by-\ht\authrun
    \advance\@tempdima by-\pagetotal
    \ifdim\@tempdima>0pt
       \vskip\@tempdima
    \fi
    \unvbox\authrun
    \global\@minipagetrue
    \global\everypar{\global\@minipagefalse\global\everypar{}}
 \else
    \typeout{\the\c@auth\space author names and}
    \typeout{\the\c@inst\space institutes processed}
    \global\@tempdima=\pagetotal
    \global\advance\@tempdima by22.47pt
 \fi
 \vskip22.47pt
}

% Define `abstract' environment
\def\abstract{\topsep=0pt\partopsep=0pt\parsep=0pt\itemsep=0pt\relax
\trivlist\item[\hskip\labelsep
{\bf\abstractname}]\if!\abstractname!\hskip-\labelsep\fi}
\let\endabstract=\endtrivlist

% Define `transsummary' environment
\def\transsummary{\trivlist\item[\hskip\labelsep
{\bf Summary.}]}
\let\endtranssummary=\endtrivlist

% Define \transtitle, (\trans-)\keywords, \subclass, and \PACS
\def\transtitle#1{\par\addvspace\baselineskip\noindent
{\bf\ignorespaces#1}\par}
\def\transkey#1{\par\addvspace\baselineskip\noindent
{\bf Key words: }\ignorespaces#1\par}
\def\keywords{\par\addvspace\baselineskip\noindent\keywordname\
\ignorespaces}
\def\subclass{\par\addvspace\baselineskip\noindent\subclassname\
\ignorespaces}
\def\PACS{\par\addvspace\baselineskip\noindent{\bf PACS: }\ignorespaces}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is runnhead.j
%
% It redefines the headings of a text. There are two
% pagestyles possible: "\pagestyle{headings}" and
% "\pagestyle{myheadings}". The later is the default.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\if@twoside
\def\ps@headings{\let\@mkboth\markboth
   \def\@oddfoot{}\def\@evenfoot{}
   \def\@evenhead{\rm\thepage\hskip\headlineindent\leftmark\hfil}
   \def\@oddhead{\hfil\rightmark\hskip\headlineindent\rm\thepage}
   \let\chaptermark\@gobble
   \def\sectionmark##1{\markright{{\ifnum\c@secnumdepth>\z@
      \thesection.\hskip\betweenumberspace\fi ##1}}}}
\else \def\ps@headings{\let\@mkboth\markboth
   \def\@oddfoot{}\def\@evenfoot{}
   \def\@oddhead{\hfil\rightmark\hskip\headlineindent\rm\thepage}
   \let\chaptermark\@gobble
   \def\sectionmark##1{\markright{{\ifnum\c@secnumdepth>\z@
      \thesection.\hskip\betweenumberspace\fi ##1}}}}
\fi
\def\ps@myheadings{\let\@mkboth\@gobbletwo
   \def\@oddfoot{}\def\@evenfoot{}
   \def\@evenhead{\small\rm\thepage\hfil\leftmark}
   \def\@oddhead{\small\rm\rightmark\hfil\thepage}
   \def\chaptermark##1{}
   \def\sectionmark##1{}%
   \def\subsectionmark##1{}}
\ps@myheadings
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is theorem.xxx
%
% It redefines the "\newtheorem" command.
%
% Usage now:
%
%     \newtheorem{env_nam}{caption}[within]{cap_font}{body_font}
% or  \newtheorem{env_nam}[numbered_like]{caption}{cap_font}{body_font}
% or  \newtheorem*{env_nam}{caption}{cap_font}{body_font}
%
% New is "cap_font" and "body_font". It stands for
% fontdefinition of the caption and the text itself.
%
% "\newtheorem*" gives a theorem without number.
%
% A defined newthoerem environment is used as described
% by Lamport.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\@thmcounterend{.}

\def\newtheorem{\@ifstar{\@sthm}{\@Sthm}}

% definition of \newtheorem with number

\def\@Sthm#1{\@ifnextchar[{\@othm{#1}}{\@nthm{#1}}}

\def\@xnthm#1#2[#3]#4#5{\expandafter\@ifdefinable\csname #1\endcsname
   {\@definecounter{#1}\@addtoreset{#1}{#3}%
   \expandafter\xdef\csname the#1\endcsname{\expandafter\noexpand
     \csname the#3\endcsname \@thmcountersep \@thmcounter{#1}}%
   \expandafter\xdef\csname #1name\endcsname{#2}%
   \global\@namedef{#1}{\@thm{#1}{\csname #1name\endcsname}{#4}{#5}}%
                              \global\@namedef{end#1}{\@endtheorem}}}

\def\@ynthm#1#2#3#4{\expandafter\@ifdefinable\csname #1\endcsname
   {\@definecounter{#1}%
   \expandafter\xdef\csname the#1\endcsname{\@thmcounter{#1}}%
   \expandafter\xdef\csname #1name\endcsname{#2}%
   \global\@namedef{#1}{\@thm{#1}{\csname #1name\endcsname}{#3}{#4}}%
                               \global\@namedef{end#1}{\@endtheorem}}}

\def\@othm#1[#2]#3#4#5{%
  \@ifundefined{c@#2}{\@latexerr{No theorem environment `#2' defined}\@eha}%
  {\expandafter\@ifdefinable\csname #1\endcsname
  {\global\@namedef{the#1}{\@nameuse{the#2}}%
  \expandafter\xdef\csname #1name\endcsname{#3}%
  \global\@namedef{#1}{\@thm{#2}{\csname #1name\endcsname}{#4}{#5}}%
  \global\@namedef{end#1}{\@endtheorem}}}}

\def\@thm#1#2#3#4{\refstepcounter
    {#1}\@ifnextchar[{\@ythm{#1}{#2}{#3}{#4}}{\@xthm{#1}{#2}{#3}{#4}}}

\def\@xthm#1#2#3#4{\@begintheorem{#2}{\csname the#1\endcsname}{#3}{#4}%
                    \ignorespaces}

\def\@ythm#1#2#3#4[#5]{\@opargbegintheorem{#2}{\csname
       the#1\endcsname}{#5}{#3}{#4}\ignorespaces}

\def\@begintheorem#1#2#3#4{\trivlist
                 \item[\hskip\labelsep{#3#1\ #2\@thmcounterend}]#4}

\def\@opargbegintheorem#1#2#3#4#5{\trivlist
      \item[\hskip\labelsep{#4#1\ #2\ (#3)\@thmcounterend}]#5}

% definition of \newtheorem* without number

\def\@sthm#1#2{\@Ynthm{#1}{#2}}

\def\@Ynthm#1#2#3#4{\expandafter\@ifdefinable\csname #1\endcsname
   {\global\@namedef{#1}{\@Thm{\csname #1name\endcsname}{#3}{#4}}%
    \expandafter\xdef\csname #1name\endcsname{#2}%
    \global\@namedef{end#1}{\@endtheorem}}}

\def\@Thm#1#2#3{\@ifnextchar[{\@Ythm{#1}{#2}{#3}}{\@Xthm{#1}{#2}{#3}}}

\def\@Xthm#1#2#3{\@Begintheorem{#1}{#2}{#3}\ignorespaces}

\def\@Ythm#1#2#3[#4]{\@Opargbegintheorem{#1}
       {#4}{#2}{#3}\ignorespaces}

\def\@Begintheorem#1#2#3{#3\trivlist
                           \item[\hskip\labelsep{#2#1\@thmcounterend}]}

\def\@Opargbegintheorem#1#2#3#4{#4\trivlist
      \item[\hskip\labelsep{#3#1\ (#2)\@thmcounterend}]}

% definition of divers theorem environments

\newtheorem*{claim}{Claim}{\it}{\rm}
\newtheorem{conjecture}{Conjecture}{\it}{\rm}
\newtheorem{corollary}{Corollary}{\bf}{\it}
\newtheorem{definition}{Definition}{\bf}{\it}
\newtheorem{example}{Example}{\it}{\rm}
\newtheorem{exercise}{Exercise}{\bf}{\rm}
\newtheorem{lemma}{Lemma}{\bf}{\it}
\newtheorem{note}{Note}{\it}{\rm}
\newtheorem{problem}{Problem}{\bf}{\rm}
\newtheorem{property}{Property}{\it}{\rm}
\newtheorem{proposition}{Proposition}{\bf}{\it}
\newtheorem{question}{Question}{\it}{\rm}
\newtheorem{solution}{Solution}{\bf}{\rm}
\newtheorem{theorem}{Theorem}{\bf}{\it}
\newtheorem*{proof}{Proof}{\it}{\rm}
\newtheorem{remark}{Remark}{\it}{\rm}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is figure.neu
%
% It redefines the captions for "figure" and "table"
% environments.
%
% There are three new kind of captions: "\firstcaption"
% and "\secondcaption" for captions set side by side.
% Usage for those two commands: like "\caption".
%
% "\sidecaption" with two parms: #1 width of picture
%                                #2 height of picture
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\long\def\@caption#1[#2]#3{\par\addcontentsline{\csname
  ext@#1\endcsname}{#1}{\protect\numberline{\csname
  the#1\endcsname}{\ignorespaces #2}}\begingroup
    \@parboxrestore
    \@makecaption{\csname fnum@#1\endcsname}{\ignorespaces #3}\par
  \endgroup}

\def\firstcaption{\refstepcounter\@captype\@dblarg%
            {\@firstcaption\@captype}}

\def\secondcaption{\refstepcounter\@captype\@dblarg%
            {\@secondcaption\@captype}}

\long\def\@firstcaption#1[#2]#3{\addcontentsline{\csname
  ext@#1\endcsname}{#1}{\protect\numberline{\csname
  the#1\endcsname}{\ignorespaces #2}}\begingroup
    \@parboxrestore
    \vskip3pt
    \@maketwocaptions{\csname fnum@#1\endcsname}{\ignorespaces #3}%
    \ignorespaces\hspace{.073\textwidth}\hfil%
  \endgroup}

\long\def\@secondcaption#1[#2]#3{\addcontentsline{\csname
  ext@#1\endcsname}{#1}{\protect\numberline{\csname
  the#1\endcsname}{\ignorespaces #2}}\begingroup
    \@parboxrestore
    \@maketwocaptions{\csname fnum@#1\endcsname}{\ignorespaces #3}\par
  \endgroup}

\long\def\@maketwocaptions#1#2{%
   \parbox[t]{.46\textwidth}{{\bf #1.} #2}}

\newdimen\figgap\figgap=18pt

\def\sidecaption#1\caption{%
\setbox\@tempboxa=\hbox{#1\unskip}%
\if@twocolumn
 \ifdim\hsize<\textwidth\else
   \ifdim\wd\@tempboxa<\columnwidth
      \typeout{Double column float fits into single column -
            ^^Jyou'd better switch the environment. }%
   \fi
 \fi
\fi
\@tempdima=\hsize
\advance\@tempdima by-\figgap
\advance\@tempdima by-\wd\@tempboxa
\ifdim\@tempdima<3cm
    \typeout{\string\sidecaption: No sufficient room for the legend;
             using normal \string\caption. }%
   \unhbox\@tempboxa
   \let\@capcommand=\@caption
\else
   \let\@capcommand=\@sidecaption
   \leavevmode
   \unhbox\@tempboxa
   \hfill
\fi
\refstepcounter\@captype
\@dblarg{\@capcommand\@captype}}

\long\def\@sidecaption#1[#2]#3{\addcontentsline{\csname
  ext@#1\endcsname}{#1}{\protect\numberline{\csname
  the#1\endcsname}{\ignorespaces #2}}\begingroup
    \@parboxrestore
    \@makesidecaption{\csname fnum@#1\endcsname}{\ignorespaces #3}\par
  \endgroup}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\fig@type{figure}

\def\leftlegendglue{\hfil}

\long\def\@makecaption#1#2{%
 \ifx\@captype\fig@type
   \vskip 3pt
   \setbox\@tempboxa\hbox{{\bf #1.} #2}%
   \ifdim \wd\@tempboxa >\hsize
     {\bf #1.} #2\par
   \else
     \hbox to\hsize{\leftlegendglue\unhbox\@tempboxa\hfil}%
   \fi
 \else
   {\bf #1.} #2\par
   \vskip5.5pt
 \fi}

\long\def\@makesidecaption#1#2{%
   \parbox[b]{\@tempdima}{{\bf #1.} #2}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is bibjour.xxx
%
% It redefines the "bibliography" environment.
%
% Usage: \begin{thebibliography}[label]{title}
%
%        where "title" is the text of the title which will be
%        typeset, e.g. "References" or "Bibliography". It must
%        be given.
%
%        together with "bibay" style option (e.g.
%          \documentstyle[bibay]{article}):
%
%        \bibitem{label}{key}{referenced text}
%             for author-year-citation
%
%        without style option (as described by Lamport):
%
%        \bibitem[label]{key}
%        \bibitem{key}
%
%        \cite{key}  (as described by Lamport)
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% definition of \begin{thebibliography}

\def\thebibliography{\@ifnextchar[{\@lThebibliography}%
                                  {\@Thebibliography}}

\def\@lThebibliography[#1]#2{\section*{#2\@mkboth{#2}{#2}}%
   \small\rm
   \list{\arabic{enumiv}.}{\settowidth\labelwidth{[#1]}
      \leftmargin\labelwidth
      \advance\leftmargin\labelsep
      \usecounter{enumiv}%
      \let\p@enumiv\@empty
      \def\theenumiv{\arabic{enumiv}}}%
   \def\newblock{\hskip .11em plus .33em minus .07em}
   \sloppy\clubpenalty4000\widowpenalty4000
   \sfcode`\.=1000\relax}

\def\@Thebibliography#1{\section*{#1\@mkboth{#1}{#1}}
   \small\rm
   \list{\arabic{enumiv}.}%
   {\settowidth\labelwidth{\hspace\parindent\hskip-\labelsep}%
      \usecounter{enumiv}%
      \let\p@enumiv\@empty
      \def\theenumiv{\arabic{enumiv}}%
      \leftmargin\labelwidth
      \advance\leftmargin\labelsep}
   \def\newblock{\hskip .11em plus .33em minus .07em}
   \sloppy\clubpenalty4000\widowpenalty4000
   \sfcode`\.=1000\relax}

\let\endthebibliography=\endlist

% definition of \bibitem and \cite
%
% Put in the if-else-fi to choose or not author-year type
% reference list (e.g. for Journal of Geodesy)
% Martin Vermeer, June 1997
%
\if@bibay
    \typeout{Author, year style bibliography chosen}
    \def\bibitem{\@bibitem}%
    \def\@bibitem#1#2#3{\ifx!#1\item[]\hskip-\labelwidth
        \hskip-\labelsep\else\item[\@biblabel{#1}]\fi#3
        \if@filesw
           {\def\protect##1{\string ##1\space}\immediate
           \write\@auxout{\string\bibcite{#2}{#3}}}%
        \fi\ignorespaces}%
%
    \def\cite{\@ifnextchar[{\@tempswatrue\@citex}%
                           {\@tempswafalse\@citex[]}}%
%
    \def\@citex[#1]#2{%
        \if@filesw
           \immediate\write\@auxout{\string\citation{#2}}%
%
%          \typeout{\string\citation{#2}}
%
        \fi
        \def\@citea{}%
        \@cite{\@for\@citeb:=#2\do
           {\@citea\def\@citea{,\penalty\@m\ }\@ifundefined
           {b@\@citeb}{{\bf ?}\@warning
           {Citation `\@citeb' on page \thepage \space undefined}}%
           \hbox{\csname b@\@citeb\endcsname}}}%
        {#1}}%
%
    \def\@cite#1#2{{#1\if@tempswa , #2\fi}}%
    \def\@biblabel#1{#1\hfill}

% }
\else
%
\@bibayfalse
\def\bibitem{\@ifnextchar[{\@lbibitem}{\@bibitem}}
\def\@lbibitem[#1]#2{\itemindent\z@
    \item[\@biblabel{#1}]\if@filesw
       {\def\protect##1{\string ##1\space}\immediate
       \write\@auxout{\string\bibcite{#2}{#1}}}\fi\ignorespaces}
%
\def\@bibitem#1{\item\if@filesw \immediate\write\@auxout
       {\string\bibcite{#1}{\the\c@enumiv}}\fi\ignorespaces}
%
\def\cite{\@ifnextchar[{\@tempswatrue\@citex}{\@tempswafalse\@citex[]}}
%
\def\@citex[#1]#2{\if@filesw\immediate\write\@auxout{\string\citation{#2}}\fi
    \def\@citea{}%
    \@cite{\@for\@citeb:=#2\do
       {\@citea\def\@citea{,\penalty\@m\ }\@ifundefined
       {b@\@citeb}{{\bf ?}\@warning
       {Citation `\@citeb' on page \thepage \space undefined}}%
       \hbox{\csname b@\@citeb\endcsname}}}%
    {#1}}
\def\@cite#1#2{[{#1\if@tempswa , #2\fi}]}
\def\@biblabel#1{[#1]\hfill}
\fi
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is biograph.xxx
%
% Usage: \begin{biography}{Author's name}
%        Text of biography
%        \end{biography}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\biography#1{%
\begingroup
\endgraf
\skip0=0pt plus2cm
\advance\skip0 by2\bigskipamount\relax
\vskip\skip0
\small
\setbox0=\vbox\bgroup\def\par{\hfil\break}%
               \sloppy
               \noindent
               \hangindent=113pt
               \hangafter=-16\relax
               \smash{\raise 6.5pt
               \llap{\vbox to0pt{\hrule
                                 \hbox to3.6cm{\vrule height5cm\hfil
                                       \vrule height5cm}
                                 \hrule\vss}\kern11pt}}%
               \vrule height\topskip width0pt
               \if!#1!\else{\sc\ignorespaces#1\/} \fi
               \ignorespaces}
%
\def\endbiography{\egroup
\dimen0=\ht0\advance\dimen0 by\dp0
\ifdim\dimen0<5cm
   \vtop to5cm{\box0\vss}
\else
   {\splittopskip=0pt
   \splitmaxdepth=\maxdepth
   \vbadness=10000
   \setbox1=\vsplit0 to 139.153dd
   \vtop{\box1}%
   \vskip-\prevdepth
   \unvbox0
   \endgraf}
\fi
\endgroup}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is fonotej.xxx
%
% It redefines how footnotes will be typeset.
%
% Usage like described by Lamport.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\newdimen\footnoterulewidth
  \footnoterulewidth=4cc

\def\footnoterule{\kern-3\p@
 \hrule width\footnoterulewidth
 \kern 2.6\p@}

\long\def\@makefntext#1{\parindent 15pt\noindent
            \hbox to15pt{\hss$\m@th^{\@thefnmark}$ }#1}
%
%\long\def\@makefntext#1{\@setpar{\@@par\@tempdima \hsize
%         \advance\@tempdima-15pt\parshape \@ne 15pt \@tempdima}\par
%         \parindent 15pt\noindent \hbox to \z@{%
%         \hss\hss$^{\@thefnmark}$ }#1}
% This is footinfo.tex
% it provides an informatory line on every page
%
\def\SpringerMacroPackageNameA{\springerstylefile}
% \thetime, \thedate and \timstamp are macros to include
% time, date (or both) of the TeX run in the document
\def\maketimestamp{\count255=\time
\divide\count255 by 60\relax
\edef\thetime{\the\count255:}%
\multiply\count255 by-60\relax
\advance\count255 by\time
\edef\thetime{\thetime\ifnum\count255<10 0\fi\the\count255}
\edef\thedate{\number\day-\ifcase\month\or Jan\or Feb\or Mar\or
             Apr\or May\or Jun\or Jul\or Aug\or Sep\or Oct\or
             Nov\or Dec\fi-\number\year}
\def\timstamp{\hbox to\hsize{\tt\hfil\thedate\hfil\thetime\hfil}}}
\maketimestamp
%
% \footinfo generates a info footline on every page containing
% pagenumber, jobname, macroname, and timestamp
% \def\ds@footinfo{
\if@footinfo 
   \typeout{You ordered a foot-info line. }
   \maketimestamp
   \def\@oddfoot{\footnotesize\tt Page: \thepage\hfil job: \jobname\hfil
                 macro: \SpringerMacroPackageNameA\hfil
                 date/time: \thedate/\thetime}%
   \let\@evenfoot=\@oddfoot
\def\footinfo{\maketimestamp
   \ds@footinfo
   \typeout{You ordered a foot-info line (2). }}
\else
\def\nofootinfo{%
   \def\@oddfoot{}\def\@evenfoot{}%
   \typeout{Foot-info has been disabled. }}
\fi
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is environ.xxx
%
% It defines the environment for acknowledgements.
%                            and noteadd
%
% Usage e.g.: \begin{acknowledgement}
%                Text
%             \end{acknowledgement}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Define `abstract' environment
\def\acknowledgement{\par\addvspace{17pt}\small\rm
\trivlist\item[\hskip\labelsep
{\it\ackname}]}
\def\endacknowledgement{\endtrivlist\addvspace{6pt}}
% Define `noteadd' environment
\def\noteadd{\par\addvspace{17pt}\small\rm
\trivlist\item[\hskip\labelsep
{\it\noteaddname}]}
\def\endnoteadd{\endtrivlist\addvspace{6pt}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is item.xxx
%
% It redefines the kind of label for "itemize", "enumerate"
% and "description" environment. The last is extended by
% an optional parameter. Its length is used for overall
% indentation.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% labels of enumerate

\def\labelenumi{\theenumi.}
\def\labelenumii{\theenumii)}
\def\theenumii{\alph{enumii}}
\def\p@enumii{\theenumi}

% labels of itemize

\def\labelitemi{\bf --}
\def\labelitemii{\bf --}
\def\labelitemiii{$\bullet$}
\def\labelitemiv{$\cdot$}

% labels of description
\def\descriptionlabel#1{\hspace\labelsep #1\hfil}

% make indentations changeable

\def\setitemindent#1{\settowidth{\labelwidth}{#1}%
        \leftmargini\labelwidth
        \advance\leftmargini\labelsep
   \def\@listi{\leftmargin\leftmargini
        \labelwidth\leftmargini\advance\labelwidth by -\labelsep
        \parsep=\parskip
        \topsep=\medskipamount
        \itemsep=\parskip \advance\itemsep by -\parsep}}
\def\setitemitemindent#1{\settowidth{\labelwidth}{#1}%
        \leftmarginii\labelwidth
        \advance\leftmarginii\labelsep
\def\@listii{\leftmargin\leftmarginii
        \labelwidth\leftmarginii\advance\labelwidth by -\labelsep
        \parsep=\parskip
        \topsep=\z@
        \itemsep=\parskip \advance\itemsep by -\parsep}}
%
% adjusted environment "description"
% if an optional parameter (at the first two levels of lists)
% is present, its width is considered to be the widest mark
% throughout the current list.
\def\description{\@ifnextchar[{\@describe}{\list{}{\labelwidth\z@
          \itemindent-\leftmargin \let\makelabel\descriptionlabel}}}
%
\def\describelabel#1{#1\hfil}
\def\@describe[#1]{\relax\ifnum\@listdepth=0
\setitemindent{#1}\else\ifnum\@listdepth=1
\setitemitemindent{#1}\fi\fi
\list{--}{\let\makelabel\describelabel}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is typeset.xxx
%
% It defines last text of every article.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\typeset{\small\noindent This article was authored in \LyX\ and typeset
           by \LaTeX, using the \LaTeXe{}\ class file {\it\springerstylefile\/}
             from Springer-Verlag.\par}

\def\enddocument{\par\vspace{2\bigskipamount}\typeset
  \@checkend{document}\clearpage\begingroup
  \if@filesw \immediate\closeout\@mainaux
  \def\global\@namedef##1##2{}\def\newlabel{\@testdef r}%
 \def\bibcite{\@testdef b}\@tempswafalse \makeatletter\input \jobname.aux
  \if@tempswa \@@warning{Label(s) may have changed.  Rerun to get
  cross-references right}\fi\fi\endgroup\deadcycles\z@\@@end}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is numberj.xxx
%
% It redefines the kind of numeration for figures,
% tables and equations like they should appear in
% journals.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\let\ds@numart=\relax
%
\def\thefigure{\@arabic\c@figure}
\def\thetable{\@arabic\c@table}
\def\theequation{\arabic{equation}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             This is mods.lj2
%
% It changes various sizes and settings from one
% to two column journals
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\if@twocolumn\twocolumn\else\onecolumn\fi
\let\sectcounterend\relax
\let\leftlegendglue\relax
\def\endabstract{\endtrivlist}
\def\fps@figure{tbp}
\def\fps@table{tbp}
\headerboxheight=180.83146pt % 20 9.5dd-lines - 2\baselineskip
%%
%% END added: (MV)
%%
\endinput
.

