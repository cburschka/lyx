/**
 * \file output_linuxdoc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "output_linuxdoc.h"

#include "buffer.h"
#include "bufferparams.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphList_fwd.h"
#include "ParagraphParameters.h"
#include "sgml.h"

#include <stack>

using std::ostream;
using std::stack;
using std::vector;
using std::string;

void linuxdocParagraphs(Buffer const & buf,
			ParagraphList const & paragraphs,
			ostream & os,
			OutputParams const & runparams)
{

	Paragraph::depth_type depth = 0; // paragraph depth
	string item_name;
	vector<string> environment_stack(5);

	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		LyXLayout_ptr const & style = pit->layout();
		// treat <toc> as a special case for compatibility with old code
		if (!pit->empty() && pit->isInset(0)) {
			InsetBase const * inset = pit->getInset(0);
			if (inset->lyxCode() == InsetBase::TOC_CODE) {
				string const temp = "toc";
				sgml::openTag(os, temp);
				continue;
			}
		}

		// environment tag closing
		for (; depth > pit->params().depth(); --depth) {
			sgml::closeTag(os, environment_stack[depth]);
			environment_stack[depth].erase();
		}

		// write opening SGML tags
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			if (depth == pit->params().depth()
			   && !environment_stack[depth].empty()) {
				sgml::closeTag(os, environment_stack[depth]);
				environment_stack[depth].erase();
				if (depth)
					--depth;
				else
					os << "</p>";
			}
			sgml::openTag(os, style->latexname());
			break;

		case LATEX_COMMAND:
			if (depth != 0)
				//error(ErrorItem(_("Error:"), _("Wrong depth for LatexType Command.\n"), pit->id(), 0, pit->size()));
				;

			if (!environment_stack[depth].empty()) {
				sgml::closeTag(os, environment_stack[depth]);
				os << "</p>";
			}

			environment_stack[depth].erase();
			sgml::openTag(os, style->latexname());
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
		case LATEX_BIB_ENVIRONMENT: {
			string const & latexname = style->latexname();

			if (depth == pit->params().depth()
			    && environment_stack[depth] != latexname) {
				sgml::closeTag(os, environment_stack[depth]);
				environment_stack[depth].erase();
			}
			if (depth < pit->params().depth()) {
			       depth = pit->params().depth();
			       environment_stack[depth].erase();
			}
			if (environment_stack[depth] != latexname) {
				if (depth == 0) {
					sgml::openTag(os, "p");
				}
				sgml::openTag(os, latexname);

				if (environment_stack.size() == depth + 1)
					environment_stack.push_back("!-- --");
				environment_stack[depth] = latexname;
			}

			if (style->latexparam() == "CDATA")
				os << "<![CDATA[";

			if (style->latextype == LATEX_ENVIRONMENT) break;

			if (style->labeltype == LABEL_MANUAL)
				item_name = "tag";
			else
				item_name = "item";

			sgml::openTag(os, item_name);
		}
		break;

		default:
			sgml::openTag(os, style->latexname());
			break;
		}

		pit->simpleLinuxDocOnePar(buf, os,
			outerFont(pit - paragraphs.begin(), paragraphs),
					  runparams, depth);

		os << "\n";
		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_COMMAND:
			break;
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
		case LATEX_BIB_ENVIRONMENT:
			if (style->latexparam() == "CDATA")
				os << "]]>";
			break;
		default:
			sgml::closeTag(os, style->latexname());
			break;
		}
	}

	// Close open tags
	for (int i = depth; i >= 0; --i)
		sgml::closeTag(os, environment_stack[i]);
}
