%
% Author: Juergen Vigna (jug@sad.it)
%
% Defining some modifications and some new Commands for an
% italian version to write letters.
%
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{iletter}[1997/06/04 v1.0 Italian Style Letter]
\LoadClassWithOptions{letter}
\setlength\topmargin{-1cm}
\setlength\oddsidemargin{-1cm}
%Grundzeilenabstand 1/6in = 4,23mm (DIN 2107,2142)
\newdimen\GZA
\GZA=1in
\divide\GZA by 6
\newcommand*{\data}[1]{\def\mydata{#1}}
\newcommand*{\citta}[1]{\def\mycitta{#1}}
\newcommand*{\oggetto}[1]{\def\myoggetto{#1}}
\data{}
\citta{}
\oggetto{}
%Nuova definizione del commando opening
\renewcommand*{\opening}[1]{%
%%    \thispagestyle{plain}
    \unitlength1mm
    \begin{picture}(0,0)
        %Absenderadresse und Datum
        \put(0,12.69){\parbox[t]{1\textwidth}{\parbox[t]{3in}{\fromaddress}}}
        %Empfaengeradresse
%%        \put(90,-21.15){\parbox[t]{3in}{\mbox{}\par\toname \\ \toaddress}}
        \put(90,-15.15){\parbox[t]{3in}{\mbox{}\par\toname \\ \toaddress}}
    \end{picture}%
    \mbox{}\par
    \vspace{-1\parskip}
    \vspace{14\GZA}
    \ifx\mydata\@empty\relax
        \ifx\mycitta\@empty\relax\else\mycitta, \@date\fi
    \else\relax
        \ifx\mycitta\@empty\relax\mydata\else\mycitta, \mydata\fi
    \fi
    \mbox{} \par
    %Anpassung wegen der verschiedenen Grundzeilenabstaende
    \ifcase\@ptsize\vspace{0.045pt}\or\vspace{-1.555pt}\or\vspace{-2.955pt}\or\fi
    \ifx\myoggetto\@empty\relax\else Oggetto: \myoggetto\fi
    \mbox{} \par
    \vspace{-1\parskip}
    \vspace{2\GZA}
    %Anpassung wegen der verschiedenen Grundzeilenabstaende
    \ifcase\@ptsize\vspace{0.045pt}\or\vspace{-1.555pt}\or\vspace{-2.955pt}\or\fi
    \newcommand{\Apertura}{#1}
    \ifx\Apertura\empty \else #1\par\fi\nobreak
}

% Redefinizione del commando closing
\renewcommand{\closing}[1]{\par\nobreak\vspace{\parskip}%
    \stopbreaks
    \noindent
    \ignorespaces #1\\[6\medskipamount]%
%%    \ifx\@empty\fromaddress\else
    \hspace*{\longindentation}% \fi
    \parbox{\indentedwidth}{%
        \raggedright
        \ifx\@empty\fromsig
             \fromname
        \else \fromsig \fi\strut
    }%
    \par
}
