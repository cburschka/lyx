/**
 * \file output_docbook.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "output_docbook.h"

#include "buffer.h"
#include "bufferparams.h"
#include "counters.h"
#include "debug.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "sgml.h"

#include "insets/insetcommand.h"

#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/tostr.h"

#include <stack>

#ifdef HAVE_LOCALE
#endif

using lyx::support::atoi;
using lyx::support::split;
using lyx::support::subst;

using std::endl;
using std::ostream;
using std::stack;
using std::vector;
using std::string;


void docbookParagraphs(Buffer const & buf,
		       ParagraphList const & paragraphs,
		       ostream & os,
		       OutputParams const & runparams)
{
	vector<string> environment_stack(10);
	vector<string> environment_inner(10);
	vector<string> command_stack(10);

	bool command_flag = false;
	Paragraph::depth_type command_depth = 0;
	Paragraph::depth_type command_base = 0;
	Paragraph::depth_type cmd_depth = 0;
	Paragraph::depth_type depth = 0; // paragraph depth

	string command_name;

	string item_tag;

	ParagraphList::const_iterator par = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();

	Counters & counters = buf.params().getLyXTextClass().counters();

	for (; par != pend; ++par) {

		LyXLayout_ptr const & style = par->layout();

		// environment tag closing
		for (; depth > par->params().depth(); --depth) {
			sgml::closeEnvTags(os, false, environment_inner[depth],
				item_tag, command_depth + depth);
			sgml::closeTag(os, depth + command_depth, false, environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if (depth == par->params().depth()
		   && environment_stack[depth] != style->latexname()
		   && !environment_stack[depth].empty()) {
				sgml::closeEnvTags(os, false, environment_inner[depth],
					item_tag, command_depth + depth);
			sgml::closeTag(os, depth + command_depth, false, environment_stack[depth]);

			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		string ls = par->getDocbookId();
		if (!ls.empty())
			ls = " id = \"" + ls + "\"";

		// Write opening SGML tags.
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			if (!style->latexparam().empty()) {
				counters.step("para");
				int i = counters.value("para");
				ls = subst(style->latexparam(), "#", tostr(i));
			}

			sgml::openTag(os, depth + command_depth,
				    false, style->latexname(), ls);
			break;

		case LATEX_COMMAND:
			if (depth != 0)
				//error(ErrorItem(_("Error"), _("Wrong depth for LatexType Command."), par->id(), 0, par->size()));
				;

			command_name = style->latexname();

			cmd_depth = style->commanddepth;

			if (command_flag) {
				if (cmd_depth < command_base) {
					for (Paragraph::depth_type j = command_depth;
					     j >= command_base; --j) {
						sgml::closeTag(os, j, false, command_stack[j]);
						os << endl;
					}
					command_depth = command_base = cmd_depth;
				} else if (cmd_depth <= command_depth) {
					for (int j = command_depth;
					     j >= int(cmd_depth); --j) {
						sgml::closeTag(os, j, false, command_stack[j]);
						os << endl;
					}
					command_depth = cmd_depth;
				} else
					command_depth = cmd_depth;
			} else {
				command_depth = command_base = cmd_depth;
				command_flag = true;
			}
			if (command_stack.size() == command_depth + 1)
				command_stack.push_back(string());
			command_stack[command_depth] = command_name;

			if (style->latexparam().find('#') != string::npos) {
				counters.step(style->counter);
			}

			if (!style->latexparam().empty()) {
				ls = style->latexparam();
				if (ls.find('#') != string::npos) {
					string el = expandLabel(buf.params().getLyXTextClass(),
						style, false);
					ls = subst(ls, "#", el);
				}
			}

			sgml::openTag(os, depth + command_depth, false, command_name, ls);

			// Label around sectioning number:
			if (!style->labeltag().empty()) {
				sgml::openTag(os, depth + 1 + command_depth, false,
					style->labeltag());
				os << expandLabel(buf.params().getLyXTextClass(), style, false);
				sgml::closeTag(os, depth + 1 + command_depth, false,
					style->labeltag());
			}

			// Inner tagged header text, e.g. <title> for sectioning:
			sgml::openTag(os, depth + 1 + command_depth, false,
				style->innertag());
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < par->params().depth()) {
				depth = par->params().depth();
				environment_stack[depth].erase();
			}

			if (environment_stack[depth] != style->latexname()) {
				if (environment_stack.size() == depth + 1) {
					environment_stack.push_back("!-- --");
					environment_inner.push_back("!-- --");
				}
				environment_stack[depth] = style->latexname();
				environment_inner[depth] = "!-- --";
				// outputs <environment_stack[depth] latexparam()>
				sgml::openTag(os, depth + command_depth, false,
						environment_stack[depth], style->latexparam() + ls);
			} else {
				sgml::closeEnvTags(os, false, environment_inner[depth],
					style->itemtag(), command_depth + depth);
			}

			if (style->latextype == LATEX_ENVIRONMENT) {
				if (!style->innertag().empty()) {
					if (style->innertag() == "CDATA")
						os << "<![CDATA[";
					else
						sgml::openTag(os, depth + command_depth, false,
							style->innertag());
				}
				break;
			}

			environment_inner[depth] = style->innertag();

			if (!environment_inner[depth].empty())
				sgml::openTag(os, depth + 1 + command_depth,
				    false, environment_inner[depth]);
			break;
		default:
			sgml::openTag(os, depth + command_depth,
				    false, style->latexname(), ls);
			break;
		}

		par->simpleDocBookOnePar(buf, os,
			outerFont(par - paragraphs.begin(), paragraphs),
			runparams, depth + 1 + command_depth);

		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_COMMAND:
			sgml::closeTag(os, depth + command_depth, false,
				style->innertag());
			break;
		case LATEX_ENVIRONMENT:
			if (!style->innertag().empty()) {
				if (style->innertag() == "CDATA")
					os << "]]>";
				else
					sgml::closeTag(os, depth + command_depth, false,
						style->innertag());
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			item_tag = style->itemtag();
			break;
		case LATEX_PARAGRAPH:
			sgml::closeTag(os, depth + command_depth, false, style->latexname());
			break;
		default:
			sgml::closeTag(os, depth + command_depth, false, style->latexname());
			break;
		}
	}

	// Close open tags
	for (int d = depth; d >= 0; --d) {
		if (!environment_stack[depth].empty()) {
			sgml::closeEnvTags(os, false, environment_inner[d], item_tag, command_depth + d);
			sgml::closeTag(os, d + command_depth, false, environment_stack[d]);
		}
	}

	for (int j = command_depth; j >= 0 ; --j)
		if (!command_stack[j].empty()) {
			sgml::closeTag(os, j, false, command_stack[j]);
			os << endl;
		}
}
