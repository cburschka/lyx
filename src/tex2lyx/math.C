/** The .tex to .lyx converter
    \author André Pönitz (2003)
 */

// {[(

#include <config.h>

#include "Lsstream.h"
#include "tex2lyx.h"

#include <iostream>
#include <vector>

using std::cerr;
using std::endl;
using std::ostream;
using std::vector;


bool is_math_env(string const & name)
{
	static char const * known_math_envs[] = { "equation", "equation*",
	"eqnarray", "eqnarray*", "align", "align*", "gather", "gather*",
	"multline", "multline*", 0};

	for (char const ** what = known_math_envs; *what; ++what)
		if (*what == name)
			return true;
	return false;
}


void parse_math(Parser & p, ostream & os, unsigned flags, const mode_type mode)
{
	while (p.good()) {
		Token const & t = p.get_token();

#ifdef FILEDEBUG
		cerr << "t: " << t << " flags: " << flags << "\n";
#endif

		if (flags & FLAG_ITEM) {
			if (t.cat() == catSpace)
				continue;

			flags &= ~FLAG_ITEM;
			if (t.cat() == catBegin) {
				// skip the brace and collect everything to the next matching
				// closing brace
				flags |= FLAG_BRACE_LAST;
				continue;
			}

			// handle only this single token, leave the loop if done
			flags |= FLAG_LEAVE;
		}


		//
		// cat codes
		//
		if (t.cat() == catMath) {
			if (mode == MATHTEXT_MODE) {
				// we are inside some text mode thingy, so opening new math is allowed
				Token const & n = p.get_token();
				if (n.cat() == catMath) {
					// TeX's $$...$$ syntax for displayed math
					os << "\\[";
					parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
					os << "\\]";
					p.get_token(); // skip the second '$' token
				} else {
					// simple $...$  stuff
					p.putback();
					os << '$';
					parse_math(p, os, FLAG_SIMPLE, MATH_MODE);
					os << '$';
				}
			}

			else if (flags & FLAG_SIMPLE) {
				// this is the end of the formula
				return;
			}

			else {
				cerr << "\nmode: " << mode << endl;
				p.error("something strange in the parser\n");
				break;
			}
		}

		else if (t.cat() == catLetter ||
			       t.cat() == catSpace ||
			       t.cat() == catSuper ||
			       t.cat() == catSub ||
			       t.cat() == catOther ||
			       t.cat() == catAlign ||
			       t.cat() == catActive ||
			       t.cat() == catParameter)
			os << t.character();

		else if (t.cat() == catNewline) {
			//if (p.next_token().cat() == catNewline) {
			//	p.get_token();
			//	handle_par(os);
			//} else {
				os << "\n "; // note the space
			//}
		}

		else if (t.cat() == catBegin) {
			os << '{';
			parse_math(p, os, FLAG_BRACE_LAST, mode);
			os << '}';
		}

		else if (t.cat() == catEnd) {
			if (flags & FLAG_BRACE_LAST)
				return;
			os << "unexpected '}' in math\n";
		}

		else if (t.cat() == catComment)
			handle_comment(p);

		//
		// control sequences
		//

		else if (t.cs() == "(") {
			os << "\\(";
			parse_math(p, os, FLAG_SIMPLE2, MATH_MODE);
			os << "\\)";
		}

		else if (t.cs() == "[") {
			// special handling of a few common SW user quirks
			p.skip_spaces();
			//if (p.next_token().cs() == 
			os << "\\[";
			parse_math(p, os, FLAG_EQUATION, MATH_MODE);
			os << "\\]";
		}

		else if (t.cs() == "protect")
			// ignore \\protect, will hopefully be re-added during output
			;

		else if (t.cs() == "begin") {
			string const name = p.getArg('{', '}');
			active_environments.push_back(name);
			os << "\\begin{" << name << "}";
			if (name == "tabular")
				parse_math(p, os, FLAG_END, MATHTEXT_MODE);
			else
				parse_math(p, os, FLAG_END, mode);
			os << "\\end{" << name << "}";
		}

		else if (t.cs() == "end") {
			if (flags & FLAG_END) {
				// eat environment name
				string const name = p.getArg('{', '}');
				if (name != active_environment())
					p.error("\\end{" + name + "} does not match \\begin{"
						+ active_environment() + "}");
				active_environments.pop_back();
				return;
			}
			p.error("found 'end' unexpectedly");
		}

		else if (t.cs() == ")") {
			if (flags & FLAG_SIMPLE2)
				return;
			p.error("found '\\)' unexpectedly");
		}

		else if (t.cs() == "]") {
			if (flags & FLAG_EQUATION)
				return;
			p.error("found '\\]' unexpectedly");
		}

		else if (t.cs() == "textrm" || t.cs() == "textsf" || t.cs() == "textbf"
				|| t.cs() == "texttt" || t.cs() == "textsc") {
			os << '\\' << t.cs() << '{';
			parse_math(p, os, FLAG_ITEM, MATHTEXT_MODE);
			os << '}';
		}

		else if (t.cs() == "mbox") {
			os << "\\mbox{";
			parse_math(p, os, FLAG_ITEM, MATHTEXT_MODE);
			os << '}';
		}

		else if (t.cs() == "\"") {
			string const name = p.verbatim_item();
			     if (name == "a") os << 'ä';
			else if (name == "o") os << 'ö';
			else if (name == "u") os << 'ü';
			else if (name == "A") os << 'Ä';
			else if (name == "O") os << 'Ö';
			else if (name == "U") os << 'Ü';
			else os << "\"{" << name << "}";
		}

		else if (t.cs() == "ss")
			os << "ß";

		else 
			os << t.asInput();

		if (flags & FLAG_LEAVE) {
			flags &= ~FLAG_LEAVE;
			break;
		}
	}
}




// }])
