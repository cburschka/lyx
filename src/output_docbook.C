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

#ifdef HAVE_LOCALE
#endif

using lyx::support::subst;

using std::endl;
using std::ostream;
using std::vector;
using std::string;

namespace {

ParagraphList::const_iterator searchParagraph(ParagraphList::const_iterator const & par,
					      ParagraphList::const_iterator const & pend)
{
	ParagraphList::const_iterator p = par + 1;

	for( ; p != pend && p->layout()->latextype == LATEX_PARAGRAPH; ++p);

	return p;
}


ParagraphList::const_iterator searchCommand(ParagraphList::const_iterator const & par,
					    ParagraphList::const_iterator const & pend)
{
	LyXLayout_ptr const & bstyle = par->layout();
	ParagraphList::const_iterator p = par + 1;

	for( ; p != pend; ++p) {
		LyXLayout_ptr const & style = p->layout();
		if( style->latextype == LATEX_COMMAND && style->commanddepth <= bstyle->commanddepth)
			return p;
	}
	return pend;
}


ParagraphList::const_iterator searchEnvironment(ParagraphList::const_iterator const & par,
						ParagraphList::const_iterator const & pend)
{
	LyXLayout_ptr const & bstyle = par->layout();
	ParagraphList::const_iterator p = par + 1;
	for( ; p != pend; ++p) {
		LyXLayout_ptr const & style = p->layout();
		if( style->latextype == LATEX_COMMAND)
			return p;

		if( style->latextype == LATEX_PARAGRAPH) {
			if (p->params().depth() > par->params().depth())
				continue;
			return p;
		}

		if( style->latexname() != bstyle->latexname() and p->params().depth() <= par->params().depth())
			return p;
	}
	return pend;
}


ParagraphList::const_iterator makeParagraph(Buffer const & buf,
					    ostream & os,
					    OutputParams const & runparams,
					    ParagraphList const & paragraphs,
					    ParagraphList::const_iterator const & pbegin,
					    ParagraphList::const_iterator const & pend) {
	ParagraphList::const_iterator par = pbegin;
	Counters & counters = buf.params().getLyXTextClass().counters();
	const int depth = 0;

	for(; par != pend; ++par) {
		LyXLayout_ptr const & style = par->layout();
		string id = par->getDocbookId();
	
		if (!style->latexparam().empty()) {
			counters.step("para");
			int i = counters.value("para");
			id = subst(style->latexparam(), "#", tostr(i));
		}

		id = id.empty()? "": " id = \"" + id + "\"";
		sgml::openTag(os, depth, true, style->latexname(), id);
		par->simpleDocBookOnePar(buf, os, outerFont(par - paragraphs.begin(), paragraphs), runparams, depth);
	
		sgml::closeTag(os, depth, true, style->latexname());
		os << '\n';
	}
	return pend;
}


ParagraphList::const_iterator makeEnvironment(Buffer const & buf,
					      ostream & os,
					      OutputParams const & runparams,
					      ParagraphList const & paragraphs,
					      ParagraphList::const_iterator const & pbegin,
					      ParagraphList::const_iterator const & pend) {
	ParagraphList::const_iterator par = pbegin;

	Counters & counters = buf.params().getLyXTextClass().counters();
	LyXLayout_ptr const & defaultstyle = buf.params().getLyXTextClass().defaultLayout();
	const int depth = 0;

	LyXLayout_ptr const & bstyle = par->layout();
	string item_tag;
	
	string id = par->getDocbookId();
	string env_name = bstyle->latexname();
	// Opening outter tag
	sgml::openTag(os, depth, false, env_name, bstyle->latexparam() + id);
	os << '\n';
	if (bstyle->latextype == LATEX_ENVIRONMENT and bstyle->innertag() == "CDATA")
		os << "<![CDATA[";

	while (par != pend) {
		LyXLayout_ptr const & style = par->layout();
		string id = "";
		ParagraphList::const_iterator send;
		string wrapper = "";

		// Opening inner tag
		switch (bstyle->latextype) {
		case LATEX_ENVIRONMENT:
			if (!bstyle->innertag().empty() and bstyle->innertag() != "CDATA") {
				sgml::openTag(os, depth, true, bstyle->innertag());
			}
			break;

		case LATEX_ITEM_ENVIRONMENT:
			if (!bstyle->labeltag().empty()) {
				sgml::openTag(os, depth, true, bstyle->labeltag());
			} else {
				if (!defaultstyle->latexparam().empty()) {
					counters.step("para");
					id = tostr(counters.value("para"));
					id = " id=\""+ subst(defaultstyle->latexparam(), "#", id) + '"';
					wrapper = defaultstyle->latexname();
				}
				sgml::openTag(os, depth, true, bstyle->itemtag());
			}
		default:
			break;
		}

		switch (style->latextype) {
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			if(par->params().depth() == pbegin->params().depth()) {
				sgml::openTag(os, depth, true, wrapper, id);
				par->simpleDocBookOnePar(buf, os, outerFont(par - paragraphs.begin(), paragraphs), runparams, depth);
				sgml::closeTag(os, depth, true, wrapper);
				++par;
			}
			else {
				send = searchEnvironment(par, pend);
				par = makeEnvironment(buf, os, runparams, paragraphs, par,send);
			}
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraph(par, pend);
			par = makeParagraph(buf, os, runparams, paragraphs, par,send);
			break;
		default:
			break;
		}

		// Closing inner tag
		switch (bstyle->latextype) {
		case LATEX_ENVIRONMENT:
			if (!bstyle->innertag().empty() and bstyle->innertag() != "CDATA") {
				sgml::closeTag(os, depth, true, bstyle->innertag());
				os << '\n';
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			sgml::closeTag(os, depth, true, bstyle->itemtag());
			break;
		default:
			break;
		}
	}

	if (bstyle->latextype == LATEX_ENVIRONMENT and bstyle->innertag() == "CDATA")
		os << "]]>";

	// Closing outter tag
	sgml::closeTag(os, depth, false, env_name);

	return pend;
}


ParagraphList::const_iterator makeCommand(Buffer const & buf,
					  ostream & os,
					  OutputParams const & runparams,
					  ParagraphList const & paragraphs,
					  ParagraphList::const_iterator const & pbegin,
					  ParagraphList::const_iterator const & pend)
{
	Paragraph::depth_type depth = 0; // paragraph depth

	ParagraphList::const_iterator par = pbegin;

	Counters & counters = buf.params().getLyXTextClass().counters();
	LyXLayout_ptr const & bstyle = par->layout();
	
	string id = par->getDocbookId();
	id = id.empty()? "" : " id = \"" + id + "\"";

	if (bstyle->latexparam().find('#') != string::npos) {
		counters.step(bstyle->counter);
	}
	
	if (!bstyle->latexparam().empty()) {
		id = bstyle->latexparam();
		if (id.find('#') != string::npos) {
			string el = expandLabel(buf.params().getLyXTextClass(),
						bstyle, false);
			id = subst(id, "#", el);
		}
	}

	//Open outter tag
	sgml::openTag(os, depth, false, bstyle->latexname(), id);
	os << '\n';

	// Label around sectioning number:
	if (!bstyle->labeltag().empty()) {
		sgml::openTag(os, depth, false, bstyle->labeltag());
		os << expandLabel(buf.params().getLyXTextClass(), bstyle, false);
		sgml::closeTag(os, depth, false, bstyle->labeltag());
	}
	
	// Opend inner tag
	sgml::openTag(os, depth, true, bstyle->innertag());

	par->simpleDocBookOnePar(buf, os, outerFont(par - paragraphs.begin(), paragraphs),
				 runparams, depth);

	// Close inner tags
	sgml::closeTag(os, depth, true, bstyle->innertag());
	os << '\n';
	
	++par;
	while (par != pend) {
		LyXLayout_ptr const & style = par->layout();
		ParagraphList::const_iterator send;

		switch (style->latextype) {
		case LATEX_COMMAND: {
			send = searchCommand(par, pend);
			par = makeCommand(buf, os, runparams, paragraphs, par,send);
			break;
		}
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			send = searchEnvironment(par, pend);
			par = makeEnvironment(buf, os, runparams, paragraphs, par,send);
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraph(par, pend);
			par = makeParagraph(buf, os, runparams, paragraphs, par,send);
			break;
		default:
			break;
		}
	}

	// Close outter tag
	sgml::closeTag(os, depth, false, bstyle->latexname());

	return pend;
}

} // end anonym namespace


void docbookParagraphs(ParagraphList const & paragraphs,
		       Buffer const & buf,
		       ostream & os,
		       OutputParams const & runparams)
{
	ParagraphList::const_iterator par = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();

	while (par != pend) {
		LyXLayout_ptr const & style = par->layout();
		ParagraphList::const_iterator send;

		switch (style->latextype) {
		case LATEX_COMMAND: {
			send = searchCommand(par, pend);
			par = makeCommand(buf, os, runparams, paragraphs, par,send);
			break;
		}
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			send = searchEnvironment(par, pend);
			par = makeEnvironment(buf, os, runparams, paragraphs, par,send);
			break;
		}
		case LATEX_PARAGRAPH:
			send = searchParagraph(par, pend);
			par = makeParagraph(buf, os, runparams, paragraphs, par,send);
			break;
		default:
			break;
		}
	}
}
