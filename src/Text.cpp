/**
 * \file src/Text.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Dov Feldstern
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 * \author Stefan Schimanski
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Text.h"

#include "Author.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "CompletionList.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "factory.h"
#include "InsetList.h"
#include "Language.h"
#include "Layout.h"
#include "Length.h"
#include "Lexer.h"
#include "lyxfind.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "TextClass.h"
#include "TextMetrics.h"
#include "WordLangTuple.h"
#include "WordList.h"

#include "insets/InsetText.h"
#include "insets/InsetBibitem.h"
#include "insets/InsetCaption.h"
#include "insets/InsetNewline.h"
#include "insets/InsetNewpage.h"
#include "insets/InsetArgument.h"
#include "insets/InsetIPAMacro.h"
#include "insets/InsetSpace.h"
#include "insets/InsetSpecialChar.h"
#include "insets/InsetTabular.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/lyxtime.h"
#include "support/textutils.h"

#include <sstream>


using namespace std;
using namespace lyx::support;

namespace lyx {

using cap::cutSelection;
using cap::pasteParagraphList;

static bool moveItem(Paragraph & fromPar, pos_type fromPos,
	Paragraph & toPar, pos_type toPos, BufferParams const & params)
{
	// Note: moveItem() does not honour change tracking!
	// Therefore, it should only be used for breaking and merging paragraphs

	// We need a copy here because the character at fromPos is going to be erased.
	Font const tmpFont = fromPar.getFontSettings(params, fromPos);
	Change const tmpChange = fromPar.lookupChange(fromPos);

	if (Inset * tmpInset = fromPar.getInset(fromPos)) {
		fromPar.releaseInset(fromPos);
		// The inset is not in fromPar any more.
		if (!toPar.insertInset(toPos, tmpInset, tmpFont, tmpChange)) {
			delete tmpInset;
			return false;
		}
		return true;
	}

	char_type const tmpChar = fromPar.getChar(fromPos);
	fromPar.eraseChar(fromPos, false);
	toPar.insertChar(toPos, tmpChar, tmpFont, tmpChange);
	return true;
}


void breakParagraphConservative(BufferParams const & bparams,
	ParagraphList & pars, pit_type pit, pos_type pos)
{
	// create a new paragraph
	Paragraph & tmp = *pars.insert(lyx::next(pars.begin(), pit + 1),
				       Paragraph());
	Paragraph & par = pars[pit];

	tmp.setInsetOwner(&par.inInset());
	tmp.makeSameLayout(par);

	LASSERT(pos <= par.size(), return);

	if (pos < par.size()) {
		// move everything behind the break position to the new paragraph
		pos_type pos_end = par.size() - 1;

		for (pos_type i = pos, j = 0; i <= pos_end; ++i) {
			if (moveItem(par, pos, tmp, j, bparams)) {
				++j;
			}
		}
		// Move over the end-of-par change information
		tmp.setChange(tmp.size(), par.lookupChange(par.size()));
		par.setChange(par.size(), Change(bparams.track_changes ?
					   Change::INSERTED : Change::UNCHANGED));
	}
}


void mergeParagraph(BufferParams const & bparams,
	ParagraphList & pars, pit_type par_offset)
{
	Paragraph & next = pars[par_offset + 1];
	Paragraph & par = pars[par_offset];

	pos_type pos_end = next.size() - 1;
	pos_type pos_insert = par.size();

	// the imaginary end-of-paragraph character (at par.size()) has to be
	// marked as unmodified. Otherwise, its change is adopted by the first
	// character of the next paragraph.
	if (par.isChanged(par.size())) {
		LYXERR(Debug::CHANGES,
		   "merging par with inserted/deleted end-of-par character");
		par.setChange(par.size(), Change(Change::UNCHANGED));
	}

	Change change = next.lookupChange(next.size());

	// move the content of the second paragraph to the end of the first one
	for (pos_type i = 0, j = pos_insert; i <= pos_end; ++i) {
		if (moveItem(next, 0, par, j, bparams)) {
			++j;
		}
	}

	// move the change of the end-of-paragraph character
	par.setChange(par.size(), change);

	pars.erase(lyx::next(pars.begin(), par_offset + 1));
}


Text::Text(InsetText * owner, bool use_default_layout)
	: owner_(owner)
{
	pars_.push_back(Paragraph());
	Paragraph & par = pars_.back();
	par.setInsetOwner(owner);
	DocumentClass const & dc = owner->buffer().params().documentClass();
	if (use_default_layout)
		par.setDefaultLayout(dc);
	else
		par.setPlainLayout(dc);
}


Text::Text(InsetText * owner, Text const & text)
	: owner_(owner), pars_(text.pars_)
{
	ParagraphList::iterator const end = pars_.end();
	ParagraphList::iterator it = pars_.begin();
	for (; it != end; ++it)
		it->setInsetOwner(owner);
}


pit_type Text::depthHook(pit_type pit, depth_type depth) const
{
	pit_type newpit = pit;

	if (newpit != 0)
		--newpit;

	while (newpit != 0 && pars_[newpit].getDepth() > depth)
		--newpit;

	if (pars_[newpit].getDepth() > depth)
		return pit;

	return newpit;
}


pit_type Text::outerHook(pit_type par_offset) const
{
	Paragraph const & par = pars_[par_offset];

	if (par.getDepth() == 0)
		return pars_.size();
	return depthHook(par_offset, depth_type(par.getDepth() - 1));
}


bool Text::isFirstInSequence(pit_type par_offset) const
{
	Paragraph const & par = pars_[par_offset];

	pit_type dhook_offset = depthHook(par_offset, par.getDepth());

	if (dhook_offset == par_offset)
		return true;

	Paragraph const & dhook = pars_[dhook_offset];

	return dhook.layout() != par.layout()
		|| dhook.getDepth() != par.getDepth();
}


int Text::getTocLevel(pit_type par_offset) const
{
	Paragraph const & par = pars_[par_offset];

	if (par.layout().isEnvironment() && !isFirstInSequence(par_offset))
		return Layout::NOT_IN_TOC;

	return par.layout().toclevel;
}


Font const Text::outerFont(pit_type par_offset) const
{
	depth_type par_depth = pars_[par_offset].getDepth();
	FontInfo tmpfont = inherit_font;
	depth_type prev_par_depth = 0;
	// Resolve against environment font information
	while (par_offset != pit_type(pars_.size())
	       && par_depth != prev_par_depth
	       && par_depth
	       && !tmpfont.resolved()) {
		prev_par_depth = par_depth;
		par_offset = outerHook(par_offset);
		if (par_offset != pit_type(pars_.size())) {
			tmpfont.realize(pars_[par_offset].layout().font);
			par_depth = pars_[par_offset].getDepth();
		}
	}

	return Font(tmpfont);
}


static void acceptOrRejectChanges(ParagraphList & pars,
	BufferParams const & bparams, Text::ChangeOp op)
{
	pit_type pars_size = static_cast<pit_type>(pars.size());

	// first, accept or reject changes within each individual
	// paragraph (do not consider end-of-par)
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		// prevent assertion failure
		if (!pars[pit].empty()) {
			if (op == Text::ACCEPT)
				pars[pit].acceptChanges(0, pars[pit].size());
			else
				pars[pit].rejectChanges(0, pars[pit].size());
		}
	}

	// next, accept or reject imaginary end-of-par characters
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		pos_type pos = pars[pit].size();
		if (pars[pit].isChanged(pos)) {
			// keep the end-of-par char if it is inserted and accepted
			// or when it is deleted and rejected.
			if (pars[pit].isInserted(pos) == (op == Text::ACCEPT)) {
				pars[pit].setChange(pos, Change(Change::UNCHANGED));
			} else {
				if (pit == pars_size - 1) {
					// we cannot remove a par break at the end of the last
					// paragraph; instead, we mark it unchanged
					pars[pit].setChange(pos, Change(Change::UNCHANGED));
				} else {
					mergeParagraph(bparams, pars, pit);
					--pit;
					--pars_size;
				}
			}
		}
	}
}


void acceptChanges(ParagraphList & pars, BufferParams const & bparams)
{
	acceptOrRejectChanges(pars, bparams, Text::ACCEPT);
}


void rejectChanges(ParagraphList & pars, BufferParams const & bparams)
{
	acceptOrRejectChanges(pars, bparams, Text::REJECT);
}


InsetText const & Text::inset() const
{
	return *owner_;
}



void Text::readParToken(Paragraph & par, Lexer & lex,
	string const & token, Font & font, Change & change, ErrorList & errorList)
{
	Buffer * buf = const_cast<Buffer *>(&owner_->buffer());
	BufferParams & bp = buf->params();

	if (token[0] != '\\') {
		docstring dstr = lex.getDocString();
		par.appendString(dstr, font, change);

	} else if (token == "\\begin_layout") {
		lex.eatLine();
		docstring layoutname = lex.getDocString();

		font = Font(inherit_font, bp.language);
		change = Change(Change::UNCHANGED);

		DocumentClass const & tclass = bp.documentClass();

		if (layoutname.empty())
			layoutname = tclass.defaultLayoutName();

		if (owner_->forcePlainLayout()) {
			// in this case only the empty layout is allowed
			layoutname = tclass.plainLayoutName();
		} else if (par.usePlainLayout()) {
			// in this case, default layout maps to empty layout
			if (layoutname == tclass.defaultLayoutName())
				layoutname = tclass.plainLayoutName();
		} else {
			// otherwise, the empty layout maps to the default
			if (layoutname == tclass.plainLayoutName())
				layoutname = tclass.defaultLayoutName();
		}

		// When we apply an unknown layout to a document, we add this layout to the textclass
		// of this document. For example, when you apply class article to a beamer document,
		// all unknown layouts such as frame will be added to document class article so that
		// these layouts can keep their original names.
		bool const added_one = tclass.addLayoutIfNeeded(layoutname);
		if (added_one) {
			// Warn the user.
			docstring const s = bformat(_("Layout `%1$s' was not found."), layoutname);
			errorList.push_back(
				ErrorItem(_("Layout Not Found"), s, par.id(), 0, par.size()));
		}

		par.setLayout(bp.documentClass()[layoutname]);

		// Test whether the layout is obsolete.
		Layout const & layout = par.layout();
		if (!layout.obsoleted_by().empty())
			par.setLayout(bp.documentClass()[layout.obsoleted_by()]);

		par.params().read(lex);

	} else if (token == "\\end_layout") {
		LYXERR0("Solitary \\end_layout in line " << lex.lineNumber() << "\n"
		       << "Missing \\begin_layout ?");
	} else if (token == "\\end_inset") {
		LYXERR0("Solitary \\end_inset in line " << lex.lineNumber() << "\n"
		       << "Missing \\begin_inset ?");
	} else if (token == "\\begin_inset") {
		Inset * inset = readInset(lex, buf);
		if (inset)
			par.insertInset(par.size(), inset, font, change);
		else {
			lex.eatLine();
			docstring line = lex.getDocString();
			errorList.push_back(ErrorItem(_("Unknown Inset"), line,
					    par.id(), 0, par.size()));
		}
	} else if (token == "\\family") {
		lex.next();
		setLyXFamily(lex.getString(), font.fontInfo());
	} else if (token == "\\series") {
		lex.next();
		setLyXSeries(lex.getString(), font.fontInfo());
	} else if (token == "\\shape") {
		lex.next();
		setLyXShape(lex.getString(), font.fontInfo());
	} else if (token == "\\size") {
		lex.next();
		setLyXSize(lex.getString(), font.fontInfo());
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
		font.fontInfo().setNumber(setLyXMisc(lex.getString()));
	} else if (token == "\\emph") {
		lex.next();
		font.fontInfo().setEmph(setLyXMisc(lex.getString()));
	} else if (token == "\\bar") {
		lex.next();
		string const tok = lex.getString();

		if (tok == "under")
			font.fontInfo().setUnderbar(FONT_ON);
		else if (tok == "no")
			font.fontInfo().setUnderbar(FONT_OFF);
		else if (tok == "default")
			font.fontInfo().setUnderbar(FONT_INHERIT);
		else
			lex.printError("Unknown bar font flag "
				       "`$$Token'");
	} else if (token == "\\strikeout") {
		lex.next();
		font.fontInfo().setStrikeout(setLyXMisc(lex.getString()));
	} else if (token == "\\uuline") {
		lex.next();
		font.fontInfo().setUuline(setLyXMisc(lex.getString()));
	} else if (token == "\\uwave") {
		lex.next();
		font.fontInfo().setUwave(setLyXMisc(lex.getString()));
	} else if (token == "\\noun") {
		lex.next();
		font.fontInfo().setNoun(setLyXMisc(lex.getString()));
	} else if (token == "\\color") {
		lex.next();
		setLyXColor(lex.getString(), font.fontInfo());
	} else if (token == "\\SpecialChar" ||
	           (token == "\\SpecialCharNoPassThru" &&
	            !par.layout().pass_thru && !inset().isPassThru())) {
		auto_ptr<Inset> inset;
		inset.reset(new InsetSpecialChar);
		inset->read(lex);
		inset->setBuffer(*buf);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\SpecialCharNoPassThru") {
		lex.next();
		docstring const s = ltrim(lex.getDocString(), "\\");
		par.insert(par.size(), s, font, change);
	} else if (token == "\\IPAChar") {
		auto_ptr<Inset> inset;
		inset.reset(new InsetIPAChar);
		inset->read(lex);
		inset->setBuffer(*buf);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\twohyphens" || token == "\\threehyphens") {
		// Ideally, this should be done by lyx2lyx, but lyx2lyx does not know the
		// running font and does not know anything about layouts (and CopyStyle).
		Layout const & layout(par.layout());
		FontInfo info = font.fontInfo();
		info.realize(layout.resfont);
		if (layout.pass_thru || inset().isPassThru() ||
		    info.family() == TYPEWRITER_FAMILY) {
			if (token == "\\twohyphens")
				par.insert(par.size(), from_ascii("--"), font, change);
			else
				par.insert(par.size(), from_ascii("---"), font, change);
		} else {
			if (token == "\\twohyphens")
				par.insertChar(par.size(), 0x2013, font, change);
			else
				par.insertChar(par.size(), 0x2014, font, change);
		}
	} else if (token == "\\backslash") {
		par.appendChar('\\', font, change);
	} else if (token == "\\LyXTable") {
		auto_ptr<Inset> inset(new InsetTabular(buf));
		inset->read(lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\change_unchanged") {
		change = Change(Change::UNCHANGED);
	} else if (token == "\\change_inserted" || token == "\\change_deleted") {
		lex.eatLine();
		istringstream is(lex.getString());
		int aid;
		time_t ct;
		is >> aid >> ct;
		BufferParams::AuthorMap const & am = bp.author_map_;
		if (am.find(aid) == am.end()) {
			errorList.push_back(ErrorItem(
				_("Change tracking author index missing"),
				bformat(_("A change tracking author information for index "
				          "%1$d is missing. This can happen after a wrong "
				          "merge by a version control system. In this case, "
				          "either fix the merge, or have this information "
				          "missing until the corresponding tracked changes "
				          "are merged or this user edits the file again.\n"),
				        aid),
				par.id(), par.size(), par.size() + 1
				));
			bp.addAuthor(Author(aid));
		}
		if (token == "\\change_inserted")
			change = Change(Change::INSERTED, am.find(aid)->second, ct);
		else
			change = Change(Change::DELETED, am.find(aid)->second, ct);
	} else {
		lex.eatLine();
		errorList.push_back(ErrorItem(_("Unknown token"),
			bformat(_("Unknown token: %1$s %2$s\n"), from_utf8(token),
			lex.getDocString()),
			par.id(), 0, par.size()));
	}
}


void Text::readParagraph(Paragraph & par, Lexer & lex,
	ErrorList & errorList)
{
	lex.nextToken();
	string token = lex.getString();
	Font font;
	Change change(Change::UNCHANGED);

	while (lex.isOK()) {
		readParToken(par, lex, token, font, change, errorList);

		lex.nextToken();
		token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_layout") {
			//Ok, paragraph finished
			break;
		}

		LYXERR(Debug::PARSER, "Handling paragraph token: `" << token << '\'');
		if (token == "\\begin_layout" || token == "\\end_document"
		    || token == "\\end_inset" || token == "\\begin_deeper"
		    || token == "\\end_deeper") {
			lex.pushToken(token);
			lyxerr << "Paragraph ended in line "
			       << lex.lineNumber() << "\n"
			       << "Missing \\end_layout.\n";
			break;
		}
	}
	// Final change goes to paragraph break:
	par.setChange(par.size(), change);

	// Initialize begin_of_body_ on load; redoParagraph maintains
	par.setBeginOfBody();

	// mark paragraph for spell checking on load
	// par.requestSpellCheck();
}


class TextCompletionList : public CompletionList
{
public:
	///
	TextCompletionList(Cursor const & cur, WordList const * list)
		: buffer_(cur.buffer()), list_(list)
	{}
	///
	virtual ~TextCompletionList() {}

	///
	virtual bool sorted() const { return true; }
	///
	virtual size_t size() const
	{
		return list_->size();
	}
	///
	virtual docstring const & data(size_t idx) const
	{
		return list_->word(idx);
	}

private:
	///
	Buffer const * buffer_;
	///
	WordList const * list_;
};


bool Text::empty() const
{
	return pars_.empty() || (pars_.size() == 1 && pars_[0].empty()
		// FIXME: Should we consider the labeled type as empty too?
		&& pars_[0].layout().labeltype == LABEL_NO_LABEL);
}


double Text::spacing(Paragraph const & par) const
{
	if (par.params().spacing().isDefault())
		return owner_->buffer().params().spacing().getValue();
	return par.params().spacing().getValue();
}


/**
 * This breaks a paragraph at the specified position.
 * The new paragraph will:
 * - Decrease depth by one (or change layout to default layout) when
 *    keep_layout == false
 * - keep current depth and layout when keep_layout == true
 */
static void breakParagraph(Text & text, pit_type par_offset, pos_type pos,
		    bool keep_layout)
{
	BufferParams const & bparams = text.inset().buffer().params();
	ParagraphList & pars = text.paragraphs();
	// create a new paragraph, and insert into the list
	ParagraphList::iterator tmp =
		pars.insert(lyx::next(pars.begin(), par_offset + 1),
			    Paragraph());

	Paragraph & par = pars[par_offset];

	// remember to set the inset_owner
	tmp->setInsetOwner(&par.inInset());
	// without doing that we get a crash when typing <Return> at the
	// end of a paragraph
	tmp->setPlainOrDefaultLayout(bparams.documentClass());

	if (keep_layout) {
		tmp->setLayout(par.layout());
		tmp->setLabelWidthString(par.params().labelWidthString());
		tmp->params().depth(par.params().depth());
	} else if (par.params().depth() > 0) {
		Paragraph const & hook = pars[text.outerHook(par_offset)];
		tmp->setLayout(hook.layout());
		// not sure the line below is useful
		tmp->setLabelWidthString(par.params().labelWidthString());
		tmp->params().depth(hook.params().depth());
	}

	bool const isempty = (par.allowEmpty() && par.empty());

	if (!isempty && (par.size() > pos || par.empty())) {
		tmp->setLayout(par.layout());
		tmp->params().align(par.params().align());
		tmp->setLabelWidthString(par.params().labelWidthString());

		tmp->params().depth(par.params().depth());
		tmp->params().noindent(par.params().noindent());

		// move everything behind the break position
		// to the new paragraph

		/* Note: if !keepempty, empty() == true, then we reach
		 * here with size() == 0. So pos_end becomes - 1. This
		 * doesn't cause problems because both loops below
		 * enforce pos <= pos_end and 0 <= pos
		 */
		pos_type pos_end = par.size() - 1;

		for (pos_type i = pos, j = 0; i <= pos_end; ++i) {
			if (moveItem(par, pos, *tmp, j, bparams)) {
				++j;
			}
		}
	}

	// Move over the end-of-par change information
	tmp->setChange(tmp->size(), par.lookupChange(par.size()));
	par.setChange(par.size(), Change(bparams.track_changes ?
					   Change::INSERTED : Change::UNCHANGED));

	if (pos) {
		// Make sure that we keep the language when
		// breaking paragraph.
		if (tmp->empty()) {
			Font changed = tmp->getFirstFontSettings(bparams);
			Font const & old = par.getFontSettings(bparams, par.size());
			changed.setLanguage(old.language());
			tmp->setFont(0, changed);
		}

		return;
	}

	if (!isempty) {
		bool const soa = par.params().startOfAppendix();
		par.params().clear();
		// do not lose start of appendix marker (bug 4212)
		par.params().startOfAppendix(soa);
		par.setPlainOrDefaultLayout(bparams.documentClass());
	}

	if (keep_layout) {
		par.setLayout(tmp->layout());
		par.setLabelWidthString(tmp->params().labelWidthString());
		par.params().depth(tmp->params().depth());
	}
}


void Text::breakParagraph(Cursor & cur, bool inverse_logic)
{
	LBUFERR(this == cur.text());

	Paragraph & cpar = cur.paragraph();
	pit_type cpit = cur.pit();

	DocumentClass const & tclass = cur.buffer()->params().documentClass();
	Layout const & layout = cpar.layout();

	if (cur.lastpos() == 0 && !cpar.allowEmpty()) {
		if (changeDepthAllowed(cur, DEC_DEPTH))
			changeDepth(cur, DEC_DEPTH);
		else {
			docstring const & lay = cur.paragraph().usePlainLayout()
			    ? tclass.plainLayoutName() : tclass.defaultLayoutName();
			setLayout(cur, lay);
		}
		return;
	}

	cur.recordUndo();

	// Always break behind a space
	// It is better to erase the space (Dekel)
	if (cur.pos() != cur.lastpos() && cpar.isLineSeparator(cur.pos()))
		cpar.eraseChar(cur.pos(), cur.buffer()->params().track_changes);

	// What should the layout for the new paragraph be?
	bool keep_layout = layout.isEnvironment()
		|| (layout.isParagraph() && layout.parbreak_is_newline);
	if (inverse_logic)
		keep_layout = !keep_layout;

	// We need to remember this before we break the paragraph, because
	// that invalidates the layout variable
	bool sensitive = layout.labeltype == LABEL_SENSITIVE;

	// we need to set this before we insert the paragraph.
	bool const isempty = cpar.allowEmpty() && cpar.empty();

	lyx::breakParagraph(*this, cpit, cur.pos(), keep_layout);

	// After this, neither paragraph contains any rows!

	cpit = cur.pit();
	pit_type next_par = cpit + 1;

	// well this is the caption hack since one caption is really enough
	if (sensitive) {
		if (cur.pos() == 0)
			// set to standard-layout
		//FIXME Check if this should be plainLayout() in some cases
			pars_[cpit].applyLayout(tclass.defaultLayout());
		else
			// set to standard-layout
			//FIXME Check if this should be plainLayout() in some cases
			pars_[next_par].applyLayout(tclass.defaultLayout());
	}

	while (!pars_[next_par].empty() && pars_[next_par].isNewline(0)) {
		if (!pars_[next_par].eraseChar(0, cur.buffer()->params().track_changes))
			break; // the character couldn't be deleted physically due to change tracking
	}

	// A singlePar update is not enough in this case.
	cur.screenUpdateFlags(Update::Force);
	cur.forceBufferUpdate();

	// This check is necessary. Otherwise the new empty paragraph will
	// be deleted automatically. And it is more friendly for the user!
	if (cur.pos() != 0 || isempty)
		setCursor(cur, cur.pit() + 1, 0);
	else
		setCursor(cur, cur.pit(), 0);
}


// needed to insert the selection
void Text::insertStringAsLines(Cursor & cur, docstring const & str,
		Font const & font)
{
	BufferParams const & bparams = owner_->buffer().params();
	pit_type pit = cur.pit();
	pos_type pos = cur.pos();

	// insert the string, don't insert doublespace
	bool space_inserted = true;
	for (docstring::const_iterator cit = str.begin();
	    cit != str.end(); ++cit) {
		Paragraph & par = pars_[pit];
		if (*cit == '\n') {
			if (inset().allowMultiPar() && (!par.empty() || par.allowEmpty())) {
				lyx::breakParagraph(*this, pit, pos,
					par.layout().isEnvironment());
				++pit;
				pos = 0;
				space_inserted = true;
			} else {
				continue;
			}
		// do not insert consecutive spaces if !free_spacing
		} else if ((*cit == ' ' || *cit == '\t') &&
			   space_inserted && !par.isFreeSpacing()) {
			continue;
		} else if (*cit == '\t') {
			if (!par.isFreeSpacing()) {
				// tabs are like spaces here
				par.insertChar(pos, ' ', font, bparams.track_changes);
				++pos;
				space_inserted = true;
			} else {
				par.insertChar(pos, *cit, font, bparams.track_changes);
				++pos;
				space_inserted = true;
			}
		} else if (!isPrintable(*cit)) {
			// Ignore unprintables
			continue;
		} else {
			// just insert the character
			par.insertChar(pos, *cit, font, bparams.track_changes);
			++pos;
			space_inserted = (*cit == ' ');
		}
	}
	setCursor(cur, pit, pos);
}


// turn double CR to single CR, others are converted into one
// blank. Then insertStringAsLines is called
void Text::insertStringAsParagraphs(Cursor & cur, docstring const & str,
		Font const & font)
{
	docstring linestr = str;
	bool newline_inserted = false;

	for (string::size_type i = 0, siz = linestr.size(); i < siz; ++i) {
		if (linestr[i] == '\n') {
			if (newline_inserted) {
				// we know that \r will be ignored by
				// insertStringAsLines. Of course, it is a dirty
				// trick, but it works...
				linestr[i - 1] = '\r';
				linestr[i] = '\n';
			} else {
				linestr[i] = ' ';
				newline_inserted = true;
			}
		} else if (isPrintable(linestr[i])) {
			newline_inserted = false;
		}
	}
	insertStringAsLines(cur, linestr, font);
}


// insert a character, moves all the following breaks in the
// same Paragraph one to the right and make a rebreak
void Text::insertChar(Cursor & cur, char_type c)
{
	LBUFERR(this == cur.text());

	cur.recordUndo(INSERT_UNDO);

	TextMetrics const & tm = cur.bv().textMetrics(this);
	Buffer const & buffer = *cur.buffer();
	Paragraph & par = cur.paragraph();
	// try to remove this
	pit_type const pit = cur.pit();

	bool const freeSpacing = par.layout().free_spacing ||
		par.isFreeSpacing();

	if (lyxrc.auto_number) {
		static docstring const number_operators = from_ascii("+-/*");
		static docstring const number_unary_operators = from_ascii("+-");
		static docstring const number_separators = from_ascii(".,:");

		if (cur.current_font.fontInfo().number() == FONT_ON) {
			if (!isDigitASCII(c) && !contains(number_operators, c) &&
			    !(contains(number_separators, c) &&
			      cur.pos() != 0 &&
			      cur.pos() != cur.lastpos() &&
			      tm.displayFont(pit, cur.pos()).fontInfo().number() == FONT_ON &&
			      tm.displayFont(pit, cur.pos() - 1).fontInfo().number() == FONT_ON)
			   )
				number(cur); // Set current_font.number to OFF
		} else if (isDigitASCII(c) &&
			   cur.real_current_font.isVisibleRightToLeft()) {
			number(cur); // Set current_font.number to ON

			if (cur.pos() != 0) {
				char_type const c = par.getChar(cur.pos() - 1);
				if (contains(number_unary_operators, c) &&
				    (cur.pos() == 1
				     || par.isSeparator(cur.pos() - 2)
				     || par.isEnvSeparator(cur.pos() - 2)
				     || par.isNewline(cur.pos() - 2))
				  ) {
					setCharFont(pit, cur.pos() - 1, cur.current_font,
						tm.font_);
				} else if (contains(number_separators, c)
				     && cur.pos() >= 2
				     && tm.displayFont(pit, cur.pos() - 2).fontInfo().number() == FONT_ON) {
					setCharFont(pit, cur.pos() - 1, cur.current_font,
						tm.font_);
				}
			}
		}
	}

	// In Bidi text, we want spaces to be treated in a special way: spaces
	// which are between words in different languages should get the
	// paragraph's language; otherwise, spaces should keep the language
	// they were originally typed in. This is only in effect while typing;
	// after the text is already typed in, the user can always go back and
	// explicitly set the language of a space as desired. But 99.9% of the
	// time, what we're doing here is what the user actually meant.
	//
	// The following cases are the ones in which the language of the space
	// should be changed to match that of the containing paragraph. In the
	// depictions, lowercase is LTR, uppercase is RTL, underscore (_)
	// represents a space, pipe (|) represents the cursor position (so the
	// character before it is the one just typed in). The different cases
	// are depicted logically (not visually), from left to right:
	//
	// 1. A_a|
	// 2. a_A|
	//
	// Theoretically, there are other situations that we should, perhaps, deal
	// with (e.g.: a|_A, A|_a). In practice, though, there really isn't any
	// point (to understand why, just try to create this situation...).

	if ((cur.pos() >= 2) && (par.isLineSeparator(cur.pos() - 1))) {
		// get font in front and behind the space in question. But do NOT
		// use getFont(cur.pos()) because the character c is not inserted yet
		Font const pre_space_font  = tm.displayFont(cur.pit(), cur.pos() - 2);
		Font const & post_space_font = cur.real_current_font;
		bool pre_space_rtl  = pre_space_font.isVisibleRightToLeft();
		bool post_space_rtl = post_space_font.isVisibleRightToLeft();

		if (pre_space_rtl != post_space_rtl) {
			// Set the space's language to match the language of the
			// adjacent character whose direction is the paragraph's
			// direction; don't touch other properties of the font
			Language const * lang =
				(pre_space_rtl == par.isRTL(buffer.params())) ?
				pre_space_font.language() : post_space_font.language();

			Font space_font = tm.displayFont(cur.pit(), cur.pos() - 1);
			space_font.setLanguage(lang);
			par.setFont(cur.pos() - 1, space_font);
		}
	}

	// Next check, if there will be two blanks together or a blank at
	// the beginning of a paragraph.
	// I decided to handle blanks like normal characters, the main
	// difference are the special checks when calculating the row.fill
	// (blank does not count at the end of a row) and the check here

	// When the free-spacing option is set for the current layout,
	// disable the double-space checking
	if (!freeSpacing && isLineSeparatorChar(c)) {
		if (cur.pos() == 0) {
			cur.message(_(
					"You cannot insert a space at the "
					"beginning of a paragraph. Please read the Tutorial."));
			return;
		}
		// LASSERT: Is it safe to continue here?
		LASSERT(cur.pos() > 0, /**/);
		if ((par.isLineSeparator(cur.pos() - 1) || par.isNewline(cur.pos() - 1))
				&& !par.isDeleted(cur.pos() - 1)) {
			cur.message(_(
					"You cannot type two spaces this way. "
					"Please read the Tutorial."));
			return;
		}
	}

	// Prevent to insert uncodable characters in verbatim and ERT
	// (workaround for bug 9012)
	// Don't do it for listings inset, since InsetListings::latex() tries
	// to switch to a usable encoding which works in many cases (bug 9102).
	if (cur.paragraph().isPassThru() && owner_->lyxCode() != LISTINGS_CODE &&
	    cur.current_font.language()) {
		Encoding const * e = cur.current_font.language()->encoding();
		if (!e->encodable(c)) {
			cur.message(_("Character is uncodable in verbatim paragraphs."));
			return;
		}
	}

	pos_type pos = cur.pos();
	if (!cur.paragraph().isPassThru() && owner_->lyxCode() != IPA_CODE &&
	    cur.real_current_font.fontInfo().family() != TYPEWRITER_FAMILY &&
	    c == '-' && pos > 0) {
		if (par.getChar(pos - 1) == '-') {
			// convert "--" to endash
			par.eraseChar(pos - 1, cur.buffer()->params().track_changes);
			c = 0x2013;
			pos--;
		} else if (par.getChar(pos - 1) == 0x2013) {
			// convert "---" to emdash
			par.eraseChar(pos - 1, cur.buffer()->params().track_changes);
			c = 0x2014;
			pos--;
		} else if (par.getChar(pos - 1) == 0x2014) {
			// convert "----" to "-"
			par.eraseChar(pos - 1, cur.buffer()->params().track_changes);
			c = '-';
			pos--;
		}
	}

	par.insertChar(pos, c, cur.current_font,
		cur.buffer()->params().track_changes);
	cur.checkBufferStructure();

//		cur.screenUpdateFlags(Update::Force);
	bool boundary = cur.boundary()
		|| tm.isRTLBoundary(cur.pit(), pos + 1);
	setCursor(cur, cur.pit(), pos + 1, false, boundary);
	charInserted(cur);
}


void Text::charInserted(Cursor & cur)
{
	Paragraph & par = cur.paragraph();

	// register word if a non-letter was entered
	if (cur.pos() > 1
	    && !par.isWordSeparator(cur.pos() - 2)
	    && par.isWordSeparator(cur.pos() - 1)) {
		// get the word in front of cursor
		LBUFERR(this == cur.text());
		cur.paragraph().updateWords();
	}
}


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.

bool Text::cursorForwardOneWord(Cursor & cur)
{
	LBUFERR(this == cur.text());

	pos_type const lastpos = cur.lastpos();
	pit_type pit = cur.pit();
	pos_type pos = cur.pos();
	Paragraph const & par = cur.paragraph();

	// Paragraph boundary is a word boundary
	if (pos == lastpos || (pos + 1 == lastpos && par.isEnvSeparator(pos))) {
		if (pit != cur.lastpit())
			return setCursor(cur, pit + 1, 0);
		else
			return false;
	}

	if (lyxrc.mac_like_cursor_movement) {
		// Skip through trailing punctuation and spaces.
		while (pos != lastpos && (par.isChar(pos) || par.isSpace(pos)))
			++pos;

		// Skip over either a non-char inset or a full word
		if (pos != lastpos && par.isWordSeparator(pos))
			++pos;
		else while (pos != lastpos && !par.isWordSeparator(pos))
			     ++pos;
	} else {
		LASSERT(pos < lastpos, return false); // see above
		if (!par.isWordSeparator(pos))
			while (pos != lastpos && !par.isWordSeparator(pos))
				++pos;
		else if (par.isChar(pos))
			while (pos != lastpos && par.isChar(pos))
				++pos;
		else if (!par.isSpace(pos)) // non-char inset
			++pos;

		// Skip over white space
		while (pos != lastpos && par.isSpace(pos))
			     ++pos;
	}

	// Don't skip a separator inset at the end of a paragraph
	if (pos == lastpos && pos && par.isEnvSeparator(pos - 1))
		--pos;

	return setCursor(cur, pit, pos);
}


bool Text::cursorBackwardOneWord(Cursor & cur)
{
	LBUFERR(this == cur.text());

	pit_type pit = cur.pit();
	pos_type pos = cur.pos();
	Paragraph & par = cur.paragraph();

	// Paragraph boundary is a word boundary
	if (pos == 0 && pit != 0) {
		Paragraph & prevpar = getPar(pit - 1);
		pos = prevpar.size();
		// Don't stop after an environment separator
		if (pos && prevpar.isEnvSeparator(pos - 1))
			--pos;
		return setCursor(cur, pit - 1, pos);
	}

	if (lyxrc.mac_like_cursor_movement) {
		// Skip through punctuation and spaces.
		while (pos != 0 && (par.isChar(pos - 1) || par.isSpace(pos - 1)))
			--pos;

		// Skip over either a non-char inset or a full word
		if (pos != 0 && par.isWordSeparator(pos - 1) && !par.isChar(pos - 1))
			--pos;
		else while (pos != 0 && !par.isWordSeparator(pos - 1))
			     --pos;
	} else {
		// Skip over white space
		while (pos != 0 && par.isSpace(pos - 1))
			     --pos;

		if (pos != 0 && !par.isWordSeparator(pos - 1))
			while (pos != 0 && !par.isWordSeparator(pos - 1))
				--pos;
		else if (pos != 0 && par.isChar(pos - 1))
			while (pos != 0 && par.isChar(pos - 1))
				--pos;
		else if (pos != 0 && !par.isSpace(pos - 1)) // non-char inset
			--pos;
	}

	return setCursor(cur, pit, pos);
}


bool Text::cursorVisLeftOneWord(Cursor & cur)
{
	LBUFERR(this == cur.text());

	pos_type left_pos, right_pos;

	Cursor temp_cur = cur;

	// always try to move at least once...
	while (temp_cur.posVisLeft(true /* skip_inset */)) {

		// collect some information about current cursor position
		temp_cur.getSurroundingPos(left_pos, right_pos);
		bool left_is_letter =
			(left_pos > -1 ? !temp_cur.paragraph().isWordSeparator(left_pos) : false);
		bool right_is_letter =
			(right_pos > -1 ? !temp_cur.paragraph().isWordSeparator(right_pos) : false);

		// if we're not at a letter/non-letter boundary, continue moving
		if (left_is_letter == right_is_letter)
			continue;

		// we should stop when we have an LTR word on our right or an RTL word
		// on our left
		if ((left_is_letter && temp_cur.paragraph().getFontSettings(
				temp_cur.buffer()->params(), left_pos).isRightToLeft())
			|| (right_is_letter && !temp_cur.paragraph().getFontSettings(
				temp_cur.buffer()->params(), right_pos).isRightToLeft()))
			break;
	}

	return setCursor(cur, temp_cur.pit(), temp_cur.pos(),
					 true, temp_cur.boundary());
}


bool Text::cursorVisRightOneWord(Cursor & cur)
{
	LBUFERR(this == cur.text());

	pos_type left_pos, right_pos;

	Cursor temp_cur = cur;

	// always try to move at least once...
	while (temp_cur.posVisRight(true /* skip_inset */)) {

		// collect some information about current cursor position
		temp_cur.getSurroundingPos(left_pos, right_pos);
		bool left_is_letter =
			(left_pos > -1 ? !temp_cur.paragraph().isWordSeparator(left_pos) : false);
		bool right_is_letter =
			(right_pos > -1 ? !temp_cur.paragraph().isWordSeparator(right_pos) : false);

		// if we're not at a letter/non-letter boundary, continue moving
		if (left_is_letter == right_is_letter)
			continue;

		// we should stop when we have an LTR word on our right or an RTL word
		// on our left
		if ((left_is_letter && temp_cur.paragraph().getFontSettings(
				temp_cur.buffer()->params(),
				left_pos).isRightToLeft())
			|| (right_is_letter && !temp_cur.paragraph().getFontSettings(
				temp_cur.buffer()->params(),
				right_pos).isRightToLeft()))
			break;
	}

	return setCursor(cur, temp_cur.pit(), temp_cur.pos(),
					 true, temp_cur.boundary());
}


void Text::selectWord(Cursor & cur, word_location loc)
{
	LBUFERR(this == cur.text());
	CursorSlice from = cur.top();
	CursorSlice to;
	getWord(from, to, loc);
	if (cur.top() != from)
		setCursor(cur, from.pit(), from.pos());
	if (to == from)
		return;
	if (!cur.selection())
		cur.resetAnchor();
	setCursor(cur, to.pit(), to.pos());
	cur.setSelection();
	cur.setWordSelection(true);
}


void Text::selectAll(Cursor & cur)
{
	LBUFERR(this == cur.text());
	if (cur.lastpos() == 0 && cur.lastpit() == 0)
		return;
	// If the cursor is at the beginning, make sure the cursor ends there
	if (cur.pit() == 0 && cur.pos() == 0) {
		setCursor(cur, cur.lastpit(), getPar(cur.lastpit()).size());
		cur.resetAnchor();
		setCursor(cur, 0, 0);
	} else {
		setCursor(cur, 0, 0);
		cur.resetAnchor();
		setCursor(cur, cur.lastpit(), getPar(cur.lastpit()).size());
	}
	cur.setSelection();
}


// Select the word currently under the cursor when no
// selection is currently set
bool Text::selectWordWhenUnderCursor(Cursor & cur, word_location loc)
{
	LBUFERR(this == cur.text());
	if (cur.selection())
		return false;
	selectWord(cur, loc);
	return cur.selection();
}


void Text::acceptOrRejectChanges(Cursor & cur, ChangeOp op)
{
	LBUFERR(this == cur.text());

	if (!cur.selection()) {
		if (!selectChange(cur))
			return;
	}

	cur.recordUndoSelection();

	pit_type begPit = cur.selectionBegin().pit();
	pit_type endPit = cur.selectionEnd().pit();

	pos_type begPos = cur.selectionBegin().pos();
	pos_type endPos = cur.selectionEnd().pos();

	// keep selection info, because endPos becomes invalid after the first loop
	bool endsBeforeEndOfPar = (endPos < pars_[endPit].size());

	// first, accept/reject changes within each individual paragraph (do not consider end-of-par)
	for (pit_type pit = begPit; pit <= endPit; ++pit) {
		pos_type parSize = pars_[pit].size();

		// ignore empty paragraphs; otherwise, an assertion will fail for
		// acceptChanges(bparams, 0, 0) or rejectChanges(bparams, 0, 0)
		if (parSize == 0)
			continue;

		// do not consider first paragraph if the cursor starts at pos size()
		if (pit == begPit && begPos == parSize)
			continue;

		// do not consider last paragraph if the cursor ends at pos 0
		if (pit == endPit && endPos == 0)
			break; // last iteration anyway

		pos_type left  = (pit == begPit ? begPos : 0);
		pos_type right = (pit == endPit ? endPos : parSize);

		if (left == right)
			// there is no change here
			continue;

		if (op == ACCEPT) {
			pars_[pit].acceptChanges(left, right);
		} else {
			pars_[pit].rejectChanges(left, right);
		}
	}

	// next, accept/reject imaginary end-of-par characters

	for (pit_type pit = begPit; pit <= endPit; ++pit) {
		pos_type pos = pars_[pit].size();

		// skip if the selection ends before the end-of-par
		if (pit == endPit && endsBeforeEndOfPar)
			break; // last iteration anyway

		// skip if this is not the last paragraph of the document
		// note: the user should be able to accept/reject the par break of the last par!
		if (pit == endPit && pit + 1 != int(pars_.size()))
			break; // last iteration anway

		if (op == ACCEPT) {
			if (pars_[pit].isInserted(pos)) {
				pars_[pit].setChange(pos, Change(Change::UNCHANGED));
			} else if (pars_[pit].isDeleted(pos)) {
				if (pit + 1 == int(pars_.size())) {
					// we cannot remove a par break at the end of the last paragraph;
					// instead, we mark it unchanged
					pars_[pit].setChange(pos, Change(Change::UNCHANGED));
				} else {
					mergeParagraph(cur.buffer()->params(), pars_, pit);
					--endPit;
					--pit;
				}
			}
		} else {
			if (pars_[pit].isDeleted(pos)) {
				pars_[pit].setChange(pos, Change(Change::UNCHANGED));
			} else if (pars_[pit].isInserted(pos)) {
				if (pit + 1 == int(pars_.size())) {
					// we mark the par break at the end of the last paragraph unchanged
					pars_[pit].setChange(pos, Change(Change::UNCHANGED));
				} else {
					mergeParagraph(cur.buffer()->params(), pars_, pit);
					--endPit;
					--pit;
				}
			}
		}
	}

	// finally, invoke the DEPM
	deleteEmptyParagraphMechanism(begPit, endPit, cur.buffer()->params().track_changes);

	cur.finishUndo();
	cur.clearSelection();
	setCursorIntern(cur, begPit, begPos);
	cur.screenUpdateFlags(Update::Force);
	cur.forceBufferUpdate();
}


void Text::acceptChanges()
{
	BufferParams const & bparams = owner_->buffer().params();
	lyx::acceptChanges(pars_, bparams);
	deleteEmptyParagraphMechanism(0, pars_.size() - 1, bparams.track_changes);
}


void Text::rejectChanges()
{
	BufferParams const & bparams = owner_->buffer().params();
	pit_type pars_size = static_cast<pit_type>(pars_.size());

	// first, reject changes within each individual paragraph
	// (do not consider end-of-par)
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		if (!pars_[pit].empty())   // prevent assertion failure
			pars_[pit].rejectChanges(0, pars_[pit].size());
	}

	// next, reject imaginary end-of-par characters
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		pos_type pos = pars_[pit].size();

		if (pars_[pit].isDeleted(pos)) {
			pars_[pit].setChange(pos, Change(Change::UNCHANGED));
		} else if (pars_[pit].isInserted(pos)) {
			if (pit == pars_size - 1) {
				// we mark the par break at the end of the last
				// paragraph unchanged
				pars_[pit].setChange(pos, Change(Change::UNCHANGED));
			} else {
				mergeParagraph(bparams, pars_, pit);
				--pit;
				--pars_size;
			}
		}
	}

	// finally, invoke the DEPM
	deleteEmptyParagraphMechanism(0, pars_size - 1, bparams.track_changes);
}


void Text::deleteWordForward(Cursor & cur)
{
	LBUFERR(this == cur.text());
	if (cur.lastpos() == 0)
		cursorForward(cur);
	else {
		cur.resetAnchor();
		cur.selection(true);
		cursorForwardOneWord(cur);
		cur.setSelection();
		cutSelection(cur, true, false);
		cur.checkBufferStructure();
	}
}


void Text::deleteWordBackward(Cursor & cur)
{
	LBUFERR(this == cur.text());
	if (cur.lastpos() == 0)
		cursorBackward(cur);
	else {
		cur.resetAnchor();
		cur.selection(true);
		cursorBackwardOneWord(cur);
		cur.setSelection();
		cutSelection(cur, true, false);
		cur.checkBufferStructure();
	}
}


// Kill to end of line.
void Text::changeCase(Cursor & cur, TextCase action, bool partial)
{
	LBUFERR(this == cur.text());
	CursorSlice from;
	CursorSlice to;

	bool const gotsel = cur.selection();
	if (gotsel) {
		from = cur.selBegin();
		to = cur.selEnd();
	} else {
		from = cur.top();
		getWord(from, to, partial ? PARTIAL_WORD : WHOLE_WORD);
		cursorForwardOneWord(cur);
	}

	cur.recordUndoSelection();

	pit_type begPit = from.pit();
	pit_type endPit = to.pit();

	pos_type begPos = from.pos();
	pos_type endPos = to.pos();

	pos_type right = 0; // needed after the for loop

	for (pit_type pit = begPit; pit <= endPit; ++pit) {
		Paragraph & par = pars_[pit];
		pos_type const pos = (pit == begPit ? begPos : 0);
		right = (pit == endPit ? endPos : par.size());
		par.changeCase(cur.buffer()->params(), pos, right, action);
	}

	// the selection may have changed due to logically-only deleted chars
	if (gotsel) {
		setCursor(cur, begPit, begPos);
		cur.resetAnchor();
		setCursor(cur, endPit, right);
		cur.setSelection();
	} else
		setCursor(cur, endPit, right);

	cur.checkBufferStructure();
}


bool Text::handleBibitems(Cursor & cur)
{
	if (cur.paragraph().layout().labeltype != LABEL_BIBLIO)
		return false;

	if (cur.pos() != 0)
		return false;

	BufferParams const & bufparams = cur.buffer()->params();
	Paragraph const & par = cur.paragraph();
	Cursor prevcur = cur;
	if (cur.pit() > 0) {
		--prevcur.pit();
		prevcur.pos() = prevcur.lastpos();
	}
	Paragraph const & prevpar = prevcur.paragraph();

	// if a bibitem is deleted, merge with previous paragraph
	// if this is a bibliography item as well
	if (cur.pit() > 0 && par.layout() == prevpar.layout()) {
		cur.recordUndo(prevcur.pit());
		mergeParagraph(bufparams, cur.text()->paragraphs(),
							prevcur.pit());
		cur.forceBufferUpdate();
		setCursorIntern(cur, prevcur.pit(), prevcur.pos());
		cur.screenUpdateFlags(Update::Force);
		return true;
	}

	// otherwise reset to default
	cur.paragraph().setPlainOrDefaultLayout(bufparams.documentClass());
	return true;
}


bool Text::erase(Cursor & cur)
{
	LASSERT(this == cur.text(), return false);
	bool needsUpdate = false;
	Paragraph & par = cur.paragraph();

	if (cur.pos() != cur.lastpos()) {
		// this is the code for a normal delete, not pasting
		// any paragraphs
		cur.recordUndo(DELETE_UNDO);
		bool const was_inset = cur.paragraph().isInset(cur.pos());
		if(!par.eraseChar(cur.pos(), cur.buffer()->params().track_changes))
			// the character has been logically deleted only => skip it
			cur.top().forwardPos();

		if (was_inset)
			cur.forceBufferUpdate();
		else
			cur.checkBufferStructure();
		needsUpdate = true;
	} else {
		if (cur.pit() == cur.lastpit())
			return dissolveInset(cur);

		if (!par.isMergedOnEndOfParDeletion(cur.buffer()->params().track_changes)) {
			par.setChange(cur.pos(), Change(Change::DELETED));
			cur.forwardPos();
			needsUpdate = true;
		} else {
			setCursorIntern(cur, cur.pit() + 1, 0);
			needsUpdate = backspacePos0(cur);
		}
	}

	needsUpdate |= handleBibitems(cur);

	if (needsUpdate) {
		// Make sure the cursor is correct. Is this really needed?
		// No, not really... at least not here!
		cur.top().setPitPos(cur.pit(), cur.pos());
		cur.checkBufferStructure();
	}

	return needsUpdate;
}


bool Text::backspacePos0(Cursor & cur)
{
	LBUFERR(this == cur.text());
	if (cur.pit() == 0)
		return false;

	bool needsUpdate = false;

	BufferParams const & bufparams = cur.buffer()->params();
	DocumentClass const & tclass = bufparams.documentClass();
	ParagraphList & plist = cur.text()->paragraphs();
	Paragraph const & par = cur.paragraph();
	Cursor prevcur = cur;
	--prevcur.pit();
	prevcur.pos() = prevcur.lastpos();
	Paragraph const & prevpar = prevcur.paragraph();

	// is it an empty paragraph?
	if (cur.lastpos() == 0
	    || (cur.lastpos() == 1 && par.isSeparator(0))) {
		cur.recordUndo(prevcur.pit());
		plist.erase(lyx::next(plist.begin(), cur.pit()));
		needsUpdate = true;
	}
	// is previous par empty?
	else if (prevcur.lastpos() == 0
		 || (prevcur.lastpos() == 1 && prevpar.isSeparator(0))) {
		cur.recordUndo(prevcur.pit());
		plist.erase(lyx::next(plist.begin(), prevcur.pit()));
		needsUpdate = true;
	}
	// Pasting is not allowed, if the paragraphs have different
	// layouts. I think it is a real bug of all other
	// word processors to allow it. It confuses the user.
	// Correction: Pasting is always allowed with standard-layout
	// or the empty layout.
	else if (par.layout() == prevpar.layout()
		 || tclass.isDefaultLayout(par.layout())
		 || tclass.isPlainLayout(par.layout())) {
		cur.recordUndo(prevcur.pit());
		mergeParagraph(bufparams, plist, prevcur.pit());
		needsUpdate = true;
	}

	if (needsUpdate) {
		cur.forceBufferUpdate();
		setCursorIntern(cur, prevcur.pit(), prevcur.pos());
	}

	return needsUpdate;
}


bool Text::backspace(Cursor & cur)
{
	LBUFERR(this == cur.text());
	bool needsUpdate = false;
	if (cur.pos() == 0) {
		if (cur.pit() == 0)
			return dissolveInset(cur);

		Cursor prev_cur = cur;
		--prev_cur.pit();

		if (!prev_cur.paragraph().isMergedOnEndOfParDeletion(cur.buffer()->params().track_changes)) {
			cur.recordUndo(prev_cur.pit(), prev_cur.pit());
			prev_cur.paragraph().setChange(prev_cur.lastpos(), Change(Change::DELETED));
			setCursorIntern(cur, prev_cur.pit(), prev_cur.lastpos());
			return true;
		}
		// The cursor is at the beginning of a paragraph, so
		// the backspace will collapse two paragraphs into one.
		needsUpdate = backspacePos0(cur);

	} else {
		// this is the code for a normal backspace, not pasting
		// any paragraphs
		cur.recordUndo(DELETE_UNDO);
		// We used to do cursorBackwardIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorBackwardIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		setCursorIntern(cur, cur.pit(), cur.pos() - 1,
				false, cur.boundary());
		bool const was_inset = cur.paragraph().isInset(cur.pos());
		cur.paragraph().eraseChar(cur.pos(), cur.buffer()->params().track_changes);
		if (was_inset)
			cur.forceBufferUpdate();
		else
			cur.checkBufferStructure();
	}

	if (cur.pos() == cur.lastpos())
		cur.setCurrentFont();

	needsUpdate |= handleBibitems(cur);

	// A singlePar update is not enough in this case.
//		cur.screenUpdateFlags(Update::Force);
	cur.top().setPitPos(cur.pit(), cur.pos());

	return needsUpdate;
}


bool Text::dissolveInset(Cursor & cur)
{
	LASSERT(this == cur.text(), return false);

	if (isMainText() || cur.inset().nargs() != 1)
		return false;

	cur.recordUndoInset();
	cur.setMark(false);
	cur.selHandle(false);
	// save position
	pos_type spos = cur.pos();
	pit_type spit = cur.pit();
	ParagraphList plist;
	if (cur.lastpit() != 0 || cur.lastpos() != 0)
		plist = paragraphs();
	cur.popBackward();
	// store cursor offset
	if (spit == 0)
		spos += cur.pos();
	spit += cur.pit();
	Buffer & b = *cur.buffer();
	cur.paragraph().eraseChar(cur.pos(), b.params().track_changes);

	if (!plist.empty()) {
		// see bug 7319
		// we clear the cache so that we won't get conflicts with labels
		// that get pasted into the buffer. we should update this before
		// its being empty matters. if not (i.e., if we encounter bugs),
		// then this should instead be:
		//	  cur.buffer().updateBuffer();
		// but we'll try the cheaper solution here.
		cur.buffer()->clearReferenceCache();

		// ERT paragraphs have the Language latex_language.
		// This is invalid outside of ERT, so we need to
		// change it to the buffer language.
		ParagraphList::iterator it = plist.begin();
		ParagraphList::iterator it_end = plist.end();
		for (; it != it_end; ++it)
			it->changeLanguage(b.params(), latex_language, b.language());

		pasteParagraphList(cur, plist, b.params().documentClassPtr(),
				   b.errorList("Paste"));
		// restore position
		cur.pit() = min(cur.lastpit(), spit);
		cur.pos() = min(cur.lastpos(), spos);
	}

	cur.forceBufferUpdate();

	// Ensure the current language is set correctly (bug 6292)
	cur.text()->setCursor(cur, cur.pit(), cur.pos());
	cur.clearSelection();
	cur.resetAnchor();
	return true;
}


void Text::getWord(CursorSlice & from, CursorSlice & to,
	word_location const loc) const
{
	to = from;
	pars_[to.pit()].locateWord(from.pos(), to.pos(), loc);
}


void Text::write(ostream & os) const
{
	Buffer const & buf = owner_->buffer();
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	depth_type dth = 0;
	for (; pit != end; ++pit)
		pit->write(os, buf.params(), dth);

	// Close begin_deeper
	for(; dth > 0; --dth)
		os << "\n\\end_deeper";
}


bool Text::read(Lexer & lex,
		ErrorList & errorList, InsetText * insetPtr)
{
	Buffer const & buf = owner_->buffer();
	depth_type depth = 0;
	bool res = true;

	while (lex.isOK()) {
		lex.nextToken();
		string const token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_inset")
			break;

		if (token == "\\end_body")
			continue;

		if (token == "\\begin_body")
			continue;

		if (token == "\\end_document") {
			res = false;
			break;
		}

		if (token == "\\begin_layout") {
			lex.pushToken(token);

			Paragraph par;
			par.setInsetOwner(insetPtr);
			par.params().depth(depth);
			par.setFont(0, Font(inherit_font, buf.params().language));
			pars_.push_back(par);
			readParagraph(pars_.back(), lex, errorList);

			// register the words in the global word list
			pars_.back().updateWords();
		} else if (token == "\\begin_deeper") {
			++depth;
		} else if (token == "\\end_deeper") {
			if (!depth)
				lex.printError("\\end_deeper: " "depth is already null");
			else
				--depth;
		} else {
			LYXERR0("Handling unknown body token: `" << token << '\'');
		}
	}

	// avoid a crash on weird documents (bug 4859)
	if (pars_.empty()) {
		Paragraph par;
		par.setInsetOwner(insetPtr);
		par.params().depth(depth);
		par.setFont(0, Font(inherit_font,
				    buf.params().language));
		par.setPlainOrDefaultLayout(buf.params().documentClass());
		pars_.push_back(par);
	}

	return res;
}


// Returns the current font and depth as a message.
docstring Text::currentState(Cursor const & cur) const
{
	LBUFERR(this == cur.text());
	Buffer & buf = *cur.buffer();
	Paragraph const & par = cur.paragraph();
	odocstringstream os;

	if (buf.params().track_changes)
		os << _("[Change Tracking] ");

	Change change = par.lookupChange(cur.pos());

	if (change.changed()) {
		docstring const author =
			buf.params().authors().get(change.author).nameAndEmail();
		docstring const date = formatted_datetime(change.changetime);
		os << bformat(_("Changed by %1$s[[author]] on %2$s[[date]]. "),
		              author, date);
	}

	// I think we should only show changes from the default
	// font. (Asger)
	// No, from the document font (MV)
	Font font = cur.real_current_font;
	font.fontInfo().reduce(buf.params().getFont().fontInfo());

	os << bformat(_("Font: %1$s"), font.stateText(&buf.params()));

	// The paragraph depth
	int depth = cur.paragraph().getDepth();
	if (depth > 0)
		os << bformat(_(", Depth: %1$d"), depth);

	// The paragraph spacing, but only if different from
	// buffer spacing.
	Spacing const & spacing = par.params().spacing();
	if (!spacing.isDefault()) {
		os << _(", Spacing: ");
		switch (spacing.getSpace()) {
		case Spacing::Single:
			os << _("Single");
			break;
		case Spacing::Onehalf:
			os << _("OneHalf");
			break;
		case Spacing::Double:
			os << _("Double");
			break;
		case Spacing::Other:
			os << _("Other (") << from_ascii(spacing.getValueAsString()) << ')';
			break;
		case Spacing::Default:
			// should never happen, do nothing
			break;
		}
	}

#ifdef DEVEL_VERSION
	os << _(", Inset: ") << &cur.inset();
	os << _(", Paragraph: ") << cur.pit();
	os << _(", Id: ") << par.id();
	os << _(", Position: ") << cur.pos();
	// FIXME: Why is the check for par.size() needed?
	// We are called with cur.pos() == par.size() quite often.
	if (!par.empty() && cur.pos() < par.size()) {
		// Force output of code point, not character
		size_t const c = par.getChar(cur.pos());
		os << _(", Char: 0x") << hex << c;
	}
	os << _(", Boundary: ") << cur.boundary();
//	Row & row = cur.textRow();
//	os << bformat(_(", Row b:%1$d e:%2$d"), row.pos(), row.endpos());
#endif
	return os.str();
}


docstring Text::getPossibleLabel(Cursor const & cur) const
{
	pit_type pit = cur.pit();

	Layout const * layout = &(pars_[pit].layout());

	docstring text;
	docstring par_text = pars_[pit].asString();

	// The return string of math matrices might contain linebreaks
	par_text = subst(par_text, '\n', '-');
	int const numwords = 3;
	for (int i = 0; i < numwords; ++i) {
		if (par_text.empty())
			break;
		docstring head;
		par_text = split(par_text, head, ' ');
		// Is it legal to use spaces in labels ?
		if (i > 0)
			text += '-';
		text += head;
	}

	// Make sure it isn't too long
	unsigned int const max_label_length = 32;
	if (text.size() > max_label_length)
		text.resize(max_label_length);

	// Will contain the label prefix.
	docstring name;

	// For section, subsection, etc...
	if (layout->latextype == LATEX_PARAGRAPH && pit != 0) {
		Layout const * layout2 = &(pars_[pit - 1].layout());
		if (layout2->latextype != LATEX_PARAGRAPH) {
			--pit;
			layout = layout2;
		}
	}
	if (layout->latextype != LATEX_PARAGRAPH)
		name = layout->refprefix;

	// For captions, we just take the caption type
	Inset * caption_inset = cur.innerInsetOfType(CAPTION_CODE);
	if (caption_inset) {
		string const & ftype = static_cast<InsetCaption *>(caption_inset)->floattype();
		FloatList const & fl = cur.buffer()->params().documentClass().floats();
		if (fl.typeExist(ftype)) {
			Floating const & flt = fl.getType(ftype);
			name = from_utf8(flt.refPrefix());
		}
		if (name.empty())
			name = from_utf8(ftype.substr(0,3));
	}

	// If none of the above worked, see if the inset knows.
	if (name.empty()) {
		InsetLayout const & il = cur.inset().getLayout();
		name = il.refprefix();
	}

	if (!name.empty())
		text = name + ':' + text;

	// We need a unique label
	docstring label = text;
	int i = 1;
	while (cur.buffer()->insetLabel(label)) {
			label = text + '-' + convert<docstring>(i);
			++i;
		}

	return label;
}


docstring Text::asString(int options) const
{
	return asString(0, pars_.size(), options);
}


docstring Text::asString(pit_type beg, pit_type end, int options) const
{
	size_t i = size_t(beg);
	docstring str = pars_[i].asString(options);
	for (++i; i != size_t(end); ++i) {
		str += '\n';
		str += pars_[i].asString(options);
	}
	return str;
}


void Text::shortenForOutliner(docstring & str, size_t const maxlen)
{
	support::truncateWithEllipsis(str, maxlen);
	docstring::iterator it = str.begin();
	docstring::iterator end = str.end();
	for (; it != end; ++it)
		if ((*it) == L'\n' || (*it) == L'\t')
			(*it) = L' ';	
}


void Text::forOutliner(docstring & os, size_t const maxlen,
					   bool const shorten) const
{
	size_t tmplen = shorten ? maxlen + 1 : maxlen;
	for (size_t i = 0; i != pars_.size() && os.length() < tmplen; ++i)
		pars_[i].forOutliner(os, tmplen, false);
	if (shorten)
		shortenForOutliner(os, maxlen);
}


void Text::charsTranspose(Cursor & cur)
{
	LBUFERR(this == cur.text());

	pos_type pos = cur.pos();

	// If cursor is at beginning or end of paragraph, do nothing.
	if (pos == cur.lastpos() || pos == 0)
		return;

	Paragraph & par = cur.paragraph();

	// Get the positions of the characters to be transposed.
	pos_type pos1 = pos - 1;
	pos_type pos2 = pos;

	// In change tracking mode, ignore deleted characters.
	while (pos2 < cur.lastpos() && par.isDeleted(pos2))
		++pos2;
	if (pos2 == cur.lastpos())
		return;

	while (pos1 >= 0 && par.isDeleted(pos1))
		--pos1;
	if (pos1 < 0)
		return;

	// Don't do anything if one of the "characters" is not regular text.
	if (par.isInset(pos1) || par.isInset(pos2))
		return;

	// Store the characters to be transposed (including font information).
	char_type const char1 = par.getChar(pos1);
	Font const font1 =
		par.getFontSettings(cur.buffer()->params(), pos1);

	char_type const char2 = par.getChar(pos2);
	Font const font2 =
		par.getFontSettings(cur.buffer()->params(), pos2);

	// And finally, we are ready to perform the transposition.
	// Track the changes if Change Tracking is enabled.
	bool const trackChanges = cur.buffer()->params().track_changes;

	cur.recordUndo();

	par.eraseChar(pos2, trackChanges);
	par.eraseChar(pos1, trackChanges);
	par.insertChar(pos1, char2, font2, trackChanges);
	par.insertChar(pos2, char1, font1, trackChanges);

	cur.checkBufferStructure();

	// After the transposition, move cursor to after the transposition.
	setCursor(cur, cur.pit(), pos2);
	cur.forwardPos();
}


DocIterator Text::macrocontextPosition() const
{
	return macrocontext_position_;
}


void Text::setMacrocontextPosition(DocIterator const & pos)
{
	macrocontext_position_ = pos;
}


docstring Text::previousWord(CursorSlice const & sl) const
{
	CursorSlice from = sl;
	CursorSlice to = sl;
	getWord(from, to, PREVIOUS_WORD);
	if (sl == from || to == from)
		return docstring();

	Paragraph const & par = sl.paragraph();
	return par.asString(from.pos(), to.pos());
}


bool Text::completionSupported(Cursor const & cur) const
{
	Paragraph const & par = cur.paragraph();
	return cur.pos() > 0
		&& (cur.pos() >= par.size() || par.isWordSeparator(cur.pos()))
		&& !par.isWordSeparator(cur.pos() - 1);
}


CompletionList const * Text::createCompletionList(Cursor const & cur) const
{
	WordList const * list = theWordList(cur.getFont().language()->lang());
	return new TextCompletionList(cur, list);
}


bool Text::insertCompletion(Cursor & cur, docstring const & s, bool /*finished*/)
{
	LBUFERR(cur.bv().cursor() == cur);
	cur.insert(s);
	cur.bv().cursor() = cur;
	if (!(cur.result().screenUpdate() & Update::Force))
		cur.screenUpdateFlags(cur.result().screenUpdate() | Update::SinglePar);
	return true;
}


docstring Text::completionPrefix(Cursor const & cur) const
{
	return previousWord(cur.top());
}

} // namespace lyx
