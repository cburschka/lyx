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
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "sgml.h"

#include "insets/insetcommand.h"

#include "support/lstrings.h"
#include "support/lyxlib.h"

#include <stack>

#ifdef HAVE_LOCALE
#endif

using lyx::support::atoi;
using lyx::support::split;

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

	string item_name;
	string command_name;

	ParagraphList::iterator par = const_cast<ParagraphList&>(paragraphs).begin();
	ParagraphList::iterator pend = const_cast<ParagraphList&>(paragraphs).end();

	for (; par != pend; ++par) {
		string sgmlparam;
		string c_depth;
		string c_params;
		int desc_on = 0; // description mode

		LyXLayout_ptr const & style = par->layout();

		// environment tag closing
		for (; depth > par->params().depth(); --depth) {
			if (!environment_inner[depth].empty()) 
			sgml::closeEnvTags(os, false, environment_inner[depth], 
					command_depth + depth);
			sgml::closeTag(os, depth + command_depth, false, environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if (depth == par->params().depth()
		   && environment_stack[depth] != style->latexname()
		   && !environment_stack[depth].empty()) {
				sgml::closeEnvTags(os, false, environment_inner[depth], 
					command_depth + depth);
			sgml::closeTag(os, depth + command_depth, false, environment_stack[depth]);

			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		// Write opening SGML tags.
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			sgml::openTag(os, depth + command_depth,
				    false, style->latexname());
			break;

		case LATEX_COMMAND:
			if (depth != 0)
				//error(ErrorItem(_("Error"), _("Wrong depth for LatexType Command."), par->id(), 0, par->size()));
				;
			
			command_name = style->latexname();

			sgmlparam = style->latexparam();
			c_params = split(sgmlparam, c_depth,'|');

			cmd_depth = atoi(c_depth);

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

			// treat label as a special case for
			// more WYSIWYM handling.
			// This is a hack while paragraphs can't have
			// attributes, like id in this case.
			if (par->isInset(0)) {
				InsetOld * inset = par->getInset(0);
				InsetOld::Code lyx_code = inset->lyxCode();
				if (lyx_code == InsetOld::LABEL_CODE) {
					command_name += " id=\"";
					command_name += (static_cast<InsetCommand *>(inset))->getContents();
					command_name += '"';
					desc_on = 3;
				}
			}

			sgml::openTag(os, depth + command_depth, false, command_name);

			item_name = c_params.empty() ? "title" : c_params;
			sgml::openTag(os, depth + 1 + command_depth, false, item_name);
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
				sgml::openTag(os, depth + command_depth, false, environment_stack[depth]);
			} else {
					sgml::closeEnvTags(os, false, environment_inner[depth], 
						command_depth + depth);
			}

			if (style->latextype == LATEX_ENVIRONMENT) {
				if (!style->latexparam().empty()) {
					if (style->latexparam() == "CDATA")
						os << "<![CDATA[";
					else
						sgml::openTag(os, depth + command_depth, false, style->latexparam());
				}
				break;
			}

			desc_on = (style->labeltype == LABEL_MANUAL);

			environment_inner[depth] = desc_on ? "varlistentry" : "listitem";
			sgml::openTag(os, depth + 1 + command_depth,
				    false, environment_inner[depth]);

			item_name = desc_on ? "term" : "para";
			sgml::openTag(os, depth + 1 + command_depth,
				    false, item_name);
			break;
		default:
			sgml::openTag(os, depth + command_depth,
				    false, style->latexname());
			break;
		}

		par->simpleDocBookOnePar(buf, os, outerFont(par, paragraphs), desc_on,
					 runparams, depth + 1 + command_depth);

		string end_tag;
		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_COMMAND:
			end_tag = c_params.empty() ? "title" : c_params;
			sgml::closeTag(os, depth + command_depth,
				     false, end_tag);
			break;
		case LATEX_ENVIRONMENT:
			if (!style->latexparam().empty()) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				else
					sgml::closeTag(os, depth + command_depth, false, style->latexparam());
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if (desc_on == 1) break;
			end_tag = "para";
			sgml::closeTag(os, depth + 1 + command_depth, false, end_tag);
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
				sgml::closeEnvTags(os, false, environment_inner[depth], 
					command_depth + depth);
		}
	}

	for (int j = command_depth; j >= 0 ; --j)
		if (!command_stack[j].empty()) {
			sgml::closeTag(os, j, false, command_stack[j]);
			os << endl;
		}
}

