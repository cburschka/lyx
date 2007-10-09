Brief discussion on LaTeX grammar
---------------------------------

LaTeX syntax is relatively simple. Any command is like this:

  \command_name [<options>]{<argument_1>}...

This can be expressed as a grammar in yacc/bison syntax. Capitalized words
are tokens (terminals) returned by lex:

command    :    LT_command options arguments
 
arguments  :    argument
           |    argument arguments
	   |    /* empty: It's possible to not have arguments */
	   ;

argument   :    '{'  anything '}'
           ;
	   
options    :     '[' word_list ']'
           |     /* empty: It's possible to don't have options */
	   ;

word_list  :     LT_word ',' word_list
           |     LT_word
	   ;

anything   :     /* Any valid token */
           |     command
	   |     LT_punctuation
	   |     LT_accent
	   |     LT_etc
	   ;

This is quite simple and can be easily parsed by any LALR parser as a yacc
or bison generated one. BUT the difficulty comes because the number of
arguments depends on the value of LT_command, and that the symbols '{' and
'}' have more uses that to separate arguments. This expression:

          \section{Hello world} {\bf Once}

is ambiguos since the parser would interpret the last group as a second
argument. Maybe the simpler approach is declaring a grammar for each of the
commands that LyX will to understand. Another is that if the parser knows
the identity of a command should know also how many arguments should expect.
[Aren't these two suggestions almost identical? I mean, if we tell the parser
what parameters each command take, we also have a grammar for each of the
commands, LyX will understand... Asger]

AAS
