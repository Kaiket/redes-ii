\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{mathnotes}[2014/01/01 Math Notes]

\LoadClass[a4paper, 12pt]{article}

\RequirePackage[spanish]{babel}
\RequirePackage[utf8]{inputenc}
\RequirePackage{etoolbox}
\RequirePackage{makeidx}
\RequirePackage{fancyhdr}
\RequirePackage{chngcntr}
\RequirePackage{hyperref}

\AfterEndPreamble{
	\maketitle
	\newpage
	\tableofcontents
	\newpage
}

\pagestyle{fancy}
\lhead{\color{DarkBlue}{Enrique Cabrerizo Fernández}}
\rhead{\color{DarkBlue}{Guillermo Ruiz Álvarez}}
\cfoot{\thepage}
\setlength{\headheight}{16pt}
\patchcmd{\headrule}{\hrule}{\color{DarkBlue}\hrule}{}{}

\vfuzz=6pt
\hfuzz=6pt

\counterwithin{figure}{section}
\counterwithin{equation}{section}

\makeindex

\endinput