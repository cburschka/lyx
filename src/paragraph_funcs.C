/**
 * \file paragraph_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "paragraph_funcs.h"

#include "buffer.h"
#include "bufferparams.h"

#include "debug.h"
#include "encoding.h"
#include "errorlist.h"
#include "factory.h"
#include "gettext.h"
#include "iterators.h"
#include "language.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "outputparams.h"
#include "paragraph_pimpl.h"
#include "sgml.h"
#include "texrow.h"
#include "vspace.h"

#include "insets/insetbibitem.h"
#include "insets/insethfill.h"
#include "insets/insetlatexaccent.h"
#include "insets/insetline.h"
#include "insets/insetnewline.h"
#include "insets/insetpagebreak.h"
#include "insets/insetoptarg.h"
#include "insets/insetspace.h"
#include "insets/insetspecialchar.h"
#include "insets/insettabular.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/std_sstream.h"

#include <vector>

using lyx::pos_type;

using lyx::support::ascii_lowercase;
using lyx::support::atoi;
using lyx::support::bformat;
using lyx::support::compare_ascii_no_case;
using lyx::support::compare_no_case;
using lyx::support::contains;
using lyx::support::split;
using lyx::support::subst;

using std::auto_ptr;
using std::endl;
using std::string;
using std::vector;
using std::istringstream;
using std::ostream;
using std::pair;


namespace {

bool moveItem(Paragraph & from, Paragraph & to,
	BufferParams const & params, pos_type i, pos_type j)
{
	char const tmpchar = from.getChar(i);
	LyXFont tmpfont = from.getFontSettings(params, i);

	if (tmpchar == Paragraph::META_INSET) {
		InsetOld * tmpinset = 0;
		if (from.getInset(i)) {
			// the inset is not in a paragraph anymore
			tmpinset = from.insetlist.release(i);
		}

		if (!to.insetAllowed(tmpinset->lyxCode()))
			return false;
		to.insertInset(j, tmpinset, tmpfont);
	} else {
		if (!to.checkInsertChar(tmpfont))
			return false;
		to.insertChar(j, tmpchar, tmpfont);
	}
	return true;
}

}


void breakParagraph(BufferParams const & bparams,
		    ParagraphList & paragraphs,
		    ParagraphList::iterator par,
		    pos_type pos,
		    int flag)
{
	// create a new paragraph, and insert into the list
	ParagraphList::iterator tmp = paragraphs.insert(boost::next(par),
							Paragraph());

	// without doing that we get a crash when typing <Return> at the
	// end of a paragraph
	tmp->layout(bparams.getLyXTextClass().defaultLayout());
	// remember to set the inset_owner
	tmp->setInsetOwner(par->inInset());

	if (bparams.tracking_changes)
		tmp->trackChanges();

	// this is an idea for a more userfriendly layout handling, I will
	// see what the users say

	// layout stays the same with latex-environments
	if (flag) {
		tmp->layout(par->layout());
		tmp->setLabelWidthString(par->params().labelWidthString());
	}

	bool const isempty = (par->allowEmpty() && par->empty());

	if (!isempty && (par->size() > pos || par->empty() || flag == 2)) {
		tmp->layout(par->layout());
		tmp->params().align(par->params().align());
		tmp->setLabelWidthString(par->params().labelWidthString());

		tmp->params().depth(par->params().depth());
		tmp->params().noindent(par->params().noindent());

		// copy everything behind the break-position
		// to the new paragraph

#ifdef WITH_WARNINGS
#warning this seems wrong
#endif
		/* FIXME: if !keepempty, empty() == true, then we reach
		 * here with size() == 0. So pos_end becomes - 1. Why
		 * doesn't this cause problems ???
		 */
		pos_type pos_end = par->size() - 1;
		pos_type i = pos;
		pos_type j = pos;

		for (; i <= pos_end; ++i) {
			Change::Type change = par->lookupChange(i);
			if (moveItem(*par, *tmp, bparams, i, j - pos)) {
				tmp->setChange(j - pos, change);
				++j;
			}
		}

		for (i = pos_end; i >= pos; --i)
			par->eraseIntern(i);
	}

	if (pos)
		return;

	par->params().clear();

	par->layout(bparams.getLyXTextClass().defaultLayout());

	// layout stays the same with latex-environments
	if (flag) {
		par->layout(tmp->layout());
		par->setLabelWidthString(tmp->params().labelWidthString());
		par->params().depth(tmp->params().depth());
	}

	// subtle, but needed to get empty pars working right
	if (bparams.tracking_changes) {
		if (!par->size()) {
			par->cleanChanges();
		} else if (!tmp->size()) {
			tmp->cleanChanges();
		}
	}
}


void breakParagraphConservative(BufferParams const & bparams,
				ParagraphList & paragraphs,
				ParagraphList::iterator par,
				pos_type pos)
{
	// create a new paragraph
	ParagraphList::iterator tmp = paragraphs.insert(boost::next(par),
							Paragraph());
	tmp->makeSameLayout(*par);

	// When can pos > size()?
	// I guess pos == size() is possible.
	if (par->size() > pos) {
		// copy everything behind the break-position to the new
		// paragraph
		pos_type pos_end = par->size() - 1;

		for (pos_type i = pos, j = pos; i <= pos_end; ++i)
			if (moveItem(*par, *tmp, bparams, i, j - pos))
				++j;

		for (pos_type k = pos_end; k >= pos; --k)
			par->erase(k);
	}
}


void mergeParagraph(BufferParams const & bparams,
		    ParagraphList & paragraphs,
		    ParagraphList::iterator par)
{
	ParagraphList::iterator the_next = boost::next(par);

	pos_type pos_end = the_next->size() - 1;
	pos_type pos_insert = par->size();

	// ok, now copy the paragraph
	for (pos_type i = 0, j = 0; i <= pos_end; ++i)
		if (moveItem(*the_next, *par, bparams, i, pos_insert + j))
			++j;

	paragraphs.erase(the_next);
}


ParagraphList::iterator depthHook(ParagraphList::iterator pit,
				  ParagraphList const & plist,
				  Paragraph::depth_type depth)
{
	ParagraphList::iterator newpit = pit;
	ParagraphList::iterator beg = const_cast<ParagraphList&>(plist).begin();

	if (newpit != beg)
		--newpit;

	while (newpit != beg && newpit->getDepth() > depth) {
		--newpit;
	}

	if (newpit->getDepth() > depth)
		return pit;

	return newpit;
}


ParagraphList::iterator outerHook(ParagraphList::iterator pit,
				  ParagraphList const & plist)
{
	if (!pit->getDepth())
		return const_cast<ParagraphList&>(plist).end();
	return depthHook(pit, plist,
			 Paragraph::depth_type(pit->getDepth() - 1));
}


bool isFirstInSequence(ParagraphList::iterator pit,
		       ParagraphList const & plist)
{
	ParagraphList::iterator dhook = depthHook(pit, plist, pit->getDepth());
	return (dhook == pit
		|| dhook->layout() != pit->layout()
		|| dhook->getDepth() != pit->getDepth());
}


int getEndLabel(ParagraphList::iterator p, ParagraphList const & plist)
{
	ParagraphList::iterator pit = p;
	Paragraph::depth_type par_depth = p->getDepth();
	while (pit != const_cast<ParagraphList&>(plist).end()) {
		LyXLayout_ptr const & layout = pit->layout();
		int const endlabeltype = layout->endlabeltype;

		if (endlabeltype != END_LABEL_NO_LABEL) {
			if (boost::next(p) == const_cast<ParagraphList&>(plist).end())
				return endlabeltype;

			Paragraph::depth_type const next_depth = boost::next(p)->getDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth &&
			     layout != boost::next(p)->layout()))
				return endlabeltype;
			break;
		}
		if (par_depth == 0)
			break;
		pit = outerHook(pit, plist);
		if (pit != const_cast<ParagraphList&>(plist).end())
			par_depth = pit->getDepth();
	}
	return END_LABEL_NO_LABEL;
}


namespace {

int readParToken(Buffer const & buf, Paragraph & par, LyXLex & lex,
	string const & token)
{
	static LyXFont font;
	static Change change;

	BufferParams const & bp = buf.params();

	if (token[0] != '\\') {
		string::const_iterator cit = token.begin();
		for (; cit != token.end(); ++cit) {
			par.insertChar(par.size(), (*cit), font, change);
		}
	} else if (token == "\\begin_layout") {
		lex.eatLine();
		string layoutname = lex.getString();

		font = LyXFont(LyXFont::ALL_INHERIT, bp.language);
		change = Change();

		LyXTextClass const & tclass = bp.getLyXTextClass();

		if (layoutname.empty()) {
			layoutname = tclass.defaultLayoutName();
		}

		bool hasLayout = tclass.hasLayout(layoutname);

		if (!hasLayout) {
			lyxerr << "Layout '" << layoutname << "' does not"
			       << " exist in textclass '" << tclass.name()
			       << "'." << endl;
			lyxerr << "Trying to use default layout instead."
			       << endl;
			layoutname = tclass.defaultLayoutName();
		}

		par.layout(bp.getLyXTextClass()[layoutname]);

		// Test whether the layout is obsolete.
		LyXLayout_ptr const & layout = par.layout();
		if (!layout->obsoleted_by().empty())
			par.layout(bp.getLyXTextClass()[layout->obsoleted_by()]);

		par.params().read(lex);

	} else if (token == "\\end_layout") {
		lyxerr << "Solitary \\end_layout in line " << lex.getLineNo() << "\n"
		       << "Missing \\begin_layout?.\n";
	} else if (token == "\\end_inset") {
		lyxerr << "Solitary \\end_inset in line " << lex.getLineNo() << "\n"
		       << "Missing \\begin_inset?.\n";
	} else if (token == "\\begin_inset") {
		InsetOld * inset = readInset(lex, buf);
		if (inset)
			par.insertInset(par.size(), inset, font, change);
		else {
			lex.eatLine();
			string line = lex.getString();
			buf.error(ErrorItem(_("Unknown Inset"), line,
					    par.id(), 0, par.size()));
			return 1;
		}
	} else if (token == "\\family") {
		lex.next();
		font.setLyXFamily(lex.getString());
	} else if (token == "\\series") {
		lex.next();
		font.setLyXSeries(lex.getString());
	} else if (token == "\\shape") {
		lex.next();
		font.setLyXShape(lex.getString());
	} else if (token == "\\size") {
		lex.next();
		font.setLyXSize(lex.getString());
	} else if (token == "\\lang") {
		lex.next();
		string const tok = lex.getString();
		Language const * lang = languages.getLanguage(tok);
		if (lang) {
			font.setLanguage(lang);
		} else {
			font.setLanguage(bp.language);
			lex.printError("Unknown language `$$Token'");
		}
	} else if (token == "\\numeric") {
		lex.next();
		font.setNumber(font.setLyXMisc(lex.getString()));
	} else if (token == "\\emph") {
		lex.next();
		font.setEmph(font.setLyXMisc(lex.getString()));
	} else if (token == "\\bar") {
		lex.next();
		string const tok = lex.getString();

		if (tok == "under")
			font.setUnderbar(LyXFont::ON);
		else if (tok == "no")
			font.setUnderbar(LyXFont::OFF);
		else if (tok == "default")
			font.setUnderbar(LyXFont::INHERIT);
		else
			lex.printError("Unknown bar font flag "
				       "`$$Token'");
	} else if (token == "\\noun") {
		lex.next();
		font.setNoun(font.setLyXMisc(lex.getString()));
	} else if (token == "\\color") {
		lex.next();
		font.setLyXColor(lex.getString());
	} else if (token == "\\InsetSpace" || token == "\\SpecialChar") {

		// Insets don't make sense in a free-spacing context! ---Kayvan
		if (par.isFreeSpacing()) {
			if (token == "\\InsetSpace")
				par.insertChar(par.size(), ' ', font, change);
			else if (lex.isOK()) {
				lex.next();
				string const next_token = lex.getString();
				if (next_token == "\\-")
					par.insertChar(par.size(), '-', font, change);
				else {
					lex.printError("Token `$$Token' "
						       "is in free space "
						       "paragraph layout!");
				}
			}
		} else {
			auto_ptr<InsetOld> inset;
			if (token == "\\SpecialChar" )
				inset.reset(new InsetSpecialChar);
			else
				inset.reset(new InsetSpace);
			inset->read(buf, lex);
			par.insertInset(par.size(), inset.release(),
					font, change);
		}
	} else if (token == "\\i") {
		auto_ptr<InsetOld> inset(new InsetLatexAccent);
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\backslash") {
		par.insertChar(par.size(), '\\', font, change);
	} else if (token == "\\newline") {
		auto_ptr<InsetOld> inset(new InsetNewline);
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\LyXTable") {
		auto_ptr<InsetOld> inset(new InsetTabular(buf));
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\bibitem") {
		InsetCommandParams p("bibitem", "dummy");
		auto_ptr<InsetBibitem> inset(new InsetBibitem(p));
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\hfill") {
		par.insertInset(par.size(), new InsetHFill, font, change);
	} else if (token == "\\lyxline") {
		par.insertInset(par.size(), new InsetLine, font, change);
	} else if (token == "\\newpage") {
		par.insertInset(par.size(), new InsetPagebreak, font, change);
	} else if (token == "\\change_unchanged") {
		// Hack ! Needed for empty paragraphs :/
		// FIXME: is it still ??
		if (!par.size())
			par.cleanChanges();
		change = Change(Change::UNCHANGED);
	} else if (token == "\\change_inserted") {
		lex.nextToken();
		istringstream is(lex.getString());
		int aid;
		lyx::time_type ct;
		is >> aid >> ct;
		change = Change(Change::INSERTED, bp.author_map[aid], ct);
	} else if (token == "\\change_deleted") {
		lex.nextToken();
		istringstream is(lex.getString());
		int aid;
		lyx::time_type ct;
		is >> aid >> ct;
		change = Change(Change::DELETED, bp.author_map[aid], ct);
	} else {
		lex.eatLine();
		string const s = bformat(_("Unknown token: %1$s %2$s\n"),
			token, lex.getString());

		buf.error(ErrorItem(_("Unknown token"), s,
				    par.id(), 0, par.size()));
		return 1;
	}
	return 0;
}

}


int readParagraph(Buffer const & buf, Paragraph & par, LyXLex & lex)
{
	int unknown = 0;

	lex.nextToken();
	string token = lex.getString();

	while (lex.isOK()) {

		unknown += readParToken(buf, par, lex, token);

		lex.nextToken();
		token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_layout") {
			//Ok, paragraph finished
			break;
		}

		lyxerr[Debug::PARSER] << "Handling paragraph token: `"
				      << token << '\'' << endl;
		if (token == "\\begin_layout" || token == "\\end_document"
		    || token == "\\end_inset" || token == "\\begin_deeper"
		    || token == "\\end_deeper") {
			lex.pushToken(token);
			lyxerr << "Paragraph ended in line "
			       << lex.getLineNo() << "\n"
			       << "Missing \\end_layout.\n";
			break;
		}
	}

	return unknown;
}


LyXFont const outerFont(ParagraphList::iterator pit,
			ParagraphList const & plist)
{
	Paragraph::depth_type par_depth = pit->getDepth();
	LyXFont tmpfont(LyXFont::ALL_INHERIT);

	// Resolve against environment font information
	while (pit != const_cast<ParagraphList&>(plist).end() &&
	       par_depth && !tmpfont.resolved()) {
		pit = outerHook(pit, plist);
		if (pit != const_cast<ParagraphList&>(plist).end()) {
			tmpfont.realize(pit->layout()->font);
			par_depth = pit->getDepth();
		}
	}

	return tmpfont;
}


ParagraphList::iterator outerPar(Buffer const & buf, InsetOld const * inset)
{
	ParIterator pit = const_cast<Buffer &>(buf).par_iterator_begin();
	ParIterator end = const_cast<Buffer &>(buf).par_iterator_end();
	for ( ; pit != end; ++pit) {
		LyXText * text;
		// the second '=' below is intentional
		for (int i = 0; (text = inset->getText(i)); ++i)
			if (&text->paragraphs() == &pit.plist())
				return pit.outerPar();

		InsetList::iterator ii = pit->insetlist.begin();
		InsetList::iterator iend = pit->insetlist.end();
		for ( ; ii != iend; ++ii)
			if (ii->inset == inset)
				return pit.outerPar();
	}
	lyxerr << "outerPar: should not happen" << endl;
	BOOST_ASSERT(false);
	return const_cast<Buffer &>(buf).paragraphs().end(); // shut up compiler
}


Paragraph const & ownerPar(Buffer const & buf, InsetOld const * inset)
{
	ParConstIterator pit = buf.par_iterator_begin();
	ParConstIterator end = buf.par_iterator_end();
	for ( ; pit != end; ++pit) {
		LyXText * text;
		// the second '=' below is intentional
		for (int i = 0; (text = inset->getText(i)); ++i)
			if (&text->paragraphs() == &pit.plist())
				return *pit.pit();

		InsetList::const_iterator ii = pit->insetlist.begin();
		InsetList::const_iterator iend = pit->insetlist.end();
		for ( ; ii != iend; ++ii)
			if (ii->inset == inset)
				return *pit.pit();
	}
	lyxerr << "ownerPar: should not happen" << endl;
	BOOST_ASSERT(false);
	return buf.paragraphs().front(); // shut up compiler
}


void getParsInRange(ParagraphList & pl,
		    int ystart, int yend,
		    ParagraphList::iterator & beg,
		    ParagraphList::iterator & end)
{
	ParagraphList::iterator const endpar = pl.end();
	ParagraphList::iterator const begpar = pl.begin();

	BOOST_ASSERT(begpar != endpar);

	beg = endpar;
	for (--beg; beg != begpar && beg->y > ystart; --beg)
		;

	for (end = beg ; end != endpar && end->y <= yend; ++end)
		;
}
