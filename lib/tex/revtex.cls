%%%%  Start of file revtex.cls %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% This file allows REVTeX v3.1 to function correctly under
%% the 12/94 release of LaTeX 2e. Put this file wherever
%% revtex.sty is. Continue to use \documentstyle{revtex}
%% (with the correct options) and REVTeX will run normally
%% in compatibility mode. Thanks to David Carlisle for
%% pointing out this fix.

\ifx\every@math@size\undefined\else
  \let\old@expast\@expast
  \def\@expast#1{\old@expast{#1}\let\@tempa\reserved@a}
\fi
\typeout{Marc's revtex class file}
\ProvidesPackage{revtex}
\if@compatibility\else\input{latex209.def}\fi
\DeclareOption{10pt}{
	\PackageWarning{revtex}{Ignoring LaTeX option \CurrentOption}
}
\DeclareOption{12pt}{
	\PackageWarning{revtex}{Ignoring LaTeX option \CurrentOption}
}
\DeclareOption{letterpaper}{
	\PackageWarning{revtex}{Ignoring LaTeX option \CurrentOption}
}
\DeclareOption{a4paper}{
	\PackageWarning{revtex}{Ignoring LaTeX option \CurrentOption}
}
\DeclareOption{oneside}{
	\PackageWarning{revtex}{Ignoring LaTeX option \CurrentOption}
}
\DeclareOption{twoside}{
	\PackageWarning{revtex}{Ignoring LaTeX option \CurrentOption}
}
\DeclareOption{onecolumn}{
	\PackageWarning{revtex}{Ignoring LaTeX option \CurrentOption}
}
\DeclareOption{aps}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{osa}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{aip}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{pra}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{prb}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{prc}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{prd}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{pre}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{prl}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{rmp}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{seg}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{manuscript}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{eqsecnum}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{preprint}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{tighten}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{amsfonts}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{amssymb}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{floats}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\DeclareOption{twocolumn}{
	\PassOptionsToPackage{\CurrentOption}{revtex}
}
\ProcessOptions\relax
\RequirePackage{revtex}
\renewcommand{\usepackage}{\RequirePackage}
%%%%  End of file revtex.cls
