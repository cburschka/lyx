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
#include "support/types.h"

#ifdef HAVE_LOCALE
#endif

using lyx::pos_type;
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

		if(p->params().depth() < par->params().depth())
			return p;

		if( style->latexname() != bstyle->latexname() and p->params().depth() == par->params().depth() )
			return p;
	}
	return pend;
}


ParagraphList::const_iterator makeParagraph(Buffer const & buf,
					    ostream & os,
					    OutputParams const & runparams,
					    ParagraphList const & paragraphs,
					    ParagraphList::const_iterator const & pbegin,
					    ParagraphList::const_iterator const & pend)
{
	LyXLayout_ptr const & defaultstyle = buf.params().getLyXTextClass().defaultLayout();
	for(ParagraphList::const_iterator par = pbegin; par != pend; ++par) {
		if (par->layout() == defaultstyle && par->emptyTag()) {
			par->simpleDocBookOnePar(buf, os, runparams, outerFont(par - paragraphs.begin(), paragraphs));
		} else {
			sgml::openTag(buf, os, runparams, *par);
			par->simpleDocBookOnePar(buf, os, runparams, outerFont(par - paragraphs.begin(), paragraphs));
			sgml::closeTag(os, *par);
			os << '\n';
		}
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

	LyXLayout_ptr const & defaultstyle = buf.params().getLyXTextClass().defaultLayout();
	LyXLayout_ptr const & bstyle = par->layout();
	string item_tag;

	// Opening outter tag
	sgml::openTag(buf, os, runparams, *pbegin);
	os << '\n';
	if (bstyle->latextype == LATEX_ENVIRONMENT and bstyle->pass_thru)
		os << "<![CDATA[";

	while (par != pend) {
		LyXLayout_ptr const & style = par->layout();
		ParagraphList::const_iterator send;
		string id = par->getID(buf, runparams);
		string wrapper = "";
		pos_type sep = 0;

		// Opening inner tag
		switch (bstyle->latextype) {
		case LATEX_ENVIRONMENT:
			if (!bstyle->innertag().empty()) {
				sgml::openTag(os, bstyle->innertag(), id);
			}
			break;

		case LATEX_ITEM_ENVIRONMENT:
			if (!bstyle->labeltag().empty()) {
				sgml::openTag(os, bstyle->innertag(), id);
				sgml::openTag(os, bstyle->labeltag());
				sep = par->getFirstWord(buf, os, runparams) + 1;
				sgml::closeTag(os, bstyle->labeltag());
			}
			wrapper = defaultstyle->latexname();
			sgml::openTag(os, bstyle->itemtag());
		default:
			break;
		}

		switch (style->latextype) {
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT: {
			if(par->params().depth() == pbegin->params().depth()) {
				sgml::openTag(os, wrapper);
				par->simpleDocBookOnePar(buf, os, runparams, outerFont(par - paragraphs.begin(), paragraphs), sep);
				sgml::closeTag(os, wrapper);
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
			if (!bstyle->innertag().empty()) {
				sgml::closeTag(os, bstyle->innertag());
				os << '\n';
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			sgml::closeTag(os, bstyle->itemtag());
			if (!bstyle->labeltag().empty())
				sgml::closeTag(os, bstyle->innertag());
			break;
		default:
			break;
		}
	}

	if (bstyle->latextype == LATEX_ENVIRONMENT and bstyle->pass_thru)
		os << "]]>";

	// Closing outter tag
	sgml::closeTag(os, *pbegin);

	return pend;
}


ParagraphList::const_iterator makeCommand(Buffer const & buf,
					  ostream & os,
					  OutputParams const & runparams,
					  ParagraphList const & paragraphs,
					  ParagraphList::const_iterator const & pbegin,
					  ParagraphList::const_iterator const & pend)
{
	ParagraphList::const_iterator par = pbegin;
	LyXLayout_ptr const & bstyle = par->layout();

	//Open outter tag
	sgml::openTag(buf, os, runparams, *pbegin);
	os << '\n';

	// Label around sectioning number:
	if (!bstyle->labeltag().empty()) {
		sgml::openTag(os, bstyle->labeltag());
		os << expandLabel(buf.params().getLyXTextClass(), bstyle, false);
		sgml::closeTag(os, bstyle->labeltag());
	}

	// Opend inner tag and	close inner tags
	sgml::openTag(os, bstyle->innertag());
	par->simpleDocBookOnePar(buf, os, runparams,  outerFont(par - paragraphs.begin(), paragraphs));
	sgml::closeTag(os, bstyle->innertag());
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
	sgml::closeTag(os, *pbegin);

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
