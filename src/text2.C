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

using lyx::par_type;
using lyx::pos_type;
using lyx::support::bformat;

using std::endl;
using std::ostringstream;
using std::string;


LyXText::LyXText(BufferView * bv)
	: width_(0), maxwidth_(bv ? bv->workWidth() : 100), height_(0),
	  background_color_(LColor::background),
	  bv_owner(bv), xo_(0), yo_(0)
{}


void LyXText::init(BufferView * bv)
{
	BOOST_ASSERT(bv);
	bv_owner = bv;
	maxwidth_ = bv->workWidth();
	width_ = maxwidth_;
	height_ = 0;

	par_type const end = paragraphs().size();
	for (par_type pit = 0; pit != end; ++pit)
		pars_[pit].rows.clear();

	current_font = getFont(0, 0);
	redoParagraphs(0, end);
	updateCounters();
}


bool LyXText::isMainText() const
{
	return &bv()->buffer()->text() == this;
}


// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as Paragraph::getFont() in paragraph.C.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
LyXFont LyXText::getFont(par_type pit, pos_type pos) const
{
	BOOST_ASSERT(pos >= 0);

	LyXLayout_ptr const & layout = pars_[pit].layout();
#ifdef WITH_WARNINGS
#warning broken?
#endif
	BufferParams const & params = bv()->buffer()->params();
	pos_type const body_pos = pars_[pit].beginOfBody();

	// We specialize the 95% common case:
	if (!pars_[pit].getDepth()) {
		LyXFont f = pars_[pit].getFontSettings(params, pos);
		if (!isMainText())
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

	LyXFont font = pars_[pit].getFontSettings(params, pos);
	font.realize(layoutfont);

	if (!isMainText())
		font.realize(font_);

	// Realize with the fonts of lesser depth.
	//font.realize(outerFont(pit, paragraphs()));
	font.realize(defaultfont_);

	return font;
}


LyXFont LyXText::getLayoutFont(par_type pit) const
{
	LyXLayout_ptr const & layout = pars_[pit].layout();

	if (!pars_[pit].getDepth())
		return layout->resfont;

	LyXFont font = layout->font;
	// Realize with the fonts of lesser depth.
	//font.realize(outerFont(pit, paragraphs()));
	font.realize(defaultfont_);

	return font;
}


LyXFont LyXText::getLabelFont(par_type pit) const
{
	LyXLayout_ptr const & layout = pars_[pit].layout();

	if (!pars_[pit].getDepth())
		return layout->reslabelfont;

	LyXFont font = layout->labelfont;
	// Realize with the fonts of lesser depth.
	font.realize(outerFont(pit, paragraphs()));
	font.realize(defaultfont_);

	return font;
}


void LyXText::setCharFont(par_type pit, pos_type pos, LyXFont const & fnt)
{
	LyXFont font = fnt;
	LyXLayout_ptr const & layout = pars_[pit].layout();

	// Get concrete layout font to reduce against
	LyXFont layoutfont;

	if (pos < pars_[pit].beginOfBody())
		layoutfont = layout->labelfont;
	else
		layoutfont = layout->font;

	// Realize against environment font information
	if (pars_[pit].getDepth()) {
		par_type tp = pit;
		while (!layoutfont.resolved() &&
		       tp != par_type(paragraphs().size()) &&
		       pars_[tp].getDepth()) {
			tp = outerHook(tp, paragraphs());
			if (tp != par_type(paragraphs().size()))
				layoutfont.realize(pars_[tp].layout()->font);
		}
	}

	layoutfont.realize(defaultfont_);

	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	pars_[pit].setFont(pos, font);
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
par_type LyXText::undoSpan(par_type pit)
{
	par_type end = paragraphs().size();
	par_type nextpit = pit + 1;
	if (nextpit == end)
		return nextpit;
	//because of parindents
	if (!pars_[pit].getDepth())
		return boost::next(nextpit);
	//because of depth constrains
	for (; nextpit != end; ++pit, ++nextpit) {
		if (!pars_[pit].getDepth())
			break;
	}
	return nextpit;
}


par_type LyXText::setLayout(par_type start, par_type end, string const & layout)
{
	BOOST_ASSERT(start != end);
	par_type undopit = undoSpan(end - 1);
	recUndo(start, undopit - 1);

	BufferParams const & bufparams = bv()->buffer()->params();
	LyXLayout_ptr const & lyxlayout = bufparams.getLyXTextClass()[layout];

	for (par_type pit = start; pit != end; ++pit) {
		pars_[pit].applyLayout(lyxlayout);
		makeFontEntriesLayoutSpecific(bufparams, pars_[pit]);
		if (lyxlayout->margintype == MARGIN_MANUAL)
			pars_[pit].setLabelWidthString(lyxlayout->labelstring());
	}

	return undopit;
}


// set layout over selection and make a total rebreak of those paragraphs
void LyXText::setLayout(LCursor & cur, string const & layout)
{
	BOOST_ASSERT(this == cur.text());
	// special handling of new environment insets
	BufferView & bv = cur.bv();
	BufferParams const & params = bv.buffer()->params();
	LyXLayout_ptr const & lyxlayout = params.getLyXTextClass()[layout];
	if (lyxlayout->is_environment) {
		// move everything in a new environment inset
		lyxerr[Debug::DEBUG] << "setting layout " << layout << endl;
		bv.owner()->dispatch(FuncRequest(LFUN_HOME));
		bv.owner()->dispatch(FuncRequest(LFUN_ENDSEL));
		bv.owner()->dispatch(FuncRequest(LFUN_CUT));
		InsetBase * inset = new InsetEnvironment(params, layout);
		insertInset(cur, inset);
		//inset->edit(cur, true);
		//bv.owner()->dispatch(FuncRequest(LFUN_PASTE));
		return;
	}

	par_type start = cur.selBegin().par();
	par_type end = cur.selEnd().par() + 1;
	par_type endpit = setLayout(start, end, layout);
	redoParagraphs(start, endpit);
	updateCounters();
}


namespace {


void getSelectionSpan(LCursor & cur, par_type & beg, par_type & end)
{
	if (!cur.selection()) {
		beg = cur.par();
		end = cur.par() + 1;
	} else {
		beg = cur.selBegin().par();
		end = cur.selEnd().par() + 1;
	}
}


bool changeDepthAllowed(LyXText::DEPTH_CHANGE type,
	Paragraph const & par, int max_depth)
{
	if (par.layout()->labeltype == LABEL_BIBLIO)
		return false;
	int const depth = par.params().depth();
	if (type == LyXText::INC_DEPTH && depth < max_depth)
		return true;
	if (type == LyXText::DEC_DEPTH && depth > 0)
		return true;
	return false;
}


}


bool LyXText::changeDepthAllowed(LCursor & cur, DEPTH_CHANGE type) const
{
	BOOST_ASSERT(this == cur.text());
	par_type beg, end;
	getSelectionSpan(cur, beg, end);
	int max_depth = 0;
	if (beg != 0)
		max_depth = pars_[beg - 1].getMaxDepthAfter();

	for (par_type pit = beg; pit != end; ++pit) {
		if (::changeDepthAllowed(type, pars_[pit], max_depth))
			return true;
		max_depth = pars_[pit].getMaxDepthAfter();
	}
	return false;
}


void LyXText::changeDepth(LCursor & cur, DEPTH_CHANGE type)
{
	BOOST_ASSERT(this == cur.text());
	par_type beg, end;
	getSelectionSpan(cur, beg, end);
	recordUndoSelection(cur);

	int max_depth = 0;
	if (beg != 0)
		max_depth = pars_[beg - 1].getMaxDepthAfter();

	for (par_type pit = beg; pit != end; ++pit) {
		if (::changeDepthAllowed(type, pars_[pit], max_depth)) {
			int const depth = pars_[pit].params().depth();
			if (type == INC_DEPTH)
				pars_[pit].params().depth(depth + 1);
			else
				pars_[pit].params().depth(depth - 1);
		}
		max_depth = pars_[pit].getMaxDepthAfter();
	}
	// this handles the counter labels, and also fixes up
	// depth values for follow-on (child) paragraphs
	updateCounters();
}


// set font over selection
void LyXText::setFont(LCursor & cur, LyXFont const & font, bool toggleall)
{
	BOOST_ASSERT(this == cur.text());
	// if there is no selection just set the current_font
	if (!cur.selection()) {
		// Determine basis font
		LyXFont layoutfont;
		par_type pit = cur.par();
		if (cur.pos() < pars_[pit].beginOfBody())
			layoutfont = getLabelFont(pit);
		else
			layoutfont = getLayoutFont(pit);

		// Update current font
		real_current_font.update(font,
					 cur.buffer().params().language,
					 toggleall);

		// Reduce to implicit settings
		current_font = real_current_font;
		current_font.reduce(layoutfont);
		// And resolve it completely
		real_current_font.realize(layoutfont);

		return;
	}

	// Ok, we have a selection.
	recordUndoSelection(cur);

	par_type const beg = cur.selBegin().par();
	par_type const end = cur.selEnd().par();

	DocIterator pos = cur.selectionBegin();
	DocIterator posend = cur.selectionEnd();

	lyxerr[Debug::DEBUG] << "pos: " << pos << " posend: " << posend
			     << endl;

	BufferParams const & params = cur.buffer().params();

	// Don't use forwardChar here as posend might have
	// pos() == lastpos() and forwardChar would miss it.
	for (; pos != posend; pos.forwardPos()) {
		if (pos.pos() != pos.lastpos()) {
			LyXFont f = getFont(pos.par(), pos.pos());
			f.update(font, params.language, toggleall);
			setCharFont(pos.par(), pos.pos(), f);
		}
	}

	redoParagraphs(beg, end + 1);
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
	CursorSlice resetCursor = cur.top();
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
		cur.top() = resetCursor;
		cur.resetAnchor();
	}
}


string LyXText::getStringToIndex(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	CursorSlice const reset_cursor = cur.top();
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
	cur.top() = reset_cursor;
	cur.resetAnchor();

	// Clear the implicit selection.
	if (implicitSelection)
		cur.clearSelection();

	return idxstring;
}


void LyXText::setParagraph(LCursor & cur,
	Spacing const & spacing, LyXAlignment align,
	string const & labelwidthstring, bool noindent)
{
	BOOST_ASSERT(cur.text());
	// make sure that the depth behind the selection are restored, too
	par_type undopit = undoSpan(cur.selEnd().par());
	recUndo(cur.selBegin().par(), undopit - 1);

	for (par_type pit = cur.selBegin().par(), end = cur.selEnd().par();
			pit <= end; ++pit) {
		Paragraph & par = pars_[pit];
		ParagraphParameters & params = par.params();
		params.spacing(spacing);

		// does the layout allow the new alignment?
		LyXLayout_ptr const & layout = par.layout();

		if (align == LYX_ALIGN_LAYOUT)
			align = layout->align;
		if (align & layout->alignpossible) {
			if (align == layout->align)
				params.align(LYX_ALIGN_LAYOUT);
			else
				params.align(align);
		}
		par.setLabelWidthString(labelwidthstring);
		params.noindent(noindent);
	}

	redoParagraphs(cur.selBegin().par(), undopit);
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

void incrementItemDepth(ParagraphList & pars, par_type pit, par_type first_pit)
{
	int const cur_labeltype = pars[pit].layout()->labeltype;

	if (cur_labeltype != LABEL_ENUMERATE && cur_labeltype != LABEL_ITEMIZE)
		return;

	int const cur_depth = pars[pit].getDepth();

	par_type prev_pit = pit - 1;
	while (true) {
		int const prev_depth = pars[prev_pit].getDepth();
		int const prev_labeltype = pars[prev_pit].layout()->labeltype;
		if (prev_depth == 0 && cur_depth > 0) {
			if (prev_labeltype == cur_labeltype) {
				pars[pit].itemdepth = pars[prev_pit].itemdepth + 1;
			}
			break;
		} else if (prev_depth < cur_depth) {
			if (prev_labeltype == cur_labeltype) {
				pars[pit].itemdepth = pars[prev_pit].itemdepth + 1;
				break;
			}
		} else if (prev_depth == cur_depth) {
			if (prev_labeltype == cur_labeltype) {
				pars[pit].itemdepth = pars[prev_pit].itemdepth;
				break;
			}
		}
		if (prev_pit == first_pit)
			break;

		--prev_pit;
	}
}


void resetEnumCounterIfNeeded(ParagraphList & pars, par_type pit,
	par_type firstpit, Counters & counters)
{
	if (pit == firstpit)
		return;

	int const cur_depth = pars[pit].getDepth();
	par_type prev_pit = pit - 1;
	while (true) {
		int const prev_depth = pars[prev_pit].getDepth();
		int const prev_labeltype = pars[prev_pit].layout()->labeltype;
		if (prev_depth <= cur_depth) {
			if (prev_labeltype != LABEL_ENUMERATE) {
				switch (pars[pit].itemdepth) {
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
void LyXText::setCounter(Buffer const & buf, par_type pit)
{
	BufferParams const & bufparams = buf.params();
	LyXTextClass const & textclass = bufparams.getLyXTextClass();
	LyXLayout_ptr const & layout = pars_[pit].layout();
	par_type first_pit = 0;
	Counters & counters = textclass.counters();

	// Always reset
	pars_[pit].itemdepth = 0;

	if (pit == first_pit) {
		pars_[pit].params().appendix(pars_[pit].params().startOfAppendix());
	} else {
		pars_[pit].params().appendix(pars_[pit - 1].params().appendix());
		if (!pars_[pit].params().appendix() &&
		    pars_[pit].params().startOfAppendix()) {
			pars_[pit].params().appendix(true);
			textclass.counters().reset();
		}

		// Maybe we have to increment the item depth.
		incrementItemDepth(pars_, pit, first_pit);
	}

	// erase what was there before
	pars_[pit].params().labelString(string());

	if (layout->margintype == MARGIN_MANUAL) {
		if (pars_[pit].params().labelWidthString().empty())
			pars_[pit].setLabelWidthString(layout->labelstring());
	} else {
		pars_[pit].setLabelWidthString(string());
	}

	// is it a layout that has an automatic label?
	if (layout->labeltype == LABEL_COUNTER) {
		BufferParams const & bufparams = buf.params();
		LyXTextClass const & textclass = bufparams.getLyXTextClass();
		counters.step(layout->counter);
		string label = expandLabel(textclass, layout, pars_[pit].params().appendix());
		pars_[pit].params().labelString(label);
	} else if (layout->labeltype == LABEL_ITEMIZE) {
		// At some point of time we should do something more
		// clever here, like:
		//   pars_[pit].params().labelString(
		//    bufparams.user_defined_bullet(pars_[pit].itemdepth).getText());
		// for now, use a simple hardcoded label
		string itemlabel;
		switch (pars_[pit].itemdepth) {
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

		pars_[pit].params().labelString(itemlabel);
	} else if (layout->labeltype == LABEL_ENUMERATE) {
		// Maybe we have to reset the enumeration counter.
		resetEnumCounterIfNeeded(pars_, pit, first_pit, counters);

		// FIXME
		// Yes I know this is a really, really! bad solution
		// (Lgb)
		string enumcounter = "enum";

		switch (pars_[pit].itemdepth) {
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

		pars_[pit].params().labelString(counters.enumLabel(enumcounter));
	} else if (layout->labeltype == LABEL_BIBLIO) {// ale970302
		counters.step("bibitem");
		int number = counters.value("bibitem");
		if (pars_[pit].bibitem()) {
			pars_[pit].bibitem()->setCounter(number);
			pars_[pit].params().labelString(layout->labelstring());
		}
		// In biblio should't be following counters but...
	} else {
		string s = buf.B_(layout->labelstring());

		// the caption hack:
		if (layout->labeltype == LABEL_SENSITIVE) {
			par_type end = paragraphs().size();
			par_type tmppit = pit;
			InsetBase * in = 0;
			bool isOK = false;
			while (tmppit != end) {
				in = pars_[tmppit].inInset();
				if (in->lyxCode() == InsetBase::FLOAT_CODE ||
				    in->lyxCode() == InsetBase::WRAP_CODE) {
					isOK = true;
					break;
				} else {
					Paragraph const * owner = &ownerPar(buf, in);
					tmppit = first_pit;
					for ( ; tmppit != end; ++tmppit)
						if (&pars_[tmppit] == owner)
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
		pars_[pit].params().labelString(s);

	}
}


// Updates all counters.
void LyXText::updateCounters()
{
	// start over
	bv()->buffer()->params().getLyXTextClass().counters().reset();

	bool update_pos = false;

	par_type end = paragraphs().size();
	for (par_type pit = 0; pit != end; ++pit) {
		string const oldLabel = pars_[pit].params().labelString();
		size_t maxdepth = 0;
		if (pit != 0)
			maxdepth = pars_[pit - 1].getMaxDepthAfter();

		if (pars_[pit].params().depth() > maxdepth)
			pars_[pit].params().depth(maxdepth);

		// setCounter can potentially change the labelString.
		setCounter(*bv()->buffer(), pit);
		string const & newLabel = pars_[pit].params().labelString();
		if (oldLabel != newLabel) {
			//lyxerr[Debug::DEBUG] << "changing labels: old: " << oldLabel << " new: "
			//	<< newLabel << endl;
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
	BOOST_ASSERT(inset);
	cur.paragraph().insertInset(cur.pos(), inset);
	redoParagraph(cur);
}


// needed to insert the selection
void LyXText::insertStringAsLines(LCursor & cur, string const & str)
{
	par_type pit = cur.par();
	par_type endpit = cur.par() + 1;
	pos_type pos = cur.pos();
	recordUndo(cur);

	// only to be sure, should not be neccessary
	cur.clearSelection();
	cur.buffer().insertStringAsLines(pars_, pit, pos, current_font, str);

	redoParagraphs(cur.par(), endpit);
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
	LCursor old = cur;
	setCursorIntern(cur, par, pos, setfont, boundary);
	return deleteEmptyParagraphMechanism(cur, old);
}


void LyXText::setCursor(CursorSlice & cur, par_type par,
	pos_type pos, bool boundary)
{
	BOOST_ASSERT(par != int(paragraphs().size()));

	cur.par() = par;
	cur.pos() = pos;
	cur.boundary() = boundary;

	// no rows, no fun...
	if (paragraphs().begin()->rows.empty())
		return;

	// now some strict checking
	Paragraph & para = getPar(par);
	Row const & row = *para.getRow(pos);
	pos_type const end = row.endpos();

	// None of these should happen, but we're scaredy-cats
	if (pos < 0) {
		lyxerr << "dont like -1" << endl;
		BOOST_ASSERT(false);
	}

	if (pos > para.size()) {
		lyxerr << "dont like 1, pos: " << pos
		       << " size: " << para.size()
		       << " row.pos():" << row.pos()
		       << " par: " << par << endl;
		BOOST_ASSERT(false);
	}

	if (pos > end) {
		lyxerr << "dont like 2, pos: " << pos
		       << " size: " << para.size()
		       << " row.pos():" << row.pos()
		       << " par: " << par << endl;
		// This shouldn't happen.
		BOOST_ASSERT(false);
	}

	if (pos < row.pos()) {
		lyxerr << "dont like 3 please report pos:" << pos
		       << " size: " << para.size()
		       << " row.pos():" << row.pos()
		       << " par: " << par << endl;
		BOOST_ASSERT(false);
	}
}


void LyXText::setCursorIntern(LCursor & cur,
	par_type par, pos_type pos, bool setfont, bool boundary)
{
	setCursor(cur.top(), par, pos, boundary);
	cur.x_target() = cursorX(cur.top());
	if (setfont)
		setCurrentFont(cur);
}


void LyXText::setCurrentFont(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	pos_type pos = cur.pos();
	par_type pit = cur.par();

	if (cur.boundary() && pos > 0)
		--pos;

	if (pos > 0) {
		if (pos == cur.lastpos())
			--pos;
		else // potentional bug... BUG (Lgb)
			if (pars_[pit].isSeparator(pos)) {
				if (pos > cur.textRow().pos() &&
				    bidi.level(pos) % 2 ==
				    bidi.level(pos - 1) % 2)
					--pos;
				else if (pos + 1 < cur.lastpos())
					++pos;
			}
	}

	BufferParams const & bufparams = cur.buffer().params();
	current_font = pars_[pit].getFontSettings(bufparams, pos);
	real_current_font = getFont(pit, pos);

	if (cur.pos() == cur.lastpos()
	    && bidi.isBoundary(cur.buffer(), pars_[pit], cur.pos())
	    && !cur.boundary()) {
		Language const * lang = pars_[pit].getParLanguage(bufparams);
		current_font.setLanguage(lang);
		current_font.setNumber(LyXFont::OFF);
		real_current_font.setLanguage(lang);
		real_current_font.setNumber(LyXFont::OFF);
	}
}


// x is an absolute screen coord
// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type LyXText::getColumnNearX(par_type pit,
	Row const & row, int & x, bool & boundary) const
{
	x -= xo_;
	RowMetrics const r = computeRowMetrics(pit, row);

	pos_type vc = row.pos();
	pos_type end = row.endpos();
	pos_type c = 0;
	LyXLayout_ptr const & layout = pars_[pit].layout();

	bool left_side = false;

	pos_type body_pos = pars_[pit].beginOfBody();

	double tmpx = r.x;
	double last_tmpx = tmpx;

	if (body_pos > 0 &&
	    (body_pos > end || !pars_[pit].isLineSeparator(body_pos - 1)))
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
			tmpx += r.label_hfill +
				font_metrics::width(layout->labelsep, getLabelFont(pit));
			if (pars_[pit].isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(pit, body_pos - 1);
		}

		if (hfillExpansion(pars_[pit], row, c)) {
			tmpx += singleWidth(pit, c);
			if (c >= body_pos)
				tmpx += r.hfill;
			else
				tmpx += r.label_hfill;
		} else if (pars_[pit].isSeparator(c)) {
			tmpx += singleWidth(pit, c);
			if (c >= body_pos)
				tmpx += r.separator;
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
	bool const lastrow = lyxrc.rtl_support && row.endpos() == pars_[pit].size();

	// If lastrow is false, we don't need to compute
	// the value of rtl.
	bool const rtl = lastrow ? isRTL(pars_[pit]) : false;
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
			boundary = bidi.isBoundary(*bv()->buffer(), pars_[pit], c);
		}
	}

	if (row.pos() < end && c >= end && pars_[pit].isNewline(end - 1)) {
		if (bidi.level(end -1) % 2 == 0)
			tmpx -= singleWidth(pit, end - 1);
		else
			tmpx += singleWidth(pit, end - 1);
		c = end - 1;
	}

	x = int(tmpx) + xo_;
	return c - row.pos();
}


// x,y are absolute coordinates
void LyXText::setCursorFromCoordinates(LCursor & cur, int x, int y)
{
	x -= xo_;
	y -= yo_;
	par_type pit;
	Row const & row = getRowNearY(y, pit);
	lyxerr[Debug::DEBUG] << "setCursorFromCoordinates:: hit row at: "
			     << row.pos() << endl;
	bool bound = false;
	int xx = x + xo_; // getRowNearX get absolute x coords
	pos_type const pos = row.pos() + getColumnNearX(pit, row, xx, bound);
	setCursor(cur, pit, pos, true, bound);
}


// x,y are absolute screen coordinates
InsetBase * LyXText::editXY(LCursor & cur, int x, int y)
{
	par_type pit;
	Row const & row = getRowNearY(y - yo_, pit);
	bool bound = false;

	int xx = x; // is modified by getColumnNearX
	pos_type const pos = row.pos() + getColumnNearX(pit, row, xx, bound);
	cur.par() = pit;
	cur.pos() = pos;
	cur.boundary() = bound;

	// try to descend into nested insets
	InsetBase * inset = checkInsetHit(x, y);
	lyxerr[Debug::DEBUG] << "inset " << inset << " hit at x: " << x << " y: " << y << endl;
	if (!inset)
		return 0;

	// This should be just before or just behind the
	// cursor position set above.
	BOOST_ASSERT((pos != 0 && inset == pars_[pit].getInset(pos - 1))
	             || inset == pars_[pit].getInset(pos));
	// Make sure the cursor points to the position before
	// this inset.
	if (inset == pars_[pit].getInset(pos - 1))
		--cur.pos();
	return inset->editXY(cur, x, y);
}


bool LyXText::checkAndActivateInset(LCursor & cur, bool front)
{
	if (cur.selection())
		return false;
	if (cur.pos() == cur.lastpos())
		return false;
	InsetBase * inset = cur.nextInset();
	if (!isHighlyEditableInset(inset))
		return false;
	inset->edit(cur, front);
	return true;
}


void LyXText::cursorLeft(LCursor & cur)
{
	if (cur.pos() != 0) {
		bool boundary = cur.boundary();
		setCursor(cur, cur.par(), cur.pos() - 1, true, false);
		if (!checkAndActivateInset(cur, false)) {
			if (false && !boundary &&
					bidi.isBoundary(cur.buffer(), cur.paragraph(), cur.pos() + 1))
				setCursor(cur, cur.par(), cur.pos() + 1, true, true);
		}
		return;
	}

	if (cur.par() != 0) {
		// steps into the paragraph above
		setCursor(cur, cur.par() - 1, getPar(cur.par() - 1).size());
	}
}


void LyXText::cursorRight(LCursor & cur)
{
	if (false && cur.boundary()) {
		setCursor(cur, cur.par(), cur.pos(), true, false);
		return;
	}

	if (cur.pos() != cur.lastpos()) {
		if (!checkAndActivateInset(cur, true)) {
			setCursor(cur, cur.par(), cur.pos() + 1, true, false);
			if (false && bidi.isBoundary(cur.buffer(), cur.paragraph(),
							 cur.pos()))
				setCursor(cur, cur.par(), cur.pos(), true, true);
		}
		return;
	}

	if (cur.par() != cur.lastpar())
		setCursor(cur, cur.par() + 1, 0);
}


void LyXText::cursorUp(LCursor & cur)
{
	Row const & row = cur.textRow();
	int x = cur.x_target();
	int y = cursorY(cur.top()) - row.baseline() - 1;
	setCursorFromCoordinates(cur, x, y);

	if (!cur.selection()) {
		InsetBase * inset_hit = checkInsetHit(cur.x_target(), y);
		if (inset_hit && isHighlyEditableInset(inset_hit))
			inset_hit->editXY(cur, cur.x_target(), y);
	}
}


void LyXText::cursorDown(LCursor & cur)
{
	Row const & row = cur.textRow();
	int x = cur.x_target();
	int y = cursorY(cur.top()) - row.baseline() + row.height() + 1;
	setCursorFromCoordinates(cur, x, y);

	if (!cur.selection()) {
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


bool LyXText::deleteEmptyParagraphMechanism(LCursor & cur, LCursor const & old)
{
	BOOST_ASSERT(cur.size() == old.size());
	// Would be wrong to delete anything if we have a selection.
	if (cur.selection())
		return false;

	//lyxerr[Debug::DEBUG] << "DEPM: cur:\n" << cur << "old:\n" << old << endl;
	Paragraph const & oldpar = pars_[old.par()];

	// We allow all kinds of "mumbo-jumbo" when freespacing.
	if (oldpar.isFreeSpacing())
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

	// If old.pos() == 0 and old.pos()(1) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	// If old.pos() == 1 and old.pos()(0) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	// If the chars around the old cursor were spaces, delete one of them.
	if (old.par() != cur.par() || old.pos() != cur.pos()) {

		// Only if the cursor has really moved.
		if (old.pos() > 0
		    && old.pos() < oldpar.size()
		    && oldpar.isLineSeparator(old.pos())
		    && oldpar.isLineSeparator(old.pos() - 1)) {
			pars_[old.par()].erase(old.pos() - 1);
#ifdef WITH_WARNINGS
#warning This will not work anymore when we have multiple views of the same buffer
// In this case, we will have to correct also the cursors held by
// other bufferviews. It will probably be easier to do that in a more
// automated way in CursorSlice code. (JMarc 26/09/2001)
#endif
			// correct all cursor parts
			fixCursorAfterDelete(cur.top(), old.top());
#ifdef WITH_WARNINGS
#warning DEPM, look here
#endif
			//fixCursorAfterDelete(cur.anchor(), old.top());
			return false;
		}
	}

	// only do our magic if we changed paragraph
	if (old.par() == cur.par())
		return false;

	// don't delete anything if this is the ONLY paragraph!
	if (pars_.size() == 1)
		return false;

	// Do not delete empty paragraphs with keepempty set.
	if (oldpar.allowEmpty())
		return false;

	// record if we have deleted a paragraph
	// we can't possibly have deleted a paragraph before this point
	bool deleted = false;

	if (oldpar.empty() || (oldpar.size() == 1 && oldpar.isLineSeparator(0))) {
		// ok, we will delete something
		CursorSlice tmpcursor;

		deleted = true;

		bool selection_position_was_oldcursor_position =
			cur.anchor().par() == old.par() && cur.anchor().pos() == old.pos();

		// This is a bit of a overkill. We change the old and the cur par
		// at max, certainly not everything in between...
		recUndo(old.par(), cur.par());

		// Delete old par.
		pars_.erase(pars_.begin() + old.par());

		// Update cursor par offset if necessary.
		// Some 'iterator registration' would be nice that takes care of
		// such events. Maybe even signal/slot?
		if (cur.par() > old.par())
			--cur.par();
#ifdef WITH_WARNINGS
#warning DEPM, look here
#endif
//		if (cur.anchor().par() > old.par())
//			--cur.anchor().par();

		if (selection_position_was_oldcursor_position) {
			// correct selection
			cur.resetAnchor();
		}
	}

	if (deleted)
		return true;

	if (pars_[old.par()].stripLeadingSpaces())
		cur.resetAnchor();

	return false;
}


ParagraphList & LyXText::paragraphs() const
{
	return const_cast<ParagraphList &>(pars_);
}


void LyXText::recUndo(par_type first, par_type last) const
{
	recordUndo(bv()->cursor(), Undo::ATOMIC, first, last);
}


void LyXText::recUndo(par_type par) const
{
	recordUndo(bv()->cursor(), Undo::ATOMIC, par, par);
}


int defaultRowHeight()
{
	return int(font_metrics::maxHeight(LyXFont(LyXFont::ALL_SANE)) *  1.2);
}
