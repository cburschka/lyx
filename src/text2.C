/**
 * \file text2.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Allan Rae
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxtext.h"

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "Bullet.h"
#include "counters.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "dispatchresult.h"
#include "errorlist.h"
#include "Floating.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "gettext.h"
#include "language.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxrow_funcs.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "undo.h"
#include "vspace.h"

#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"

#include "insets/insetbibitem.h"
#include "insets/insetenv.h"
#include "insets/insetfloat.h"
#include "insets/insetwrap.h"

#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/tostr.h"
#include "support/std_sstream.h"

#include <boost/tuple/tuple.hpp>

using lyx::pos_type;
using lyx::paroffset_type;
using lyx::support::bformat;

using std::endl;
using std::ostringstream;
using std::string;


LyXText::LyXText(BufferView * bv, InsetText * inset, bool ininset,
	  ParagraphList & paragraphs)
	: height(0), width(0),
	  inset_owner(inset), the_locking_inset(0), bv_owner(bv),
	  in_inset_(ininset), paragraphs_(&paragraphs),
		cache_pos_(-1)
{
}


void LyXText::init(BufferView * bview)
{
	bv_owner = bview;

	ParagraphList::iterator const beg = ownerParagraphs().begin();
	ParagraphList::iterator const end = ownerParagraphs().end();
	for (ParagraphList::iterator pit = beg; pit != end; ++pit)
		pit->rows.clear();

	width = 0;
	height = 0;
	cache_pos_ = -1;

	current_font = getFont(beg, 0);

	redoParagraphs(beg, end);
	setCursorIntern(0, 0);
	selection.cursor = cursor;

	updateCounters();
}


// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as Paragraph::getFont() in paragraph.C.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
LyXFont LyXText::getFont(ParagraphList::iterator pit, pos_type pos) const
{
	BOOST_ASSERT(pos >= 0);

	LyXLayout_ptr const & layout = pit->layout();
#warning broken?
	BufferParams const & params = bv()->buffer()->params();
	pos_type const body_pos = pit->beginningOfBody();

	// We specialize the 95% common case:
	if (!pit->getDepth()) {
		LyXFont f = pit->getFontSettings(params, pos);
		if (pit->inInset())
			pit->inInset()->getDrawFont(f);
		if (layout->labeltype == LABEL_MANUAL && pos < body_pos)
			return f.realize(layout->reslabelfont);
		else
			return f.realize(layout->resfont);
	}

	// The uncommon case need not be optimized as much
	LyXFont layoutfont;
	if (pos < body_pos)
		layoutfont = layout->labelfont;
	else
		layoutfont = layout->font;

	LyXFont font = pit->getFontSettings(params, pos);
	font.realize(layoutfont);

	if (pit->inInset())
		pit->inInset()->getDrawFont(font);

	// Realize with the fonts of lesser depth.
	//font.realize(outerFont(pit, ownerParagraphs()));
	font.realize(defaultfont_);

	return font;
}


LyXFont LyXText::getLayoutFont(ParagraphList::iterator pit) const
{
	LyXLayout_ptr const & layout = pit->layout();

	if (!pit->getDepth())
		return layout->resfont;

	LyXFont font = layout->font;
	// Realize with the fonts of lesser depth.
	//font.realize(outerFont(pit, ownerParagraphs()));
	font.realize(defaultfont_);

	return font;
}


LyXFont LyXText::getLabelFont(ParagraphList::iterator pit) const
{
	LyXLayout_ptr const & layout = pit->layout();

	if (!pit->getDepth())
		return layout->reslabelfont;

	LyXFont font = layout->labelfont;
	// Realize with the fonts of lesser depth.
	font.realize(outerFont(pit, ownerParagraphs()));
	font.realize(defaultfont_);

	return font;
}


void LyXText::setCharFont(ParagraphList::iterator pit,
			  pos_type pos, LyXFont const & fnt,
			  bool toggleall)
{
	BufferParams const & params = bv()->buffer()->params();
	LyXFont font = getFont(pit, pos);
	font.update(fnt, params.language, toggleall);
	// Let the insets convert their font
	if (pit->isInset(pos)) {
		InsetOld * inset = pit->getInset(pos);
		if (isEditableInset(inset)) {
			static_cast<UpdatableInset *>(inset)
				->setFont(bv(), fnt, toggleall, true);
		}
	}

	// Plug through to version below:
	setCharFont(pit, pos, font);
}


void LyXText::setCharFont(
	ParagraphList::iterator pit, pos_type pos, LyXFont const & fnt)
{
	LyXFont font = fnt;
	LyXLayout_ptr const & layout = pit->layout();

	// Get concrete layout font to reduce against
	LyXFont layoutfont;

	if (pos < pit->beginningOfBody())
		layoutfont = layout->labelfont;
	else
		layoutfont = layout->font;

	// Realize against environment font information
	if (pit->getDepth()) {
		ParagraphList::iterator tp = pit;
		while (!layoutfont.resolved() &&
		       tp != ownerParagraphs().end() &&
		       tp->getDepth()) {
			tp = outerHook(tp, ownerParagraphs());
			if (tp != ownerParagraphs().end())
				layoutfont.realize(tp->layout()->font);
		}
	}

	layoutfont.realize(defaultfont_);

	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	pit->setFont(pos, font);
}


InsetOld * LyXText::getInset() const
{
	ParagraphList::iterator pit = cursorPar();
	pos_type const pos = cursor.pos();

	if (pos < pit->size() && pit->isInset(pos)) {
		return pit->getInset(pos);
	}
	return 0;
}


void LyXText::toggleInset()
{
	InsetOld * inset = getInset();
	// is there an editable inset at cursor position?
	if (!isEditableInset(inset)) {
		// No, try to see if we are inside a collapsable inset
		if (inset_owner && inset_owner->owner()
		    && inset_owner->owner()->isOpen()) {
			bv()->unlockInset(inset_owner->owner());
			inset_owner->owner()->close(bv());
			bv()->getLyXText()->cursorRight(bv());
		}
		return;
	}
	//bv()->owner()->message(inset->editMessage());

	// do we want to keep this?? (JMarc)
	if (!isHighlyEditableInset(inset))
		recUndo(cursor.par());

	if (inset->isOpen())
		inset->close(bv());
	else
		inset->open(bv());

	bv()->updateInset(inset);
}


/* used in setlayout */
// Asger is not sure we want to do this...
void LyXText::makeFontEntriesLayoutSpecific(BufferParams const & params,
					    Paragraph & par)
{
	LyXLayout_ptr const & layout = par.layout();
	pos_type const psize = par.size();

	LyXFont layoutfont;
	for (pos_type pos = 0; pos < psize; ++pos) {
		if (pos < par.beginningOfBody())
			layoutfont = layout->labelfont;
		else
			layoutfont = layout->font;

		LyXFont tmpfont = par.getFontSettings(params, pos);
		tmpfont.reduce(layoutfont);
		par.setFont(pos, tmpfont);
	}
}


ParagraphList::iterator
LyXText::setLayout(LyXCursor & cur, LyXCursor & sstart_cur,
		   LyXCursor & send_cur,
		   string const & layout)
{
	ParagraphList::iterator endpit = boost::next(getPar(send_cur));
	ParagraphList::iterator undoendpit = endpit;
	ParagraphList::iterator pars_end = ownerParagraphs().end();

	if (endpit != pars_end && endpit->getDepth()) {
		while (endpit != pars_end && endpit->getDepth()) {
			++endpit;
			undoendpit = endpit;
		}
	} else if (endpit != pars_end) {
		// because of parindents etc.
		++endpit;
	}

	recUndo(sstart_cur.par(), parOffset(undoendpit) - 1);

	// ok we have a selection. This is always between sstart_cur
	// and sel_end cursor
	cur = sstart_cur;
	ParagraphList::iterator pit = getPar(sstart_cur);
	ParagraphList::iterator epit = boost::next(getPar(send_cur));

	BufferParams const & bufparams = bv()->buffer()->params();
	LyXLayout_ptr const & lyxlayout =
		bufparams.getLyXTextClass()[layout];

	do {
		pit->applyLayout(lyxlayout);
		makeFontEntriesLayoutSpecific(bufparams, *pit);
		pit->params().spaceTop(lyxlayout->fill_top ?
					 VSpace(VSpace::VFILL)
					 : VSpace(VSpace::NONE));
		pit->params().spaceBottom(lyxlayout->fill_bottom ?
					    VSpace(VSpace::VFILL)
					    : VSpace(VSpace::NONE));
		if (lyxlayout->margintype == MARGIN_MANUAL)
			pit->setLabelWidthString(lyxlayout->labelstring());
		cur.par(std::distance(ownerParagraphs().begin(), pit));
		++pit;
	} while (pit != epit);

	return endpit;
}


// set layout over selection and make a total rebreak of those paragraphs
void LyXText::setLayout(string const & layout)
{
	LyXCursor tmpcursor = cursor;  // store the current cursor

	// if there is no selection just set the layout
	// of the current paragraph
	if (!selection.set()) {
		selection.start = cursor;  // dummy selection
		selection.end = cursor;
	}

	// special handling of new environment insets
	BufferParams const & params = bv()->buffer()->params();
	LyXLayout_ptr const & lyxlayout = params.getLyXTextClass()[layout];
	if (lyxlayout->is_environment) {
		// move everything in a new environment inset
		lyxerr << "setting layout " << layout << endl;
		bv()->owner()->dispatch(FuncRequest(LFUN_HOME));
		bv()->owner()->dispatch(FuncRequest(LFUN_ENDSEL));
		bv()->owner()->dispatch(FuncRequest(LFUN_CUT));
		InsetOld * inset = new InsetEnvironment(params, layout);
		if (bv()->insertInset(inset)) {
			//inset->edit(bv());
			//bv()->owner()->dispatch(FuncRequest(LFUN_PASTE));
		}
		else
			delete inset;
		return;
	}

	ParagraphList::iterator endpit = setLayout(cursor, selection.start,
						   selection.end, layout);
	redoParagraphs(getPar(selection.start), endpit);

	// we have to reset the selection, because the
	// geometry could have changed
	setCursor(selection.start.par(), selection.start.pos(), false);
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos(), false);
	updateCounters();
	clearSelection();
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos(), true);
}


bool LyXText::changeDepth(bv_funcs::DEPTH_CHANGE type, bool test_only)
{
	ParagraphList::iterator pit = cursorPar();
	ParagraphList::iterator end = cursorPar();
	ParagraphList::iterator start = pit;

	if (selection.set()) {
		pit = getPar(selection.start);
		end = getPar(selection.end);
		start = pit;
	}

	ParagraphList::iterator pastend = boost::next(end);

	if (!test_only)
		recUndo(parOffset(start), parOffset(end));

	bool changed = false;

	int prev_after_depth = 0;
#warning parlist ... could be nicer ?
	if (start != ownerParagraphs().begin()) {
		prev_after_depth = boost::prior(start)->getMaxDepthAfter();
	}

	while (true) {
		int const depth = pit->params().depth();
		if (type == bv_funcs::INC_DEPTH) {
			if (depth < prev_after_depth
			    && pit->layout()->labeltype != LABEL_BIBLIO) {
				changed = true;
				if (!test_only)
					pit->params().depth(depth + 1);
			}
		} else if (depth) {
			changed = true;
			if (!test_only)
				pit->params().depth(depth - 1);
		}

		prev_after_depth = pit->getMaxDepthAfter();

		if (pit == end) {
			break;
		}

		++pit;
	}

	if (test_only)
		return changed;

	redoParagraphs(start, pastend);

	// We need to actually move the text->cursor. I don't
	// understand why ...
	LyXCursor tmpcursor = cursor;

	// we have to reset the visual selection because the
	// geometry could have changed
	if (selection.set()) {
		setCursor(selection.start.par(), selection.start.pos());
		selection.cursor = cursor;
		setCursor(selection.end.par(), selection.end.pos());
	}

	// this handles the counter labels, and also fixes up
	// depth values for follow-on (child) paragraphs
	updateCounters();

	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos());

	return changed;
}


// set font over selection and make a total rebreak of those paragraphs
void LyXText::setFont(LyXFont const & font, bool toggleall)
{
	// if there is no selection just set the current_font
	if (!selection.set()) {
		// Determine basis font
		LyXFont layoutfont;
		if (cursor.pos() < cursorPar()->beginningOfBody()) {
			layoutfont = getLabelFont(cursorPar());
		} else {
			layoutfont = getLayoutFont(cursorPar());
		}
		// Update current font
		real_current_font.update(font,
					 bv()->buffer()->params().language,
					 toggleall);

		// Reduce to implicit settings
		current_font = real_current_font;
		current_font.reduce(layoutfont);
		// And resolve it completely
		real_current_font.realize(layoutfont);

		return;
	}

	LyXCursor tmpcursor = cursor; // store the current cursor

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor

	recUndo(selection.start.par(), selection.end.par());
	freezeUndo();
	cursor = selection.start;
	while (cursor.par() != selection.end.par() ||
	       cursor.pos() < selection.end.pos())
	{
		if (cursor.pos() < cursorPar()->size()) {
			// an open footnote should behave like a closed one
			setCharFont(cursorPar(), cursor.pos(), font, toggleall);
			cursor.pos(cursor.pos() + 1);
		} else {
			cursor.pos(0);
			cursor.par(cursor.par() + 1);
		}
	}
	unFreezeUndo();

	redoParagraph(getPar(selection.start));

	// we have to reset the selection, because the
	// geometry could have changed, but we keep
	// it for user convenience
	setCursor(selection.start.par(), selection.start.pos());
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos());
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos(), true,
		  tmpcursor.boundary());
}


// important for the screen


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.

// need the selection cursor:
void LyXText::setSelection()
{
	TextCursor::setSelection();
}



void LyXText::clearSelection()
{
	TextCursor::clearSelection();

	// reset this in the bv_owner!
	if (bv_owner && bv_owner->text)
		bv_owner->text->xsel_cache.set(false);
}


void LyXText::cursorHome()
{
	ParagraphList::iterator cpit = cursorPar();
	setCursor(cpit, cpit->getRow(cursor.pos())->pos());
}


void LyXText::cursorEnd()
{
	ParagraphList::iterator cpit = cursorPar();
	pos_type end = cpit->getRow(cursor.pos())->endpos();
	/* if not on the last row of the par, put the cursor before
	  the final space */
	setCursor(cpit, end == cpit->size() ? end : end - 1);
}


void LyXText::cursorTop()
{
	setCursor(ownerParagraphs().begin(), 0);
}


void LyXText::cursorBottom()
{
	ParagraphList::iterator lastpit =
		boost::prior(ownerParagraphs().end());
	setCursor(lastpit, lastpit->size());
}


void LyXText::toggleFree(LyXFont const & font, bool toggleall)
{
	// If the mask is completely neutral, tell user
	if (font == LyXFont(LyXFont::ALL_IGNORE)) {
		// Could only happen with user style
		bv()->owner()->message(_("No font change defined. Use Character under the Layout menu to define font change."));
		return;
	}

	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	LyXCursor resetCursor = cursor;
	bool implicitSelection =
		font.language() == ignore_language
		&& font.number() == LyXFont::IGNORE
		&& selectWordWhenUnderCursor(lyx::WHOLE_WORD_STRICT);

	// Set font
	setFont(font, toggleall);

	// Implicit selections are cleared afterwards
	//and cursor is set to the original position.
	if (implicitSelection) {
		clearSelection();
		cursor = resetCursor;
		setCursor(cursorPar(), cursor.pos());
		selection.cursor = cursor;
	}
}


string LyXText::getStringToIndex()
{
	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	LyXCursor const reset_cursor = cursor;
	bool const implicitSelection =
		selectWordWhenUnderCursor(lyx::PREVIOUS_WORD);

	string idxstring;
	if (!selection.set())
		bv()->owner()->message(_("Nothing to index!"));
	else if (selection.start.par() != selection.end.par())
		bv()->owner()->message(_("Cannot index more than one paragraph!"));
	else
		idxstring = selectionAsString(*bv()->buffer(), false);

	// Reset cursors to their original position.
	cursor = reset_cursor;
	setCursor(cursorPar(), cursor.pos());
	selection.cursor = cursor;

	// Clear the implicit selection.
	if (implicitSelection)
		clearSelection();

	return idxstring;
}


// the DTP switches for paragraphs. LyX will store them in the first
// physical paragraph. When a paragraph is broken, the top settings rest,
// the bottom settings are given to the new one. So I can make sure,
// they do not duplicate themself and you cannnot make dirty things with
// them!

void LyXText::setParagraph(
			   VSpace const & space_top,
			   VSpace const & space_bottom,
			   Spacing const & spacing,
			   LyXAlignment align,
			   string const & labelwidthstring,
			   bool noindent)
{
	LyXCursor tmpcursor = cursor;
	if (!selection.set()) {
		selection.start = cursor;
		selection.end = cursor;
	}

	// make sure that the depth behind the selection are restored, too
	ParagraphList::iterator endpit = boost::next(getPar(selection.end));
	ParagraphList::iterator undoendpit = endpit;
	ParagraphList::iterator pars_end = ownerParagraphs().end();

	if (endpit != pars_end && endpit->getDepth()) {
		while (endpit != pars_end && endpit->getDepth()) {
			++endpit;
			undoendpit = endpit;
		}
	} else if (endpit != pars_end) {
		// because of parindents etc.
		++endpit;
	}

	recUndo(selection.start.par(), parOffset(undoendpit) - 1);


	int tmppit = selection.end.par();

	while (tmppit != selection.start.par() - 1) {
		setCursor(tmppit, 0);

		ParagraphList::iterator const pit = cursorPar();
		ParagraphParameters & params = pit->params();
		params.spaceTop(space_top);
		params.spaceBottom(space_bottom);
		params.spacing(spacing);

		// does the layout allow the new alignment?
		LyXLayout_ptr const & layout = pit->layout();

		if (align == LYX_ALIGN_LAYOUT)
			align = layout->align;
		if (align & layout->alignpossible) {
			if (align == layout->align)
				params.align(LYX_ALIGN_LAYOUT);
			else
				params.align(align);
		}
		pit->setLabelWidthString(labelwidthstring);
		params.noindent(noindent);
		--tmppit;
	}

	redoParagraphs(getPar(selection.start), endpit);

	clearSelection();
	setCursor(selection.start.par(), selection.start.pos());
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos());
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos());
	if (inset_owner)
		bv()->updateInset(inset_owner);
}


namespace {

string expandLabel(LyXTextClass const & textclass,
	LyXLayout_ptr const & layout, bool appendix)
{
	string fmt = appendix ?
		layout->labelstring_appendix() : layout->labelstring();

	// handle 'inherited level parts' in 'fmt',
	// i.e. the stuff between '@' in   '@Section@.\arabic{subsection}'
	size_t const i = fmt.find('@', 0);
	if (i != string::npos) {
		size_t const j = fmt.find('@', i + 1);
		if (j != string::npos) {
			string parent(fmt, i + 1, j - i - 1);
			string label = expandLabel(textclass, textclass[parent], appendix);
			fmt = string(fmt, 0, i) + label + string(fmt, j + 1, string::npos);
		}
	}

	return textclass.counters().counterLabel(fmt);
}


void incrementItemDepth(ParagraphList::iterator pit,
			ParagraphList::iterator first_pit)
{
	int const cur_labeltype = pit->layout()->labeltype;

	if (cur_labeltype != LABEL_ENUMERATE &&
	    cur_labeltype != LABEL_ITEMIZE)
		return;

	int const cur_depth = pit->getDepth();

	ParagraphList::iterator prev_pit = boost::prior(pit);
	while (true) {
		int const prev_depth = prev_pit->getDepth();
		int const prev_labeltype = prev_pit->layout()->labeltype;
		if (prev_depth == 0 && cur_depth > 0) {
			if (prev_labeltype == cur_labeltype) {
				pit->itemdepth = prev_pit->itemdepth + 1;
			}
			break;
		} else if (prev_depth < cur_depth) {
			if (prev_labeltype == cur_labeltype) {
				pit->itemdepth = prev_pit->itemdepth + 1;
				break;
			}
		} else if (prev_depth == cur_depth) {
			if (prev_labeltype == cur_labeltype) {
				pit->itemdepth = prev_pit->itemdepth;
				break;
			}
		}
		if (prev_pit == first_pit)
			break;

		--prev_pit;
	}
}


void resetEnumCounterIfNeeded(ParagraphList::iterator pit,
			      ParagraphList::iterator firstpit,
			      Counters & counters)
{
	if (pit == firstpit)
		return;

	int const cur_depth = pit->getDepth();
	ParagraphList::iterator prev_pit = boost::prior(pit);
	while (true) {
		int const prev_depth = prev_pit->getDepth();
		int const prev_labeltype = prev_pit->layout()->labeltype;
		if (prev_depth <= cur_depth) {
			if (prev_labeltype != LABEL_ENUMERATE) {
				switch (pit->itemdepth) {
				case 0:
					counters.reset("enumi");
				case 1:
					counters.reset("enumii");
				case 2:
					counters.reset("enumiii");
				case 3:
					counters.reset("enumiv");
				}
			}
			break;
		}

		if (prev_pit == firstpit)
			break;

		--prev_pit;
	}
}

} // anon namespace


// set the counter of a paragraph. This includes the labels
void LyXText::setCounter(Buffer const & buf, ParagraphList::iterator pit)
{
	BufferParams const & bufparams = buf.params();
	LyXTextClass const & textclass = bufparams.getLyXTextClass();
	LyXLayout_ptr const & layout = pit->layout();
	ParagraphList::iterator first_pit = ownerParagraphs().begin();
	Counters & counters = textclass.counters();

	// Always reset
	pit->itemdepth = 0;

	if (pit == first_pit) {
		pit->params().appendix(pit->params().startOfAppendix());
	} else {
		pit->params().appendix(boost::prior(pit)->params().appendix());
		if (!pit->params().appendix() &&
		    pit->params().startOfAppendix()) {
			pit->params().appendix(true);
			textclass.counters().reset();
		}

		// Maybe we have to increment the item depth.
		incrementItemDepth(pit, first_pit);
	}

	// erase what was there before
	pit->params().labelString(string());

	if (layout->margintype == MARGIN_MANUAL) {
		if (pit->params().labelWidthString().empty())
			pit->setLabelWidthString(layout->labelstring());
	} else {
		pit->setLabelWidthString(string());
	}

	// is it a layout that has an automatic label?
	if (layout->labeltype == LABEL_COUNTER) {
		BufferParams const & bufparams = buf.params();
		LyXTextClass const & textclass = bufparams.getLyXTextClass();
		counters.step(layout->counter);
		string label = expandLabel(textclass, layout, pit->params().appendix());
		pit->params().labelString(label);
	} else if (layout->labeltype == LABEL_ITEMIZE) {
		// At some point of time we should do something more
		// clever here, like:
		//   pit->params().labelString(
		//    bufparams.user_defined_bullet(pit->itemdepth).getText());
		// for now, use a simple hardcoded label
		string itemlabel;
		switch (pit->itemdepth) {
		case 0:
			itemlabel = "*";
			break;
		case 1:
			itemlabel = "-";
			break;
		case 2:
			itemlabel = "@";
			break;
		case 3:
			itemlabel = "·";
			break;
		}

		pit->params().labelString(itemlabel);
	} else if (layout->labeltype == LABEL_ENUMERATE) {
		// Maybe we have to reset the enumeration counter.
		resetEnumCounterIfNeeded(pit, first_pit, counters);

		// FIXME
		// Yes I know this is a really, really! bad solution
		// (Lgb)
		string enumcounter = "enum";

		switch (pit->itemdepth) {
		case 2:
			enumcounter += 'i';
		case 1:
			enumcounter += 'i';
		case 0:
			enumcounter += 'i';
			break;
		case 3:
			enumcounter += "iv";
			break;
		default:
			// not a valid enumdepth...
			break;
		}

		counters.step(enumcounter);

		pit->params().labelString(counters.enumLabel(enumcounter));
	} else if (layout->labeltype == LABEL_BIBLIO) {// ale970302
		counters.step("bibitem");
		int number = counters.value("bibitem");
		if (pit->bibitem()) {
			pit->bibitem()->setCounter(number);
			pit->params().labelString(layout->labelstring());
		}
		// In biblio should't be following counters but...
	} else {
		string s = buf.B_(layout->labelstring());

		// the caption hack:
		if (layout->labeltype == LABEL_SENSITIVE) {
			ParagraphList::iterator end = ownerParagraphs().end();
			ParagraphList::iterator tmppit = pit;
			InsetOld * in = 0;
			bool isOK = false;
			while (tmppit != end && tmppit->inInset()
			       // the single '=' is intended below
			       && (in = tmppit->inInset()->owner()))
			{
				if (in->lyxCode() == InsetOld::FLOAT_CODE ||
				    in->lyxCode() == InsetOld::WRAP_CODE) {
					isOK = true;
					break;
				} else {
					Paragraph const * owner = &ownerPar(buf, in);
					tmppit = first_pit;
					for ( ; tmppit != end; ++tmppit)
						if (&*tmppit == owner)
							break;
				}
			}

			if (isOK) {
				string type;

				if (in->lyxCode() == InsetOld::FLOAT_CODE)
					type = static_cast<InsetFloat*>(in)->params().type;
				else if (in->lyxCode() == InsetOld::WRAP_CODE)
					type = static_cast<InsetWrap*>(in)->params().type;
				else
					BOOST_ASSERT(false);

				Floating const & fl = textclass.floats().getType(type);

				counters.step(fl.type());

				// Doesn't work... yet.
				s = bformat(_("%1$s #:"), buf.B_(fl.name()));
			} else {
				// par->SetLayout(0);
				// s = layout->labelstring;
				s = _("Senseless: ");
			}
		}
		pit->params().labelString(s);

	}
}


// Updates all counters. Paragraphs with changed label string will be rebroken
void LyXText::updateCounters()
{
	// start over
	bv()->buffer()->params().getLyXTextClass().counters().reset();

	ParagraphList::iterator beg = ownerParagraphs().begin();
	ParagraphList::iterator end = ownerParagraphs().end();
	for (ParagraphList::iterator pit = beg; pit != end; ++pit) {
		string const oldLabel = pit->params().labelString();

		size_t maxdepth = 0;
		if (pit != beg)
			maxdepth = boost::prior(pit)->getMaxDepthAfter();

		if (pit->params().depth() > maxdepth)
			pit->params().depth(maxdepth);

		// setCounter can potentially change the labelString.
		setCounter(*bv()->buffer(), pit);

		string const & newLabel = pit->params().labelString();

		if (oldLabel != newLabel)
			redoParagraph(pit);
	}
}


void LyXText::insertInset(InsetOld * inset)
{
	if (!cursorPar()->insetAllowed(inset->lyxCode()))
		return;
	recUndo(cursor.par());
	freezeUndo();
	cursorPar()->insertInset(cursor.pos(), inset);
	// Just to rebreak and refresh correctly.
	// The character will not be inserted a second time
	insertChar(Paragraph::META_INSET);
	// If we enter a highly editable inset the cursor should be before
	// the inset. After an Undo LyX tries to call inset->edit(...)
	// and fails if the cursor is behind the inset and getInset
	// does not return the inset!
	if (isHighlyEditableInset(inset))
		cursorLeft(true);
	unFreezeUndo();
}


void LyXText::cutSelection(bool doclear, bool realcut)
{
	// Stuff what we got on the clipboard. Even if there is no selection.

	// There is a problem with having the stuffing here in that the
	// larger the selection the slower LyX will get. This can be
	// solved by running the line below only when the selection has
	// finished. The solution used currently just works, to make it
	// faster we need to be more clever and probably also have more
	// calls to stuffClipboard. (Lgb)
	bv()->stuffClipboard(selectionAsString(*bv()->buffer(), true));

	// This doesn't make sense, if there is no selection
	if (!selection.set())
		return;

	// OK, we have a selection. This is always between selection.start
	// and selection.end

	// make sure that the depth behind the selection are restored, too
	ParagraphList::iterator endpit = boost::next(getPar(selection.end.par()));
	ParagraphList::iterator undoendpit = endpit;
	ParagraphList::iterator pars_end = ownerParagraphs().end();

	if (endpit != pars_end && endpit->getDepth()) {
		while (endpit != pars_end && endpit->getDepth()) {
			++endpit;
			undoendpit = endpit;
		}
	} else if (endpit != pars_end) {
		// because of parindents etc.
		++endpit;
	}

	recUndo(selection.start.par(), parOffset(undoendpit) - 1);

	endpit = getPar(selection.end.par());
	int endpos = selection.end.pos();

	BufferParams const & bufparams = bv()->buffer()->params();
	boost::tie(endpit, endpos) = realcut ?
		CutAndPaste::cutSelection(bufparams,
					  ownerParagraphs(),
					  getPar(selection.start.par()), endpit,
					  selection.start.pos(), endpos,
					  bufparams.textclass,
					  doclear)
		: CutAndPaste::eraseSelection(bufparams,
					      ownerParagraphs(),
					      getPar(selection.start.par()), endpit,
					      selection.start.pos(), endpos,
					      doclear);
	// sometimes necessary
	if (doclear)
		getPar(selection.start.par())->stripLeadingSpaces();

	redoParagraphs(getPar(selection.start.par()), boost::next(endpit));
	// cutSelection can invalidate the cursor so we need to set
	// it anew. (Lgb)
	// we prefer the end for when tracking changes
	cursor.pos(endpos);
	cursor.par(parOffset(endpit));

	// need a valid cursor. (Lgb)
	clearSelection();

	setCursor(cursorPar(), cursor.pos());
	selection.cursor = cursor;
	updateCounters();
}


void LyXText::copySelection()
{
	// stuff the selection onto the X clipboard, from an explicit copy request
	bv()->stuffClipboard(selectionAsString(*bv()->buffer(), true));

	// this doesnt make sense, if there is no selection
	if (!selection.set())
		return;

	// ok we have a selection. This is always between selection.start
	// and sel_end cursor

	// copy behind a space if there is one
	while (getPar(selection.start)->size() > selection.start.pos()
	       && getPar(selection.start)->isLineSeparator(selection.start.pos())
	       && (selection.start.par() != selection.end.par()
		   || selection.start.pos() < selection.end.pos()))
		selection.start.pos(selection.start.pos() + 1);

	CutAndPaste::copySelection(getPar(selection.start.par()),
				   getPar(selection.end.par()),
				   selection.start.pos(), selection.end.pos(),
				   bv()->buffer()->params().textclass);
}


void LyXText::pasteSelection(size_t sel_index)
{
	// this does not make sense, if there is nothing to paste
	if (!CutAndPaste::checkPastePossible())
		return;

	recUndo(cursor.par());

	ParagraphList::iterator endpit;
	PitPosPair ppp;

	ErrorList el;

	boost::tie(ppp, endpit) =
		CutAndPaste::pasteSelection(*bv()->buffer(),
					    ownerParagraphs(),
					    cursorPar(), cursor.pos(),
					    bv()->buffer()->params().textclass,
					    sel_index, el);
	bufferErrors(*bv()->buffer(), el);
	bv()->showErrorList(_("Paste"));

	redoParagraphs(cursorPar(), endpit);

	setCursor(cursor.par(), cursor.pos());
	clearSelection();

	selection.cursor = cursor;
	setCursor(ppp.first, ppp.second);
	setSelection();
	updateCounters();
}


void LyXText::setSelectionRange(lyx::pos_type length)
{
	if (!length)
		return;

	selection.cursor = cursor;
	while (length--)
		cursorRight(bv());
	setSelection();
}


// simple replacing. The font of the first selected character is used
void LyXText::replaceSelectionWithString(string const & str)
{
	recUndo(cursor.par());
	freezeUndo();

	if (!selection.set()) { // create a dummy selection
		selection.end = cursor;
		selection.start = cursor;
	}

	// Get font setting before we cut
	pos_type pos = selection.end.pos();
	LyXFont const font = getPar(selection.start)
		->getFontSettings(bv()->buffer()->params(),
				  selection.start.pos());

	// Insert the new string
	string::const_iterator cit = str.begin();
	string::const_iterator end = str.end();
	for (; cit != end; ++cit) {
		getPar(selection.end)->insertChar(pos, (*cit), font);
		++pos;
	}

	// Cut the selection
	cutSelection(true, false);

	unFreezeUndo();
}


// needed to insert the selection
void LyXText::insertStringAsLines(string const & str)
{
	ParagraphList::iterator pit = cursorPar();
	pos_type pos = cursor.pos();
	ParagraphList::iterator endpit = boost::next(cursorPar());

	recUndo(cursor.par());

	// only to be sure, should not be neccessary
	clearSelection();

	bv()->buffer()->insertStringAsLines(pit, pos, current_font, str);

	redoParagraphs(cursorPar(), endpit);
	setCursor(cursorPar(), cursor.pos());
	selection.cursor = cursor;
	setCursor(pit, pos);
	setSelection();
}


// turns double-CR to single CR, others where converted into one
// blank. Then InsertStringAsLines is called
void LyXText::insertStringAsParagraphs(string const & str)
{
	string linestr(str);
	bool newline_inserted = false;
	string::size_type const siz = linestr.length();

	for (string::size_type i = 0; i < siz; ++i) {
		if (linestr[i] == '\n') {
			if (newline_inserted) {
				// we know that \r will be ignored by
				// InsertStringA. Of course, it is a dirty
				// trick, but it works...
				linestr[i - 1] = '\r';
				linestr[i] = '\n';
			} else {
				linestr[i] = ' ';
				newline_inserted = true;
			}
		} else if (IsPrintable(linestr[i])) {
			newline_inserted = false;
		}
	}
	insertStringAsLines(linestr);
}


void LyXText::setCursor(ParagraphList::iterator pit, pos_type pos)
{
	setCursor(parOffset(pit), pos);
}


bool LyXText::setCursor(paroffset_type par, pos_type pos, bool setfont, bool boundary)
{
	LyXCursor old_cursor = cursor;
	setCursorIntern(par, pos, setfont, boundary);
	return deleteEmptyParagraphMechanism(old_cursor);
}


void LyXText::redoCursor()
{
#warning maybe the same for selections?
	setCursor(cursor, cursor.par(), cursor.pos(), cursor.boundary());
}


void LyXText::setCursor(LyXCursor & cur, paroffset_type par,
	pos_type pos, bool boundary)
{
	BOOST_ASSERT(par != int(ownerParagraphs().size()));

	cur.par(par);
	cur.pos(pos);
	cur.boundary(boundary);

	// no rows, no fun...
	if (ownerParagraphs().begin()->rows.empty())
		return;

	// get the cursor y position in text

	ParagraphList::iterator pit = getPar(par);
	Row const & row = *pit->getRow(pos);
	int y = pit->y + row.y_offset();

	// y is now the beginning of the cursor row
	y += row.baseline();
	// y is now the cursor baseline
	cur.y(y);

	pos_type const end = row.endpos();

	// None of these should happen, but we're scaredy-cats
	if (pos < 0) {
		lyxerr << "dont like -1" << endl;
		pos = 0;
		cur.pos(0);
		BOOST_ASSERT(false);
	} else if (pos > pit->size()) {
		lyxerr << "dont like 1, pos: " << pos
		       << " size: " << pit->size()
		       << " row.pos():" << row.pos()
		       << " paroffset: " << par << endl;
		pos = 0;
		cur.pos(0);
		BOOST_ASSERT(false);
	} else if (pos > end) {
		lyxerr << "dont like 2 please report" << endl;
		// This shouldn't happen.
		pos = end;
		cur.pos(pos);
		BOOST_ASSERT(false);
	} else if (pos < row.pos()) {
		lyxerr << "dont like 3 please report pos:" << pos
		       << " size: " << pit->size()
		       << " row.pos():" << row.pos()
		       << " paroffset: " << par << endl;
		pos = row.pos();
		cur.pos(pos);
		BOOST_ASSERT(false);
	}
	// now get the cursors x position
	float x = getCursorX(pit, row, pos, boundary);
	cur.x(int(x));
	cur.x_fix(cur.x());
}


float LyXText::getCursorX(ParagraphList::iterator pit, Row const & row,
			  pos_type pos, bool boundary) const
{
	pos_type cursor_vpos    = 0;
	double x                = row.x();
	double fill_separator   = row.fill_separator();
	double fill_hfill       = row.fill_hfill();
	double fill_label_hfill = row.fill_label_hfill();
	pos_type const row_pos  = row.pos();
	pos_type const end = row.endpos();

	if (end <= row_pos)
		cursor_vpos = row_pos;
	else if (pos >= end && !boundary)
		cursor_vpos = (pit->isRightToLeftPar(bv()->buffer()->params()))
			? row_pos : end;
	else if (pos > row_pos && (pos >= end || boundary))
		// Place cursor after char at (logical) position pos - 1
		cursor_vpos = (bidi.level(pos - 1) % 2 == 0)
			? bidi.log2vis(pos - 1) + 1 : bidi.log2vis(pos - 1);
	else
		// Place cursor before char at (logical) position pos
		cursor_vpos = (bidi.level(pos) % 2 == 0)
			? bidi.log2vis(pos) : bidi.log2vis(pos) + 1;

	pos_type body_pos = pit->beginningOfBody();
	if (body_pos > 0 &&
	    (body_pos > end || !pit->isLineSeparator(body_pos - 1)))
		body_pos = 0;

	for (pos_type vpos = row_pos; vpos < cursor_vpos; ++vpos) {
		pos_type pos = bidi.vis2log(vpos);
		if (body_pos > 0 && pos == body_pos - 1) {
			x += fill_label_hfill
				+ font_metrics::width(pit->layout()->labelsep,
						      getLabelFont(pit));
			if (pit->isLineSeparator(body_pos - 1))
				x -= singleWidth(pit, body_pos - 1);
		}

		if (hfillExpansion(*pit, row, pos)) {
			x += singleWidth(pit, pos);
			if (pos >= body_pos)
				x += fill_hfill;
			else
				x += fill_label_hfill;
		} else if (pit->isSeparator(pos)) {
			x += singleWidth(pit, pos);
			if (pos >= body_pos)
				x += fill_separator;
		} else
			x += singleWidth(pit, pos);
	}
	return x;
}


void LyXText::setCursorIntern(paroffset_type par,
			      pos_type pos, bool setfont, bool boundary)
{
	setCursor(cursor, par, pos, boundary);
	if (setfont)
		setCurrentFont();
}


void LyXText::setCurrentFont()
{
	pos_type pos = cursor.pos();
	ParagraphList::iterator pit = cursorPar();

	if (cursor.boundary() && pos > 0)
		--pos;

	if (pos > 0) {
		if (pos == pit->size())
			--pos;
		else // potentional bug... BUG (Lgb)
			if (pit->isSeparator(pos)) {
				if (pos > pit->getRow(pos)->pos() &&
				    bidi.level(pos) % 2 ==
				    bidi.level(pos - 1) % 2)
					--pos;
				else if (pos + 1 < pit->size())
					++pos;
			}
	}

	BufferParams const & bufparams = bv()->buffer()->params();
	current_font = pit->getFontSettings(bufparams, pos);
	real_current_font = getFont(pit, pos);

	if (cursor.pos() == pit->size() &&
	    bidi.isBoundary(*bv()->buffer(), *pit, cursor.pos()) &&
	    !cursor.boundary()) {
		Language const * lang =
			pit->getParLanguage(bufparams);
		current_font.setLanguage(lang);
		current_font.setNumber(LyXFont::OFF);
		real_current_font.setLanguage(lang);
		real_current_font.setNumber(LyXFont::OFF);
	}
}


// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type LyXText::getColumnNearX(ParagraphList::iterator pit,
	Row const & row, int & x, bool & boundary) const
{
	double tmpx             = row.x();
	double fill_separator   = row.fill_separator();
	double fill_hfill       = row.fill_hfill();
	double fill_label_hfill = row.fill_label_hfill();

	pos_type vc = row.pos();
	pos_type end = row.endpos();
	pos_type c = 0;
	LyXLayout_ptr const & layout = pit->layout();

	bool left_side = false;

	pos_type body_pos = pit->beginningOfBody();
	double last_tmpx = tmpx;

	if (body_pos > 0 &&
	    (body_pos > end ||
	     !pit->isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// check for empty row
	if (vc == end) {
		x = int(tmpx);
		return 0;
	}

	while (vc < end && tmpx <= x) {
		c = bidi.vis2log(vc);
		last_tmpx = tmpx;
		if (body_pos > 0 && c == body_pos - 1) {
			tmpx += fill_label_hfill +
				font_metrics::width(layout->labelsep, getLabelFont(pit));
			if (pit->isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(pit, body_pos - 1);
		}

		if (hfillExpansion(*pit, row, c)) {
			tmpx += singleWidth(pit, c);
			if (c >= body_pos)
				tmpx += fill_hfill;
			else
				tmpx += fill_label_hfill;
		} else if (pit->isSeparator(c)) {
			tmpx += singleWidth(pit, c);
			if (c >= body_pos)
				tmpx += fill_separator;
		} else {
			tmpx += singleWidth(pit, c);
		}
		++vc;
	}

	if ((tmpx + last_tmpx) / 2 > x) {
		tmpx = last_tmpx;
		left_side = true;
	}

	BOOST_ASSERT(vc <= end);  // This shouldn't happen.

	boundary = false;
	// This (rtl_support test) is not needed, but gives
	// some speedup if rtl_support == false
	bool const lastrow = lyxrc.rtl_support && row.endpos() == pit->size();

	// If lastrow is false, we don't need to compute
	// the value of rtl.
	bool const rtl = (lastrow)
		? pit->isRightToLeftPar(bv()->buffer()->params())
		: false;
	if (lastrow &&
		 ((rtl  &&  left_side && vc == row.pos() && x < tmpx - 5) ||
		  (!rtl && !left_side && vc == end  && x > tmpx + 5)))
		c = end;
	else if (vc == row.pos()) {
		c = bidi.vis2log(vc);
		if (bidi.level(c) % 2 == 1)
			++c;
	} else {
		c = bidi.vis2log(vc - 1);
		bool const rtl = (bidi.level(c) % 2 == 1);
		if (left_side == rtl) {
			++c;
			boundary = bidi.isBoundary(*bv()->buffer(), *pit, c);
		}
	}

	if (row.pos() < end && c >= end && pit->isNewline(end - 1)) {
		if (bidi.level(end -1) % 2 == 0)
			tmpx -= singleWidth(pit, end - 1);
		else
			tmpx += singleWidth(pit, end - 1);
		c = end - 1;
	}

	c -= row.pos();
	x = int(tmpx);
	return c;
}


void LyXText::setCursorFromCoordinates(int x, int y)
{
	LyXCursor old_cursor = cursor;
	setCursorFromCoordinates(cursor, x, y);
	setCurrentFont();
	deleteEmptyParagraphMechanism(old_cursor);
}


void LyXText::setCursorFromCoordinates(LyXCursor & cur, int x, int y)
{
	// Get the row first.
	ParagraphList::iterator pit;
	Row const & row = *getRowNearY(y, pit);
	y = pit->y + row.y_offset();

	bool bound = false;
	pos_type const column = getColumnNearX(pit, row, x, bound);
	cur.par(parOffset(pit));
	cur.pos(row.pos() + column);
	cur.x(x);
	cur.y(y + row.baseline());

	cur.boundary(bound);
}


void LyXText::cursorLeft(bool internal)
{
	if (cursor.pos() > 0) {
		bool boundary = cursor.boundary();
		setCursor(cursor.par(), cursor.pos() - 1, true, false);
		if (!internal && !boundary &&
		    bidi.isBoundary(*bv()->buffer(), *cursorPar(), cursor.pos() + 1))
			setCursor(cursor.par(), cursor.pos() + 1, true, true);
	} else if (cursor.par() != 0) {
		// steps into the paragraph above
		setCursor(cursor.par() - 1, boost::prior(cursorPar())->size());
	}
}


void LyXText::cursorRight(bool internal)
{
	bool const at_end = (cursor.pos() == cursorPar()->size());
	bool const at_newline = !at_end &&
		cursorPar()->isNewline(cursor.pos());

	if (!internal && cursor.boundary() && !at_newline)
		setCursor(cursor.par(), cursor.pos(), true, false);
	else if (!at_end) {
		setCursor(cursor.par(), cursor.pos() + 1, true, false);
		if (!internal && bidi.isBoundary(*bv()->buffer(), *cursorPar(),
						 cursor.pos()))
			setCursor(cursor.par(), cursor.pos(), true, true);
	} else if (cursor.par() + 1 != int(ownerParagraphs().size()))
		setCursor(cursor.par() + 1, 0);
}


void LyXText::cursorUp(bool selecting)
{
	ParagraphList::iterator cpit = cursorPar();
	Row const & crow = *cpit->getRow(cursor.pos());
#if 1
	int x = cursor.x_fix();
	int y = cursor.y() - crow.baseline() - 1;
	setCursorFromCoordinates(x, y);
	if (!selecting) {
		int topy = bv_owner->top_y();
		int y1 = cursor.y() - topy;
		int y2 = y1;
		y -= topy;
		InsetOld * inset_hit = checkInsetHit(x, y1);
		if (inset_hit && isHighlyEditableInset(inset_hit)) {
			inset_hit->dispatch(
				FuncRequest(bv(), LFUN_INSET_EDIT, x, y - (y2 - y1), mouse_button::none));
		}
	}
#else
	lyxerr << "cursorUp: y " << cursor.y() << " bl: " <<
		crow.baseline() << endl;
	setCursorFromCoordinates(cursor.x_fix(),
		cursor.y() - crow.baseline() - 1);
#endif
}


void LyXText::cursorDown(bool selecting)
{
	ParagraphList::iterator cpit = cursorPar();
	Row const & crow = *cpit->getRow(cursor.pos());
#if 1
	int x = cursor.x_fix();
	int y = cursor.y() - crow.baseline() + crow.height() + 1;
	setCursorFromCoordinates(x, y);
	if (!selecting) {
		int topy = bv_owner->top_y();
		int y1 = cursor.y() - topy;
		int y2 = y1;
		y -= topy;
		InsetOld * inset_hit = checkInsetHit(x, y1);
		if (inset_hit && isHighlyEditableInset(inset_hit)) {
			FuncRequest cmd(bv(), LFUN_INSET_EDIT, x, y - (y2 - y1), mouse_button::none);
			inset_hit->dispatch(cmd);
		}
	}
#else
	setCursorFromCoordinates(cursor.x_fix(),
		 cursor.y() - crow.baseline() + crow.height() + 1);
#endif
}


void LyXText::cursorUpParagraph()
{
	ParagraphList::iterator cpit = cursorPar();
	if (cursor.pos() > 0)
		setCursor(cpit, 0);
	else if (cpit != ownerParagraphs().begin())
		setCursor(boost::prior(cpit), 0);
}


void LyXText::cursorDownParagraph()
{
	ParagraphList::iterator pit = cursorPar();
	ParagraphList::iterator next_pit = boost::next(pit);

	if (next_pit != ownerParagraphs().end())
		setCursor(next_pit, 0);
	else
		setCursor(pit, pit->size());
}


// fix the cursor `cur' after a characters has been deleted at `where'
// position. Called by deleteEmptyParagraphMechanism
void LyXText::fixCursorAfterDelete(LyXCursor & cur, LyXCursor const & where)
{
	// if cursor is not in the paragraph where the delete occured,
	// do nothing
	if (cur.par() != where.par())
		return;

	// if cursor position is after the place where the delete occured,
	// update it
	if (cur.pos() > where.pos())
		cur.pos(cur.pos()-1);

	// check also if we don't want to set the cursor on a spot behind the
	// pagragraph because we erased the last character.
	if (cur.pos() > getPar(cur)->size())
		cur.pos(getPar(cur)->size());

	// recompute row et al. for this cursor
	setCursor(cur, cur.par(), cur.pos(), cur.boundary());
}


bool LyXText::deleteEmptyParagraphMechanism(LyXCursor const & old_cursor)
{
	// Would be wrong to delete anything if we have a selection.
	if (selection.set())
		return false;

	// Don't do anything if the cursor is invalid
	if (old_cursor.par() == -1)
		return false;

	// We allow all kinds of "mumbo-jumbo" when freespacing.
	ParagraphList::iterator const old_pit = getPar(old_cursor);
	if (old_pit->isFreeSpacing())
		return false;

	/* Ok I'll put some comments here about what is missing.
	   I have fixed BackSpace (and thus Delete) to not delete
	   double-spaces automagically. I have also changed Cut,
	   Copy and Paste to hopefully do some sensible things.
	   There are still some small problems that can lead to
	   double spaces stored in the document file or space at
	   the beginning of paragraphs. This happens if you have
	   the cursor between to spaces and then save. Or if you
	   cut and paste and the selection have a space at the
	   beginning and then save right after the paste. I am
	   sure none of these are very hard to fix, but I will
	   put out 1.1.4pre2 with FIX_DOUBLE_SPACE defined so
	   that I can get some feedback. (Lgb)
	*/

	// If old_cursor.pos() == 0 and old_cursor.pos()(1) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	// If old_cursor.pos() == 1 and old_cursor.pos()(0) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	// If the pos around the old_cursor were spaces, delete one of them.
	if (old_cursor.par() != cursor.par()
	    || old_cursor.pos() != cursor.pos()) {

		// Only if the cursor has really moved
		if (old_cursor.pos() > 0
		    && old_cursor.pos() < old_pit->size()
		    && old_pit->isLineSeparator(old_cursor.pos())
		    && old_pit->isLineSeparator(old_cursor.pos() - 1)) {
			bool erased = old_pit->erase(old_cursor.pos() - 1);
			redoParagraph(old_pit);

			if (!erased)
				return false;
#ifdef WITH_WARNINGS
#warning This will not work anymore when we have multiple views of the same buffer
// In this case, we will have to correct also the cursors held by
// other bufferviews. It will probably be easier to do that in a more
// automated way in LyXCursor code. (JMarc 26/09/2001)
#endif
			// correct all cursors held by the LyXText
			fixCursorAfterDelete(cursor, old_cursor);
			fixCursorAfterDelete(selection.cursor, old_cursor);
			fixCursorAfterDelete(selection.start, old_cursor);
			fixCursorAfterDelete(selection.end, old_cursor);
			return false;
		}
	}

	// don't delete anything if this is the ONLY paragraph!
	if (ownerParagraphs().size() == 1)
		return false;

	// Do not delete empty paragraphs with keepempty set.
	if (old_pit->allowEmpty())
		return false;

	// only do our magic if we changed paragraph
	if (old_cursor.par() == cursor.par())
		return false;

	// record if we have deleted a paragraph
	// we can't possibly have deleted a paragraph before this point
	bool deleted = false;

	if (old_pit->empty()
	    || (old_pit->size() == 1 && old_pit->isLineSeparator(0))) {
		// ok, we will delete something
		LyXCursor tmpcursor;

		deleted = true;

		bool selection_position_was_oldcursor_position =
			selection.cursor.par() == old_cursor.par()
			&& selection.cursor.pos() == old_cursor.pos();

		tmpcursor = cursor;
		cursor = old_cursor; // that undo can restore the right cursor position

		ParagraphList::iterator endpit = boost::next(old_pit);
		while (endpit != ownerParagraphs().end() && endpit->getDepth())
			++endpit;

		recUndo(parOffset(old_pit), parOffset(endpit) - 1);
		cursor = tmpcursor;

		// delete old par
		ownerParagraphs().erase(old_pit);
		redoParagraph();

		// correct cursor y
		setCursorIntern(cursor.par(), cursor.pos());

		if (selection_position_was_oldcursor_position) {
			// correct selection
			selection.cursor = cursor;
		}
	}

	if (deleted)
		return true;

	if (old_pit->stripLeadingSpaces()) {
		redoParagraph(old_pit);
		// correct cursor y
		setCursorIntern(cursor.par(), cursor.pos());
		selection.cursor = cursor;
	}
	return false;
}


ParagraphList & LyXText::ownerParagraphs() const
{
	return *paragraphs_;
}


void LyXText::recUndo(paroffset_type first, paroffset_type last) const
{
	recordUndo(Undo::ATOMIC, this, first, last);
}


void LyXText::recUndo(lyx::paroffset_type par) const
{
	recordUndo(Undo::ATOMIC, this, par, par);
}


bool LyXText::isInInset() const
{
	// Sub-level has non-null bv owner and non-null inset owner.
	return inset_owner != 0;
}


int defaultRowHeight()
{
	LyXFont const font(LyXFont::ALL_SANE);
	return int(font_metrics::maxHeight(font) *  1.2);
}
