%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Prolog
%
% Shortcut generator v1.0
%
% This program is known to work with SWI-Prolog version 2.9.5, which you
% can find at
%
%    http://www.swi.psy.uva.nl/usr/jan/SWI-Prolog.html
%
% Purpose: to generate shortcuts for labels in menus and dialogs that are
% guaranteed to be unique within a set of labels. The shortcuts are
% generated in a prioritized manner, such that characters at the beginning
% of words are preferred to characters in the middle of words.
%
% You might find this progam useful if you are translating LyX.
%
% Don't use this file in the development/tools-directory; copy it
% somewhere out of the LyX-distribution tree, unless you're improving
% the tool itself.  The input is to be hard-coded into the file and that
% may cause unnecessary garbage to appear in any patch you're putting
% together.
%
% The labels are supposed to be entered at "LABELS" below,
% and you get one solution (hopefully) with "go.". 
% If there are no solutions, the Prolog-interpretator will reply with a
% "No."
%
% You can get all candidate solutions with "all.", but this requires
% lots of memory.
%
% You can inspect the correspondance between the original strings and
% the prioritized ones with "inspect.".
%
% I have no idea what the big O for the algorithm is, but I suspect it's
% rather steep.
% Asger speculates that the algorithm is O(n^m), where n is the cardinality
% of the candidate sets, while m is the number of sets. Since we do an
% exhausitive search, this has to be the case.

% Predicates:

% print out one solution, i.e. a set of label/shortcut's
% writef/2 may be specific for SWI-prolog
show_one_alternative([]).
show_one_alternative([L/_/C|Rest]):-
  writef('%s|#%n\n',[L,C]),
  show_one_alternative(Rest).

% printout predicate for "all."
show_solutions([]).
show_solutions([H|T]):-
  write('-----------\n'),
  show_one_alternative(H),
  show_solutions(T).

% print out correspondance between original strings and prioritized ones.
show_priority([]).
show_priority([L/P/_|Rest]):-
  writef('%s|%n\n',[L,P]),
  show_priority(Rest).

% character is from a set of allowed characters.
% "AZaz09"=[65, 90, 97, 122, 48, 57]
allowed_char(C):-
  C >= 97, C =< 122. % a-z
allowed_char(C):-
  C >= 65, C =< 90. % A-Z
allowed_char(C):-
  C >= 48, C =< 57. % 0-9

% turn lowercase to uppercase; alt-<key> is case insensitive
uppercase(L,U):-
  (L >= 97, L =< 122) -> U is L - 32;
  U is L.

% possible_char/2: Gets all characters in label, one after one.
possible_char(_/Label,Char):-
  member(Char,Label). 		% the character is part of the label

% prepare_labels/2: Prepares all labels. Constructs a new list of pairs 
% where the original string is coupled with the prepared string.
prepare_labels([], []).
prepare_labels([H1|T1], [H1/H2|T2]):-
	prepare_string(H1, H2),
	prepare_labels(T1, T2).

% prepare_string/2: Prepares a string by removing duplicate characters,
% prioritizing initials letters, removing illegal characters and turning
% lowercase to uppercase characters.
prepare_string(Label,Result):-
  string_to_list(Label,List1),	% make a list of the string
  prioritize(List1, List2),	% Prioritize string
  filter_chars(List2, List3),	% Filter out unwanted chars
  unique(List3, Result). 	% Remove duplicates

% prioritize/2: This predicate rearranges a list, such that
% chars at the beginning of words are put first in the list.
% i.e. prioritize("Foo bar", "Fboo ar")" is true.
prioritize(L1,L2):-
	initial_chars(L1, I),	% Find all initial characters
	subtract(L1, I, Rest),	% and the others
	append(I, Rest, L2).	% and we have the result.

% initial_chars/2: Returns a list of characters that appear at the beginning
% of words. i.e. initial_chars("Foo bar", "Fb") is true.
initial_chars([],[]).
initial_chars([A|T1], [A|T3]):-
	rest_after_space(T1, T2),	% Return rest of list after space
	initial_chars(T2, T3).

% rest_after_space/2: Returns the list after the first space.
% i.e. "rest_after_space("Foo bar", "bar") is true.
rest_after_space([], []).
rest_after_space([32, H1|T1], [H1|T1]):- !.
rest_after_space([_|T1], T2):-
	rest_after_space(T1, T2).

% filterchars/2: Filter outs non-allowed characters from list, and turns
% lowercase to uppercase.
filter_chars([], []).
filter_chars([H|T1], [C|T2]):-
 	allowed_char(H), !, uppercase(H, C),
	filter_chars(T1, T2).
filter_chars([_|T1], T2):-
	filter_chars(T1, T2).

% unique/2: This predicate removes duplicate characters without reordering.
% i.e. unique("Foo bar", "Fo bar") is true.
unique([], []).
unique([H|T1], [H|T3]):-
	delete(T1, H, T2),	% Remove duplicates.
	unique(T2,T3).

% working_shortcuts/2 instantiates Char in the first argument for a whole
% menu/dialog.
working_shortcuts([],_). % end of the list

working_shortcuts([Label/Char|Rest],Used):-
  possible_char(Label,Char), % i.e. part of the label string
  \+member(Char,Used),  % not used by any other shortcut
  working_shortcuts(Rest,[Char|Used]). % and all the other labels have
                                       % shortcuts
% The prolog motor will backtrack up and down the list of labels
% until it finds a set with unique characters for each label


% Label strings goes here.
% Just cut&paste the strings from the LyX-source, and remove any
% control sequences for menus.  We could add a couple of predicates to scan
% a file, but re-consulting this file takes only a fraction of a second
% so I didn't bother to add a fancy user interface here.  The solution is
% printed like <label> '|#' <key>, allthough this isn't nessecarily useful,
% for menus for instance.
%
% The characters are picked with priority from left to right within
% strings, and the labels are listed in falling priority.
% If you want a certain label to have a certain shortcut, try adding that
% character in front of the string (like 'Exit' -> 'xExit') and move it
% higher up in the list.
% If this doesn't work, replace the string with only that character
% ('Exit' -> 'x'). If you get a "No." then, you lose.
% Use "inspect." to inspect the resulting priotized strings.

% LABELS
lab(Labels):-
  Strings = [
"New...",
"New from template...",
"Open...",
"Close",
"Save",
"Save As...",
"Revert to saved",
"View dvi",
"View PostScript",
"Update dvi",
"Update PostScript",
"Print...",
"Fax...",
"Export",
"Exit"
% NB, no comma on the last one. Easy to forget.
  ],
  prepare_labels(Strings, Str),
  bagof(L/_,member(L,Str),Labels).

% Inspect mapping between original string and prioritized ones.
inspect:-
	lab(Labels),
	show_priority(Labels).

% Find ALL solutions (they are often legion - don't bother ;)
all:- % May (probably, on large dialogs) run out on memory.
  lab(Labels),
  setof(Labels,working_shortcuts(Labels,[]),Solutions),
  show_solutions(Solutions).

% Find one solution
one:-
  lab(Labels),
  working_shortcuts(Labels,[]),
  show_one_alternative(Labels).

% Entry point.
go:-
  one.
