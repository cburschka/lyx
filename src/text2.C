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
#include "cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "dispatchresult.h"
#include "errorlist.h"
#include "Floating.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxrow_funcs.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "PosIterator.h"
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
using lyx::support::bformat;

using std::endl;
using std::ostringstream;
using std::string;


LyXText::LyXText(BufferView * bv, bool in_inset)
	: height(0), width(0), textwidth_(bv ? bv->workWidth() : 100),
		background_color_(LColor::background),
	  bv_owner(bv), in_inset_(in_inset), xo_(0), yo_(0)
{}


void LyXText::init(BufferView * bv)
{
	bv_owner = bv;

	ParagraphList::iterator const beg = paragraphs().begin();
	ParagraphList::iterator const end = paragraphs().end();
	for (ParagraphList::iterator pit = beg; pit != end; ++pit)
		pit->rows.clear();

	width = 0;
	height = 0;

	current_font = getFont(beg, 0);

	redoParagraphs(beg, end);
	bv->cursor().resetAnchor();

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
	pos_type const body_pos = pit->beginOfBody();

	// We specialize the 95% common case:
	if (!pit->getDepth()) {
		LyXFont f = pit->getFontSettings(params, pos);
		if (in_inset_)
			f.realize(font_);
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

	if (in_inset_)
		font.realize(font_);

	// Realize with the fonts of lesser depth.
	//font.realize(outerFont(pit, paragraphs()));
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
	//font.realize(outerFont(pit, paragraphs()));
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
	font.realize(outerFont(pit, paragraphs()));
	font.realize(defaultfont_);

	return font;
}


void LyXText::setCharFont(
	ParagraphList::iterator pit, pos_type pos, LyXFont const & fnt)
{
	LyXFont font = fnt;
	LyXLayout_ptr const & layout = pit->layout();

	// Get concrete layout font to reduce against
	LyXFont layoutfont;

	if (pos < pit->beginOfBody())
		layoutfont = layout->labelfont;
	else
		layoutfont = layout->font;

	// Realize against environment font information
	if (pit->getDepth()) {
		ParagraphList::iterator tp = pit;
		while (!layoutfont.resolved() &&
		       tp != paragraphs().end() &&
		       tp->getDepth()) {
			tp = outerHook(tp, paragraphs());
			if (tp != paragraphs().end())
				layoutfont.realize(tp->layout()->font);
		}
	}

	layoutfont.realize(defaultfont_);

	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	pit->setFont(pos, font);
}


// used in setLayout
// Asger is not sure we want to do this...
void LyXText::makeFontEntriesLayoutSpecific(BufferParams const & params,
					    Paragraph & par)
{
	LyXLayout_ptr const & layout = par.layout();
	pos_type const psize = par.size();

	LyXFont layoutfont;
	for (pos_type pos = 0; pos < psize; ++pos) {
		if (pos < par.beginOfBody())
			layoutfont = layout->labelfont;
		else
			layoutfont = layout->font;

		LyXFont tmpfont = par.getFontSettings(params, pos);
		tmpfont.reduce(layoutfont);
		par.setFont(pos, tmpfont);
	}
}


// return past-the-last paragraph influenced by a layout change on pit
ParagraphList::iterator LyXText::undoSpan(ParagraphList::iterator pit)
{
	ParagraphList::iterator end = paragraphs().end();
	ParagraphList::iterator nextpit = boost::next(pit);
	if (nextpit == end)
		return nextpit;
	//because of parindents
	if (!pit->getDepth())
		return boost::next(nextpit);
	//because of depth constrains
	for (; nextpit != end; ++pit, ++nextpit) {
		if (!pit->getDepth())
			break;
	}
	return nextpit;
}


ParagraphList::iterator
LyXText::setLayout(ParagraphList::iterator start,
		   ParagraphList::iterator end,
		   string const & layout)
{
	BOOST_ASSERT(start != end);
	ParagraphList::iterator undopit = undoSpan(boost::prior(end));
	recUndo(parOffset(start), parOffset(undopit) - 1);

	BufferParams const & bufparams = bv()->buffer()->params();
	LyXLayout_ptr const & lyxlayout =
		bufparams.getLyXTextClass()[layout];

	for (ParagraphList::iterator pit = start; pit != end; ++pit) {
		pit->applyLayout(lyxlayout);
		makeFontEntriesLayoutSpecific(bufparams, *pit);
		if (lyxlayout->margintype == MARGIN_MANUAL)
			pit->setLabelWidthString(lyxlayout->labelstring());
	}

	return undopit;
}


// set layout over selection and make a total rebreak of those paragraphs
void LyXText::setLayout(LCursor & cur, string const & layout)
{
	BOOST_ASSERT(this == cur.text());
	// special handling of new environment insets
	BufferParams const & params = bv()->buffer()->params();
	LyXLayout_ptr const & lyxlayout = params.getLyXTextClass()[layout];
	if (lyxlayout->is_environment) {
		// move everything in a new environment inset
		lyxerr << "setting layout " << layout << endl;
		bv()->owner()->dispatch(FuncRequest(LFUN_HOME));
		bv()->owner()->dispatch(FuncRequest(LFUN_ENDSEL));
		bv()->owner()->dispatch(FuncRequest(LFUN_CUT));
		InsetBase * inset = new InsetEnvironment(params, layout);
		insertInset(cur, inset);
		//inset->edit(cur, true);
		//bv()->owner()->dispatch(FuncRequest(LFUN_PASTE));
		return;
	}

	ParagraphList::iterator start = getPar(cur.selBegin().par());
	ParagraphList::iterator end = boost::next(getPar(cur.selEnd().par()));
	ParagraphList::iterator endpit = setLayout(start, end, layout);
	redoParagraphs(start, endpit);
	updateCounters();
}


namespace {


void getSelectionSpan(LCursor & cur, LyXText & text,
	ParagraphList::iterator & beg,
	ParagraphList::iterator & end)
{
	if (!cur.selection()) {
		beg = text.getPar(cur.par());
		end = boost::next(beg);
	} else {
		beg = text.getPar(cur.selBegin());
		end = boost::next(text.getPar(cur.selEnd()));
	}
}


bool changeDepthAllowed(bv_funcs::DEPTH_CHANGE type,
			Paragraph const & par,
			int max_depth)
{
	if (par.layout()->labeltype == LABEL_BIBLIO)
		return false;
	int const depth = par.params().depth();
	if (type == bv_funcs::INC_DEPTH && depth < max_depth)
		return true;
	if (type == bv_funcs::DEC_DEPTH && depth > 0)
		return true;
	return false;
}


}


bool LyXText::changeDepthAllowed(LCursor & cur, bv_funcs::DEPTH_CHANGE type)
{
	BOOST_ASSERT(this == cur.text());
	ParagraphList::iterator beg, end; 
	getSelectionSpan(cur, *this, beg, end);
	int max_depth = 0;
	if (beg != paragraphs().begin())
		max_depth = boost::prior(beg)->getMaxDepthAfter();

	for (ParagraphList::iterator pit = beg; pit != end; ++pit) {
		if (::changeDepthAllowed(type, *pit, max_depth))
			return true;
		max_depth = pit->getMaxDepthAfter();
	}
	return false;
}


void LyXText::changeDepth(LCursor & cur, bv_funcs::DEPTH_CHANGE type)
{
	BOOST_ASSERT(this == cur.text());
	ParagraphList::iterator beg, end;
	getSelectionSpan(cur, *this, beg, end);
	recordUndoSelection(cur);

	int max_depth = 0;
	if (beg != paragraphs().begin())
		max_depth = boost::prior(beg)->getMaxDepthAfter();

	for (ParagraphList::iterator pit = beg; pit != end; ++pit) {
		if (::changeDepthAllowed(type, *pit, max_depth)) {
			int const depth = pit->params().depth();
			if (type == bv_funcs::INC_DEPTH)
				pit->params().depth(depth + 1);
			else
				pit->params().depth(depth - 1);
		}
		max_depth = pit->getMaxDepthAfter();
	}
	// this handles the counter labels, and also fixes up
	// depth values for follow-on (child) paragraphs
	updateCounters();
}


// set font over selection and make a total rebreak of those paragraphs
void LyXText::setFont(LCursor & cur, LyXFont const & font, bool toggleall)
{
	BOOST_ASSERT(this == cur.text());
	// if there is no selection just set the current_font
	if (!cur.selection()) {
		// Determine basis font
		LyXFont layoutfont;
		ParagraphList::iterator pit = getPar(cur.par());
		if (cur.pos() < pit->beginOfBody())
			layoutfont = getLabelFont(pit);
		else
			layoutfont = getLayoutFont(pit);

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

	// ok we have a selection.
	recordUndoSelection(cur);
	freezeUndo();

	ParagraphList::iterator beg = getPar(cur.selBegin().par());
	ParagraphList::iterator end = getPar(cur.selEnd().par());
	
	PosIterator pos(&paragraphs(), beg, cur.selBegin().pos());
	PosIterator posend(&paragraphs(), end, cur.selEnd().pos());

	BufferParams const & params = bv()->buffer()->params();

	for (; pos != posend; ++pos) {
		LyXFont f = getFont(pos.pit(), pos.pos());
		f.update(font, params.language, toggleall);
		setCharFont(pos.pit(), pos.pos(), f);
	}
	
	unFreezeUndo();

	redoParagraphs(beg, ++end);
}


// the cursor set functions have a special mechanism. When they
// realize you left an empty paragraph, they will delete it.

void LyXText::cursorHome(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	setCursor(cur, cur.par(), cur.textRow().pos());
}


void LyXText::cursorEnd(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	// if not on the last row of the par, put the cursor before
	// the final space
	pos_type const end = cur.textRow().endpos();
	setCursor(cur, cur.par(), end == cur.lastpos() ? end : end - 1);
}


void LyXText::cursorTop(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	setCursor(cur, 0, 0);
}


void LyXText::cursorBottom(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	setCursor(cur, cur.lastpar(), boost::prior(paragraphs().end())->size());
}


void LyXText::toggleFree(LCursor & cur, LyXFont const & font, bool toggleall)
{
	BOOST_ASSERT(this == cur.text());
	// If the mask is completely neutral, tell user
	if (font == LyXFont(LyXFont::ALL_IGNORE)) {
		// Could only happen with user style
		cur.message(_("No font change defined. "
			"Use Character under the Layout menu to define font change."));
		return;
	}

	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	CursorSlice resetCursor = cur.current();
	bool implicitSelection =
		font.language() == ignore_language
		&& font.number() == LyXFont::IGNORE
		&& selectWordWhenUnderCursor(cur, lyx::WHOLE_WORD_STRICT);

	// Set font
	setFont(cur, font, toggleall);

	// Implicit selections are cleared afterwards
	// and cursor is set to the original position.
	if (implicitSelection) {
		cur.clearSelection();
		cur.current() = resetCursor;
		cur.resetAnchor();
	}
}


string LyXText::getStringToIndex(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	CursorSlice const reset_cursor = cur.current();
	bool const implicitSelection =
		selectWordWhenUnderCursor(cur, lyx::PREVIOUS_WORD);

	string idxstring;
	if (!cur.selection())
		cur.message(_("Nothing to index!"));
	else if (cur.selBegin().par() != cur.selEnd().par())
		cur.message(_("Cannot index more than one paragraph!"));
	else
		idxstring = cur.selectionAsString(false);

	// Reset cursors to their original position.
	cur.current() = reset_cursor;
	cur.resetAnchor();

	// Clear the implicit selection.
	if (implicitSelection)
		cur.clearSelection();

	return idxstring;
}


// the DTP switches for paragraphs(). LyX will store them in the first
// physical paragraph. When a paragraph is broken, the top settings rest,
// the bottom settings are given to the new one. So I can make sure,
// they do not duplicate themself and you cannot play dirty tricks with
// them!

void LyXText::setParagraph(LCursor & cur,
	Spacing const & spacing, LyXAlignment align,
	string const & labelwidthstring, bool noindent)
{
	BOOST_ASSERT(cur.text());
	// make sure that the depth behind the selection are restored, too
	ParagraphList::iterator undopit = undoSpan(getPar(cur.selEnd()));
	recUndo(cur.selBegin().par(), parOffset(undopit) - 1);

	ParagraphList::reverse_iterator pit(getPar(cur.selEnd().par()));
	ParagraphList::reverse_iterator beg(getPar(cur.selBegin().par()));

	for (--pit; pit != beg; ++pit) {
		ParagraphParameters & params = pit->params();
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
	}

	redoParagraphs(getPar(cur.selBegin()), undopit);
}


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


namespace {

void incrementItemDepth(ParagraphList::iterator pit,
			ParagraphList::iterator first_pit)
{
	int const cur_labeltype = pit->layout()->labeltype;

	if (cur_labeltype != LABEL_ENUMERATE && cur_labeltype != LABEL_ITEMIZE)
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
	ParagraphList::iterator first_pit = paragraphs().begin();
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
			ParagraphList::iterator end = paragraphs().end();
			ParagraphList::iterator tmppit = pit;
			InsetBase * in = 0;
			bool isOK = false;
			while (tmppit != end && tmppit->inInset()
			       // the single '=' is intended below
			       && (in = tmppit->inInset()->owner()))
			{
				if (in->lyxCode() == InsetBase::FLOAT_CODE ||
				    in->lyxCode() == InsetBase::WRAP_CODE) {
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

				if (in->lyxCode() == InsetBase::FLOAT_CODE)
					type = static_cast<InsetFloat*>(in)->params().type;
				else if (in->lyxCode() == InsetBase::WRAP_CODE)
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


// Updates all counters.
void LyXText::updateCounters()
{
	// start over
	bv()->buffer()->params().getLyXTextClass().counters().reset();

	bool update_pos = false;
	
	ParagraphList::iterator beg = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
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
		if (oldLabel != newLabel) {
			redoParagraphInternal(pit);
			update_pos = true;
		}
		
	}
	if (update_pos)
		updateParPositions();
}


void LyXText::insertInset(LCursor & cur, InsetBase * inset)
{
	BOOST_ASSERT(this == cur.text());
	recordUndo(cur);
	freezeUndo();
	cur.paragraph().insertInset(cur.pos(), inset);
	// Just to rebreak and refresh correctly.
	// The character will not be inserted a second time
	insertChar(cur, Paragraph::META_INSET);
	// If we enter a highly editable inset the cursor should be before
	// the inset. After an undo LyX tries to call inset->edit(...)
	// and fails if the cursor is behind the inset and getInset
	// does not return the inset!
	if (isHighlyEditableInset(inset))
		cursorLeft(cur, true);
	unFreezeUndo();
}


void LyXText::cutSelection(LCursor & cur, bool doclear, bool realcut)
{
	BOOST_ASSERT(this == cur.text());
	// Stuff what we got on the clipboard. Even if there is no selection.

	// There is a problem with having the stuffing here in that the
	// larger the selection the slower LyX will get. This can be
	// solved by running the line below only when the selection has
	// finished. The solution used currently just works, to make it
	// faster we need to be more clever and probably also have more
	// calls to stuffClipboard. (Lgb)
	bv()->stuffClipboard(cur.selectionAsString(true));

	// This doesn't make sense, if there is no selection
	if (!cur.selection())
		return;

	// OK, we have a selection. This is always between cur.selBegin()
	// and cur.selEnd()

	// make sure that the depth behind the selection are restored, too
	recordUndoSelection(cur);
	ParagraphList::iterator begpit = getPar(cur.selBegin().par());
	ParagraphList::iterator endpit = getPar(cur.selEnd().par());
	ParagraphList::iterator undopit = undoSpan(endpit);

	int endpos = cur.selEnd().pos();

	BufferParams const & bufparams = bv()->buffer()->params();
	boost::tie(endpit, endpos) = realcut ?
		CutAndPaste::cutSelection(bufparams,
					  paragraphs(),
					  begpit, endpit,
					  cur.selBegin().pos(), endpos,
					  bufparams.textclass,
					  doclear)
		: CutAndPaste::eraseSelection(bufparams,
					      paragraphs(),
					      begpit, endpit,
					      cur.selBegin().pos(), endpos,
					      doclear);
	// sometimes necessary
	if (doclear)
		begpit->stripLeadingSpaces();

	redoParagraphs(begpit, undopit);
	// cutSelection can invalidate the cursor so we need to set
	// it anew. (Lgb)
	// we prefer the end for when tracking changes
	cur.pos() = endpos;
	cur.par() = parOffset(endpit);

	// need a valid cursor. (Lgb)
	cur.clearSelection();
	updateCounters();
}


void LyXText::copySelection(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	// stuff the selection onto the X clipboard, from an explicit copy request
	bv()->stuffClipboard(cur.selectionAsString(true));

	// this doesnt make sense, if there is no selection
	if (!cur.selection())
		return;

	// ok we have a selection. This is always between cur.selBegin()
	// and sel_end cursor

	// copy behind a space if there is one
	while (getPar(cur.selBegin())->size() > cur.selBegin().pos()
	       && getPar(cur.selBegin())->isLineSeparator(cur.selBegin().pos())
	       && (cur.selBegin().par() != cur.selEnd().par()
		   || cur.selBegin().pos() < cur.selEnd().pos()))
		cur.selBegin().pos(cur.selBegin().pos() + 1);

	CutAndPaste::copySelection(getPar(cur.selBegin().par()),
				   getPar(cur.selEnd().par()),
				   cur.selBegin().pos(), 
				   cur.selEnd().pos(),
				   bv()->buffer()->params().textclass);
}


void LyXText::pasteSelection(LCursor & cur, size_t sel_index)
{
	// this does not make sense, if there is nothing to paste
	if (!CutAndPaste::checkPastePossible())
		return;

	recordUndo(cur);

	ParagraphList::iterator endpit;
	PitPosPair ppp;

	ErrorList el;

	boost::tie(ppp, endpit) =
		CutAndPaste::pasteSelection(*bv()->buffer(),
					    paragraphs(),
					    getPar(cur.par()), cur.pos(),
					    bv()->buffer()->params().textclass,
					    sel_index, el);
	bufferErrors(*bv()->buffer(), el);
	bv()->showErrorList(_("Paste"));

	redoParagraphs(getPar(cur.par()), endpit);

	cur.clearSelection();
	cur.resetAnchor();
	setCursor(cur, parOffset(ppp.first), ppp.second);
	cur.setSelection();
	updateCounters();
}


void LyXText::setSelectionRange(LCursor & cur, lyx::pos_type length)
{
	if (!length)
		return;
	cur.resetAnchor();
	while (length--)
		cursorRight(cur, true);
	cur.setSelection();
}


// simple replacing. The font of the first selected character is used
void LyXText::replaceSelectionWithString(LCursor & cur, string const & str)
{
	recordUndo(cur);
	freezeUndo();

	// Get font setting before we cut
	pos_type pos = cur.selEnd().pos();
	LyXFont const font = getPar(cur.selBegin())
		->getFontSettings(bv()->buffer()->params(),
				  cur.selBegin().pos());

	// Insert the new string
	string::const_iterator cit = str.begin();
	string::const_iterator end = str.end();
	for (; cit != end; ++cit) {
		getPar(cur.selEnd())->insertChar(pos, (*cit), font);
		++pos;
	}

	// Cut the selection
	cutSelection(cur, true, false);
	unFreezeUndo();
}


// needed to insert the selection
void LyXText::insertStringAsLines(LCursor & cur, string const & str)
{
	ParagraphList::iterator pit = getPar(cur.par());
	ParagraphList::iterator endpit = boost::next(pit);
	pos_type pos = cursor().pos();
	recordUndo(cur);

	// only to be sure, should not be neccessary
	cur.clearSelection();
	bv()->buffer()->insertStringAsLines(pit, pos, current_font, str);

	redoParagraphs(getPar(cur.par()), endpit);
	cur.resetAnchor();
	setCursor(cur, cur.par(), pos);
	cur.setSelection();
}


// turn double CR to single CR, others are converted into one
// blank. Then insertStringAsLines is called
void LyXText::insertStringAsParagraphs(LCursor & cur, string const & str)
{
	string linestr = str;
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
		} else if (IsPrintable(linestr[i])) {
			newline_inserted = false;
		}
	}
	insertStringAsLines(cur, linestr);
}


bool LyXText::setCursor(LCursor & cur, par_type par, pos_type pos,
	bool setfont, bool boundary)
{
	CursorSlice old_cursor = cur.current();
	setCursorIntern(cur, par, pos, setfont, boundary);
	return deleteEmptyParagraphMechanism(cur.current(), old_cursor);
}


void LyXText::setCursor(CursorSlice & cur, par_type par,
	pos_type pos, bool boundary)
{
	BOOST_ASSERT(par != int(paragraphs().size()));

	cur.par(par);
	cur.pos(pos);
	cur.boundary(boundary);

	// no rows, no fun...
	if (paragraphs().begin()->rows.empty())
		return;

	// now some strict checking
	Paragraph & para = *getPar(par);
	Row const & row = *para.getRow(pos);
	pos_type const end = row.endpos();

	// None of these should happen, but we're scaredy-cats
	if (pos < 0) {
		lyxerr << "dont like -1" << endl;
		pos = 0;
		cur.pos(0);
		BOOST_ASSERT(false);
	} else if (pos > para.size()) {
		lyxerr << "dont like 1, pos: " << pos
		       << " size: " << para.size()
		       << " row.pos():" << row.pos()
		       << " par: " << par << endl;
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
		       << " size: " << para.size()
		       << " row.pos():" << row.pos()
		       << " par: " << par << endl;
		pos = row.pos();
		cur.pos(pos);
		BOOST_ASSERT(false);
	}
}


void LyXText::setCursorIntern(LCursor & cur,
	par_type par, pos_type pos, bool setfont, bool boundary)
{
	setCursor(cur.current(), par, pos, boundary);
	cur.x_target() = cursorX(cur.current());
	if (setfont)
		setCurrentFont(cur);
}


void LyXText::setCurrentFont(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	pos_type pos = cur.pos();
	ParagraphList::iterator pit = getPar(cur.par());

	if (cur.boundary() && pos > 0)
		--pos;

	if (pos > 0) {
		if (pos == cur.lastpos())
			--pos;
		else // potentional bug... BUG (Lgb)
			if (pit->isSeparator(pos)) {
				if (pos > cur.textRow().pos() &&
				    bidi.level(pos) % 2 ==
				    bidi.level(pos - 1) % 2)
					--pos;
				else if (pos + 1 < cur.lastpos())
					++pos;
			}
	}

	BufferParams const & bufparams = bv()->buffer()->params();
	current_font = pit->getFontSettings(bufparams, pos);
	real_current_font = getFont(pit, pos);

	if (cur.pos() == cur.lastpos()
	    && bidi.isBoundary(*bv()->buffer(), *pit, cur.pos())
	    && !cur.boundary()) {
		Language const * lang = pit->getParLanguage(bufparams);
		current_font.setLanguage(lang);
		current_font.setNumber(LyXFont::OFF);
		real_current_font.setLanguage(lang);
		real_current_font.setNumber(LyXFont::OFF);
	}
}


// x is an absolute screen coord
// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type LyXText::getColumnNearX(ParagraphList::iterator pit,
	Row const & row, int & x, bool & boundary) const
{
	x -= xo_;
	double tmpx             = row.x();
	double fill_separator   = row.fill_separator();
	double fill_hfill       = row.fill_hfill();
	double fill_label_hfill = row.fill_label_hfill();

	pos_type vc = row.pos();
	pos_type end = row.endpos();
	pos_type c = 0;
	LyXLayout_ptr const & layout = pit->layout();

	bool left_side = false;

	pos_type body_pos = pit->beginOfBody();
	double last_tmpx = tmpx;

	if (body_pos > 0 &&
	    (body_pos > end || !pit->isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// check for empty row
	if (vc == end) {
		x = int(tmpx) + xo_;
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
	bool const rtl = lastrow
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

	x = int(tmpx) + xo_;
	return c - row.pos();
}


// x,y are coordinates relative to this LyXText
void LyXText::setCursorFromCoordinates(LCursor & cur, int x, int y)
{
	CursorSlice old_cursor = cur.current();
	ParagraphList::iterator pit;
	Row const & row = *getRowNearY(y, pit);
	lyxerr << "hit row at: " << row.pos() << endl;
	bool bound = false;
	int xx = x + xo_; // getRowNearX get absolute x coords
	pos_type const pos = row.pos() + getColumnNearX(pit, row, xx, bound);
	cur.par() = parOffset(pit);
	cur.pos() = pos;
	cur.boundary() = bound;
	deleteEmptyParagraphMechanism(cur.current(), old_cursor);
}


// x,y are absolute screen coordinates
InsetBase * LyXText::editXY(LCursor & cur, int x, int y)
{
	ParagraphList::iterator pit;
	Row const & row = *getRowNearY(y - yo_, pit);
	bool bound = false;

	int xx = x; // is modified by getColumnNearX
	pos_type const pos = row.pos() + getColumnNearX(pit, row, xx, bound);
	cur.par() = parOffset(pit);
	cur.pos() = pos;
	cur.boundary() = bound;

	// try to descend into nested insets
	InsetBase * inset = checkInsetHit(x, y);
	if (!inset)
		return 0;

	// This should be just before or just behind the
	// cursor position set above.
	BOOST_ASSERT((pos != 0 && inset == pit->getInset(pos - 1))
	             || inset == pit->getInset(pos));
	// Make sure the cursor points to the position before
	// this inset.
	if (inset == pit->getInset(pos - 1))
		--cur.pos();
	return inset->editXY(cur, x, y);
}


bool LyXText::checkAndActivateInset(LCursor & cur, bool front)
{
	if (cur.pos() == cur.lastpos())
		return false;
	InsetBase * inset = cur.nextInset();
	if (!isHighlyEditableInset(inset))
		return false;
	inset->edit(cur, front);
	return true;
}


DispatchResult LyXText::moveRight(LCursor & cur)
{
	if (cur.paragraph().isRightToLeftPar(bv()->buffer()->params()))
		return moveLeftIntern(cur, false, true, false);
	else
		return moveRightIntern(cur, true, true, false);
}


DispatchResult LyXText::moveLeft(LCursor & cur)
{
	if (cur.paragraph().isRightToLeftPar(bv()->buffer()->params()))
		return moveRightIntern(cur, true, true, false);
	else
		return moveLeftIntern(cur, false, true, false);
}


DispatchResult LyXText::moveRightIntern(LCursor & cur, 
	bool front, bool activate_inset, bool selecting)
{
	if (cur.par() == cur.lastpar() && cur.pos() == cur.lastpos())
		return DispatchResult(false, FINISHED_RIGHT);
	if (activate_inset && checkAndActivateInset(cur, front))
		return DispatchResult(true, true);
	cursorRight(cur, true);
	if (!selecting)
		cur.clearSelection();
	return DispatchResult(true);
}


DispatchResult LyXText::moveLeftIntern(LCursor & cur,
	bool front, bool activate_inset, bool selecting)
{
	if (cur.par() == 0 && cur.pos() == 0)
		return DispatchResult(false, FINISHED);
	cursorLeft(cur, true);
	if (!selecting)
		cur.clearSelection();
	if (activate_inset && checkAndActivateInset(cur, front))
		return DispatchResult(true, true);
	return DispatchResult(true);
}


DispatchResult LyXText::moveUp(LCursor & cur)
{
	if (cur.par() == 0 && cur.crow() == 0)
		return DispatchResult(false, FINISHED_UP);
	cursorUp(cur, false);
	cur.clearSelection();
	return DispatchResult(true);
}


DispatchResult LyXText::moveDown(LCursor & cur)
{
	if (cur.par() == cur.lastpar() && cur.textRow().endpos() == cur.lastpos())
		return DispatchResult(false, FINISHED_DOWN);
	cursorDown(cur, false);
	cur.clearSelection();
	return DispatchResult(true);
}


bool LyXText::cursorLeft(LCursor & cur, bool internal)
{
	if (cur.pos() != 0) {
		bool boundary = cur.boundary();
		setCursor(cur, cur.par(), cur.pos() - 1, true, false);
		if (!internal && !boundary &&
		    bidi.isBoundary(*bv()->buffer(), cur.paragraph(), cur.pos() + 1))
			setCursor(cur, cur.par(), cur.pos() + 1, true, true);
		return true;
	}

	if (cur.par() != 0) {
		// steps into the paragraph above
		setCursor(cur, cur.par() - 1, getPar(cur.par() - 1)->size());
		return true;
	}

	return false;
}


bool LyXText::cursorRight(LCursor & cur, bool internal)
{
	if (!internal && cur.boundary()) {
		setCursor(cur, cur.par(), cur.pos(), true, false);
		return true;
	}

	if (cur.pos() != cur.lastpos()) {
		setCursor(cur, cur.par(), cur.pos() + 1, true, false);
		if (!internal && bidi.isBoundary(*bv()->buffer(), cur.paragraph(),
						 cur.pos()))
			setCursor(cur, cur.par(), cur.pos(), true, true);
		return true;
	}

	if (cur.par() != cur.lastpar()) {
		setCursor(cur, cur.par() + 1, 0);
		return true;
	}

	return false;
}


void LyXText::cursorUp(LCursor & cur, bool selecting)
{
	Row const & row = cur.textRow();
	int x = cur.x_target();
	int y = cursorY(cur.current()) - row.baseline() - 1;
	setCursorFromCoordinates(cur, x - xo_, y - yo_);

	if (!selecting) {
		InsetBase * inset_hit = checkInsetHit(cur.x_target(), y);
		if (inset_hit && isHighlyEditableInset(inset_hit))
			inset_hit->editXY(cur, cur.x_target(), y);
	}
}


void LyXText::cursorDown(LCursor & cur, bool selecting)
{
	Row const & row = cur.textRow();
	int x = cur.x_target();
	int y = cursorY(cur.current()) - row.baseline() + row.height() + 1;
	setCursorFromCoordinates(cur, x - xo_, y - yo_);

	if (!selecting) {
		InsetBase * inset_hit = checkInsetHit(cur.x_target(), y);
		if (inset_hit && isHighlyEditableInset(inset_hit))
			inset_hit->editXY(cur, cur.x_target(), y);
	}
}


void LyXText::cursorUpParagraph(LCursor & cur)
{
	if (cur.pos() > 0)
		setCursor(cur, cur.par(), 0);
	else if (cur.par() != 0)
		setCursor(cur, cur.par() - 1, 0);
}


void LyXText::cursorDownParagraph(LCursor & cur)
{
	if (cur.par() != cur.lastpar())
		setCursor(cur, cur.par() + 1, 0);
	else
		setCursor(cur, cur.par(), cur.lastpos());
}


// fix the cursor `cur' after a characters has been deleted at `where'
// position. Called by deleteEmptyParagraphMechanism
void LyXText::fixCursorAfterDelete(CursorSlice & cur, CursorSlice const & where)
{
	// do notheing if cursor is not in the paragraph where the
	// deletion occured,
	if (cur.par() != where.par())
		return;

	// if cursor position is after the deletion place update it
	if (cur.pos() > where.pos())
		--cur.pos();

	// check also if we don't want to set the cursor on a spot behind the
	// pagragraph because we erased the last character.
	if (cur.pos() > cur.lastpos())
		cur.pos() = cur.lastpos();
}


bool LyXText::deleteEmptyParagraphMechanism(CursorSlice & cur,
	CursorSlice const & old_cursor)
{
#warning Disabled as it crashes after the cursor data shift... (Andre)
	return false;

	// Would be wrong to delete anything if we have a selection.
	//if (cur.selection())
	//	return false;

#if 0
	// We allow all kinds of "mumbo-jumbo" when freespacing.
	ParagraphList::iterator const old_pit = getPar(old_cursor.par());
	if (old_pit->isFreeSpacing())
		return false;

	/* Ok I'll put some comments here about what is missing.
	   I have fixed BackSpace (and thus Delete) to not delete
	   double-spaces automagically. I have also changed Cut,
	   Copy and Paste to hopefully do some sensible things.
	   There are still some small problems that can lead to
	   double spaces stored in the document file or space at
	   the beginning of paragraphs(). This happens if you have
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
	if (old_cursor.par() != cur.par() || old_cursor.pos() != cur.pos()) {

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
// automated way in CursorSlice code. (JMarc 26/09/2001)
#endif
			// correct all cursors held by the LyXText
			fixCursorAfterDelete(cursor(), old_cursor);
			fixCursorAfterDelete(anchor(), old_cursor);
			return false;
		}
	}

	// don't delete anything if this is the ONLY paragraph!
	if (paragraphs().size() == 1)
		return false;

	// Do not delete empty paragraphs with keepempty set.
	if (old_pit->allowEmpty())
		return false;

	// only do our magic if we changed paragraph
	if (old_cursor.par() == cur.par())
		return false;

	// record if we have deleted a paragraph
	// we can't possibly have deleted a paragraph before this point
	bool deleted = false;

	if (old_pit->empty()
	    || (old_pit->size() == 1 && old_pit->isLineSeparator(0))) {
		// ok, we will delete something
		CursorSlice tmpcursor;

		deleted = true;

		bool selection_position_was_oldcursor_position =
			anchor().par() == old_cursor.par()
			&& anchor().pos() == old_cursor.pos();

		tmpcursor = cursor();
		cursor() = old_cursor; // that undo can restore the right cursor position

		ParagraphList::iterator endpit = boost::next(old_pit);
		while (endpit != paragraphs().end() && endpit->getDepth())
			++endpit;

		recUndo(parOffset(old_pit), parOffset(endpit) - 1);
		cursor() = tmpcursor;

		// delete old par
		paragraphs().erase(old_pit);
		// update cursor par offset
		--cur.par();
		redoParagraph();

		if (selection_position_was_oldcursor_position) {
			// correct selection
			bv()->resetAnchor();
		}
	}

	if (deleted)
		return true;

	if (old_pit->stripLeadingSpaces()) {
		redoParagraph(old_pit);
		bv()->resetAnchor();
	}
	return false;
#endif
}


ParagraphList & LyXText::paragraphs() const
{
	return const_cast<ParagraphList &>(paragraphs_);
}


void LyXText::recUndo(par_type first, par_type last) const
{
	recordUndo(bv()->cursor(), Undo::ATOMIC, first, last);
}


void LyXText::recUndo(par_type par) const
{
	recordUndo(bv()->cursor(), Undo::ATOMIC, par, par);
}


bool LyXText::isInInset() const
{
	return in_inset_;
}


bool LyXText::toggleInset(LCursor & cur)
{
	InsetBase * inset = cur.nextInset();
	// is there an editable inset at cursor position?
	if (!isEditableInset(inset))
		return false;
	cur.message(inset->editMessage());

	// do we want to keep this?? (JMarc)
	if (!isHighlyEditableInset(inset))
		recordUndo(cur);

	if (inset->isOpen())
		inset->close();
	else
		inset->open();
	return true;
}


int defaultRowHeight()
{
	return int(font_metrics::maxHeight(LyXFont(LyXFont::ALL_SANE)) *  1.2);
}
