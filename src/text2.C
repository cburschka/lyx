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
#include "coordcache.h"
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

#include <sstream>

using lyx::pit_type;
using lyx::pos_type;
using lyx::support::bformat;

using std::endl;
using std::ostringstream;
using std::string;


LyXText::LyXText(BufferView * bv)
	: maxwidth_(bv ? bv->workWidth() : 100),
	  background_color_(LColor::background),
	  bv_owner(bv),
	  autoBreakRows_(false)
{}


void LyXText::init(BufferView * bv)
{
	BOOST_ASSERT(bv);
	bv_owner = bv;
	maxwidth_ = bv->workWidth();
	dim_.wid = maxwidth_;
	dim_.asc = 10;
	dim_.des = 10;

	pit_type const end = paragraphs().size();
	for (pit_type pit = 0; pit != end; ++pit)
		pars_[pit].rows().clear();

	current_font = getFont(pars_[0], 0);
	updateCounters();
}


bool LyXText::isMainText() const
{
	return &bv()->buffer()->text() == this;
}


//takes screen x,y coordinates
InsetBase * LyXText::checkInsetHit(int x, int y) const
{
	pit_type pit = getPitNearY(y);
	BOOST_ASSERT(pit != -1);

	Paragraph const & par = pars_[pit];

	lyxerr << "checkInsetHit: x: " << x << " y: " << y << endl;
	lyxerr << "  pit: " << pit << endl;
	InsetList::const_iterator iit = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for (; iit != iend; ++iit) {
		InsetBase * inset = iit->inset;
#if 1
		lyxerr << "examining inset " << inset << endl;
		if (theCoords.insets_.has(inset))
			lyxerr
				<< " xo: " << inset->xo() << "..."
				<< inset->xo() + inset->width()
				<< " yo: " << inset->yo() - inset->ascent()
				<< "..."
				<< inset->yo() + inset->descent() << endl;
		else
			lyxerr << " inset has no cached position" << endl;
#endif
		if (inset->covers(x, y)) {
			lyxerr << "Hit inset: " << inset << endl;
			return inset;
		}
	}
	lyxerr << "No inset hit. " << endl;
	return 0;
}



// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as Paragraph::getFont() in paragraph.C.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
LyXFont LyXText::getFont(Paragraph const & par, pos_type const pos) const
{
	BOOST_ASSERT(pos >= 0);

	LyXLayout_ptr const & layout = par.layout();
#ifdef WITH_WARNINGS
#warning broken?
#endif
	BufferParams const & params = bv()->buffer()->params();
	pos_type const body_pos = par.beginOfBody();

	// We specialize the 95% common case:
	if (!par.getDepth()) {
		LyXFont f = par.getFontSettings(params, pos);
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

	LyXFont font = par.getFontSettings(params, pos);
	font.realize(layoutfont);

	if (!isMainText())
		font.realize(font_);

	// Realize with the fonts of lesser depth.
	font.realize(defaultfont_);

	return font;
}


LyXFont LyXText::getLayoutFont(pit_type const pit) const
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


LyXFont LyXText::getLabelFont(Paragraph const & par) const
{
	LyXLayout_ptr const & layout = par.layout();

	if (!par.getDepth())
		return layout->reslabelfont;

	LyXFont font = layout->labelfont;
	// Realize with the fonts of lesser depth.
	font.realize(defaultfont_);

	return font;
}


void LyXText::setCharFont(pit_type pit, pos_type pos, LyXFont const & fnt)
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
		pit_type tp = pit;
		while (!layoutfont.resolved() &&
		       tp != pit_type(paragraphs().size()) &&
		       pars_[tp].getDepth()) {
			tp = outerHook(tp, paragraphs());
			if (tp != pit_type(paragraphs().size()))
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
pit_type LyXText::undoSpan(pit_type pit)
{
	pit_type end = paragraphs().size();
	pit_type nextpit = pit + 1;
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


pit_type LyXText::setLayout(pit_type start, pit_type end, string const & layout)
{
	BOOST_ASSERT(start != end);
	pit_type undopit = undoSpan(end - 1);
	recUndo(start, undopit - 1);

	BufferParams const & bufparams = bv()->buffer()->params();
	LyXLayout_ptr const & lyxlayout = bufparams.getLyXTextClass()[layout];

	for (pit_type pit = start; pit != end; ++pit) {
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

	pit_type start = cur.selBegin().pit();
	pit_type end = cur.selEnd().pit() + 1;
	setLayout(start, end, layout);
	updateCounters();
}


namespace {


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
	pit_type const beg = cur.selBegin().pit();
	pit_type const end = cur.selEnd().pit() + 1;
	int max_depth = (beg != 0 ? pars_[beg - 1].getMaxDepthAfter() : 0);

	for (pit_type pit = beg; pit != end; ++pit) {
		if (::changeDepthAllowed(type, pars_[pit], max_depth))
			return true;
		max_depth = pars_[pit].getMaxDepthAfter();
	}
	return false;
}


void LyXText::changeDepth(LCursor & cur, DEPTH_CHANGE type)
{
	BOOST_ASSERT(this == cur.text());
	pit_type const beg = cur.selBegin().pit();
	pit_type const end = cur.selEnd().pit() + 1;
	recordUndoSelection(cur);
	int max_depth = (beg != 0 ? pars_[beg - 1].getMaxDepthAfter() : 0);

	for (pit_type pit = beg; pit != end; ++pit) {
		Paragraph & par = pars_[pit];
		if (::changeDepthAllowed(type, par, max_depth)) {
			int const depth = par.params().depth();
			if (type == INC_DEPTH)
				par.params().depth(depth + 1);
			else
				par.params().depth(depth - 1);
		}
		max_depth = par.getMaxDepthAfter();
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
		pit_type pit = cur.pit();
		if (cur.pos() < pars_[pit].beginOfBody())
			layoutfont = getLabelFont(pars_[pit]);
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

	DocIterator dit = cur.selectionBegin();
	DocIterator ditend = cur.selectionEnd();

	BufferParams const & params = cur.buffer().params();

	// Don't use forwardChar here as ditend might have
	// pos() == lastpos() and forwardChar would miss it.
	for (; dit != ditend; dit.forwardPos()) {
		if (dit.pos() != dit.lastpos()) {
			LyXFont f = getFont(dit.paragraph(), dit.pos());
			f.update(font, params.language, toggleall);
			setCharFont(dit.pit(), dit.pos(), f);
		}
	}
}


// the cursor set functions have a special mechanism. When they
// realize you left an empty paragraph, they will delete it.

void LyXText::cursorHome(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	setCursor(cur, cur.pit(), cur.textRow().pos());
}


void LyXText::cursorEnd(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	// if not on the last row of the par, put the cursor before
	// the final space
// FIXME: does this final space exist?
	pos_type const end = cur.textRow().endpos();
	setCursor(cur, cur.pit(), end == cur.lastpos() ? end : end - 1);
}


void LyXText::cursorTop(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	setCursor(cur, 0, 0);
}


void LyXText::cursorBottom(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	setCursor(cur, cur.lastpit(), boost::prior(paragraphs().end())->size());
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
	else if (cur.selBegin().pit() != cur.selEnd().pit())
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
	pit_type undopit = undoSpan(cur.selEnd().pit());
	recUndo(cur.selBegin().pit(), undopit - 1);

	for (pit_type pit = cur.selBegin().pit(), end = cur.selEnd().pit();
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

void incrementItemDepth(ParagraphList & pars, pit_type pit, pit_type first_pit)
{
	int const cur_labeltype = pars[pit].layout()->labeltype;

	if (cur_labeltype != LABEL_ENUMERATE && cur_labeltype != LABEL_ITEMIZE)
		return;

	int const cur_depth = pars[pit].getDepth();

	pit_type prev_pit = pit - 1;
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


void resetEnumCounterIfNeeded(ParagraphList & pars, pit_type pit,
	pit_type firstpit, Counters & counters)
{
	if (pit == firstpit)
		return;

	int const cur_depth = pars[pit].getDepth();
	pit_type prev_pit = pit - 1;
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
void LyXText::setCounter(Buffer const & buf, pit_type pit)
{
	Paragraph & par = pars_[pit];
	BufferParams const & bufparams = buf.params();
	LyXTextClass const & textclass = bufparams.getLyXTextClass();
	LyXLayout_ptr const & layout = par.layout();
	Counters & counters = textclass.counters();

	// Always reset
	par.itemdepth = 0;

	if (pit == 0) {
		par.params().appendix(par.params().startOfAppendix());
	} else {
		par.params().appendix(pars_[pit - 1].params().appendix());
		if (!par.params().appendix() &&
		    par.params().startOfAppendix()) {
			par.params().appendix(true);
			textclass.counters().reset();
		}

		// Maybe we have to increment the item depth.
		incrementItemDepth(pars_, pit, 0);
	}

	// erase what was there before
	par.params().labelString(string());

	if (layout->margintype == MARGIN_MANUAL) {
		if (par.params().labelWidthString().empty())
			par.setLabelWidthString(layout->labelstring());
	} else {
		par.setLabelWidthString(string());
	}

	// is it a layout that has an automatic label?
	if (layout->labeltype == LABEL_COUNTER) {
		BufferParams const & bufparams = buf.params();
		LyXTextClass const & textclass = bufparams.getLyXTextClass();
		counters.step(layout->counter);
		string label = expandLabel(textclass, layout, par.params().appendix());
		par.params().labelString(label);
	} else if (layout->labeltype == LABEL_ITEMIZE) {
		// At some point of time we should do something more
		// clever here, like:
		//   par.params().labelString(
		//    bufparams.user_defined_bullet(par.itemdepth).getText());
		// for now, use a simple hardcoded label
		string itemlabel;
		switch (par.itemdepth) {
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

		par.params().labelString(itemlabel);
	} else if (layout->labeltype == LABEL_ENUMERATE) {
		// Maybe we have to reset the enumeration counter.
		resetEnumCounterIfNeeded(pars_, pit, 0, counters);

		// FIXME
		// Yes I know this is a really, really! bad solution
		// (Lgb)
		string enumcounter = "enum";

		switch (par.itemdepth) {
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

		par.params().labelString(counters.enumLabel(enumcounter));
	} else if (layout->labeltype == LABEL_BIBLIO) {// ale970302
		counters.step("bibitem");
		int number = counters.value("bibitem");
		if (par.bibitem()) {
			par.bibitem()->setCounter(number);
			par.params().labelString(layout->labelstring());
		}
		// In biblio should't be following counters but...
	} else {
		string s = buf.B_(layout->labelstring());

		// the caption hack:
		if (layout->labeltype == LABEL_SENSITIVE) {
			pit_type end = paragraphs().size();
			pit_type tmppit = pit;
			InsetBase * in = 0;
			bool isOK = false;
			while (tmppit != end) {
				in = pars_[tmppit].inInset();
				if (in->lyxCode() == InsetBase::FLOAT_CODE ||
				    in->lyxCode() == InsetBase::WRAP_CODE) {
					isOK = true;
					break;
				}
#ifdef WITH_WARNINGS
#warning replace this code by something that works
// This code does not work because we have currently no way to move up
// in the hierarchy of insets (JMarc 16/08/2004)
#endif
#if 0
/* I think this code is supposed to be useful when one has a caption
 * in a minipage in a figure inset. We need to go up to be able to see
 * that the caption should use "Figure" as label
 */
				else {
					Paragraph const * owner = &ownerPar(buf, in);
					tmppit = 0;
					for ( ; tmppit != end; ++tmppit)
						if (&pars_[tmppit] == owner)
							break;
				}
#else
				++tmppit;
#endif
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
		par.params().labelString(s);

	}
}


// Updates all counters.
void LyXText::updateCounters()
{
	// start over
	bv()->buffer()->params().getLyXTextClass().counters().reset();

	bool update_pos = false;

	pit_type end = paragraphs().size();
	for (pit_type pit = 0; pit != end; ++pit) {
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
			update_pos = true;
		}
	}
}


// this really should just insert the inset and not move the cursor.
void LyXText::insertInset(LCursor & cur, InsetBase * inset)
{
	BOOST_ASSERT(this == cur.text());
	BOOST_ASSERT(inset);
	cur.paragraph().insertInset(cur.pos(), inset);
}


// needed to insert the selection
void LyXText::insertStringAsLines(LCursor & cur, string const & str)
{
	pit_type pit = cur.pit();
	pos_type pos = cur.pos();
	recordUndo(cur);

	// only to be sure, should not be neccessary
	cur.clearSelection();
	cur.buffer().insertStringAsLines(pars_, pit, pos, current_font, str,
		autoBreakRows_);

	cur.resetAnchor();
	setCursor(cur, cur.pit(), pos);
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


bool LyXText::setCursor(LCursor & cur, pit_type par, pos_type pos,
	bool setfont, bool boundary)
{
	LCursor old = cur;
	setCursorIntern(cur, par, pos, setfont, boundary);
	return deleteEmptyParagraphMechanism(cur, old);
}


void LyXText::setCursor(CursorSlice & cur, pit_type par,
	pos_type pos, bool boundary)
{
	BOOST_ASSERT(par != int(paragraphs().size()));
	cur.pit() = par;
	cur.pos() = pos;
	cur.boundary() = boundary;

	// now some strict checking
	Paragraph & para = getPar(par);

	// None of these should happen, but we're scaredy-cats
	if (pos < 0) {
		lyxerr << "dont like -1" << endl;
		BOOST_ASSERT(false);
	}

	if (pos > para.size()) {
		lyxerr << "dont like 1, pos: " << pos
		       << " size: " << para.size()
		       << " par: " << par << endl;
		BOOST_ASSERT(false);
	}
}


void LyXText::setCursorIntern(LCursor & cur,
	pit_type par, pos_type pos, bool setfont, bool boundary)
{
	setCursor(cur.top(), par, pos, boundary);
	cur.setTargetX();
	if (setfont)
		setCurrentFont(cur);
}


void LyXText::setCurrentFont(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	pos_type pos = cur.pos();
	Paragraph & par = cur.paragraph();

	if (cur.boundary() && pos > 0)
		--pos;

	if (pos > 0) {
		if (pos == cur.lastpos())
			--pos;
		else // potentional bug... BUG (Lgb)
			if (par.isSeparator(pos)) {
				if (pos > cur.textRow().pos() &&
				    bidi.level(pos) % 2 ==
				    bidi.level(pos - 1) % 2)
					--pos;
				else if (pos + 1 < cur.lastpos())
					++pos;
			}
	}

	BufferParams const & bufparams = cur.buffer().params();
	current_font = par.getFontSettings(bufparams, pos);
	real_current_font = getFont(par, pos);

	if (cur.pos() == cur.lastpos()
	    && bidi.isBoundary(cur.buffer(), par, cur.pos())
	    && !cur.boundary()) {
		Language const * lang = par.getParLanguage(bufparams);
		current_font.setLanguage(lang);
		current_font.setNumber(LyXFont::OFF);
		real_current_font.setLanguage(lang);
		real_current_font.setNumber(LyXFont::OFF);
	}
}


// x is an absolute screen coord
// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type LyXText::getColumnNearX(pit_type const pit,
	Row const & row, int & x, bool & boundary) const
{
	int const xo = theCoords.get(this, pit).x_;
	x -= xo;
	RowMetrics const r = computeRowMetrics(pit, row);
	Paragraph const & par = pars_[pit];

	pos_type vc = row.pos();
	pos_type end = row.endpos();
	pos_type c = 0;
	LyXLayout_ptr const & layout = par.layout();

	bool left_side = false;

	pos_type body_pos = par.beginOfBody();

	double tmpx = r.x;
	double last_tmpx = tmpx;

	if (body_pos > 0 &&
	    (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// check for empty row
	if (vc == end) {
		x = int(tmpx) + xo;
		return 0;
	}

	while (vc < end && tmpx <= x) {
		c = bidi.vis2log(vc);
		last_tmpx = tmpx;
		if (body_pos > 0 && c == body_pos - 1) {
			tmpx += r.label_hfill +
				font_metrics::width(layout->labelsep, getLabelFont(par));
			if (par.isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(par, body_pos - 1);
		}

		if (hfillExpansion(par, row, c)) {
			tmpx += singleWidth(par, c);
			if (c >= body_pos)
				tmpx += r.hfill;
			else
				tmpx += r.label_hfill;
		} else if (par.isSeparator(c)) {
			tmpx += singleWidth(par, c);
			if (c >= body_pos)
				tmpx += r.separator;
		} else {
			tmpx += singleWidth(par, c);
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
	bool const lastrow = lyxrc.rtl_support && row.endpos() == par.size();

	// If lastrow is false, we don't need to compute
	// the value of rtl.
	bool const rtl = lastrow ? isRTL(par) : false;
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
			boundary = bidi.isBoundary(*bv()->buffer(), par, c);
		}
	}

	if (row.pos() < end && c >= end && par.isNewline(end - 1)) {
		if (bidi.level(end -1) % 2 == 0)
			tmpx -= singleWidth(par, end - 1);
		else
			tmpx += singleWidth(par, end - 1);
		c = end - 1;
	}

	x = int(tmpx) + xo;
	return c - row.pos();
}


// y is screen coordinate
pit_type LyXText::getPitNearY(int y) const
{
	BOOST_ASSERT(!paragraphs().empty());
	BOOST_ASSERT(theCoords.pars_.find(this) != theCoords.pars_.end());
	CoordCache::InnerParPosCache const & cc = theCoords.pars_[this];
	lyxerr << "LyXText::getPitNearY: y: " << y << " cache size: "
		<< cc.size() << endl;

	// look for highest numbered paragraph with y coordinate less than given y
	pit_type pit = 0;
	int yy = -1;
	CoordCache::InnerParPosCache::const_iterator it = cc.begin();
	CoordCache::InnerParPosCache::const_iterator et = cc.end();
	for (; it != et; ++it) {
		lyxerr << "  examining: pit: " << it->first << " y: "
			<< it->second.y_ << endl;
		if (it->first >= pit && int(it->second.y_) - int(pars_[it->first].ascent()) <= y) {
			pit = it->first;
			yy = it->second.y_;
		}
	}

	lyxerr << " found best y: " << yy << " for pit: " << pit << endl;
	return pit;
}


Row const & LyXText::getRowNearY(int y, pit_type pit) const
{
	Paragraph const & par = pars_[pit];
	int yy = theCoords.get(this, pit).y_ - par.ascent();
	BOOST_ASSERT(!par.rows().empty());
	RowList::const_iterator rit = par.rows().begin();
	RowList::const_iterator const rlast = boost::prior(par.rows().end());
	for (; rit != rlast; yy += rit->height(), ++rit)
		if (yy + rit->height() > y)
			break;
	return *rit;
}

// x,y are absolute screen coordinates
// sets cursor recursively descending into nested editable insets
InsetBase * LyXText::editXY(LCursor & cur, int x, int y) const
{
	pit_type pit = getPitNearY(y);
	BOOST_ASSERT(pit != -1);
	Row const & row = getRowNearY(y, pit);
	bool bound = false;

	int xx = x; // is modified by getColumnNearX
	pos_type const pos = row.pos() + getColumnNearX(pit, row, xx, bound);
	cur.pit() = pit;
	cur.pos() = pos;
	cur.boundary() = bound;
	cur.x_target() = x;

	// try to descend into nested insets
	InsetBase * inset = checkInsetHit(x, y);
	lyxerr << "inset " << inset << " hit at x: " << x << " y: " << y << endl;
	if (!inset) {
		//either we deconst editXY or better we move current_font
		//and real_current_font to LCursor
		const_cast<LyXText *>(this)->setCurrentFont(cur);
		return 0;
	}

	// This should be just before or just behind the
	// cursor position set above.
	BOOST_ASSERT((pos != 0 && inset == pars_[pit].getInset(pos - 1))
	             || inset == pars_[pit].getInset(pos));
	// Make sure the cursor points to the position before
	// this inset.
	if (inset == pars_[pit].getInset(pos - 1))
		--cur.pos();
	inset = inset->editXY(cur, x, y);
	if (cur.top().text() == this)
		const_cast<LyXText *>(this)->setCurrentFont(cur);
	return inset;
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
		setCursor(cur, cur.pit(), cur.pos() - 1, true, false);
		if (!checkAndActivateInset(cur, false)) {
			if (false && !boundary &&
					bidi.isBoundary(cur.buffer(), cur.paragraph(), cur.pos() + 1))
				setCursor(cur, cur.pit(), cur.pos() + 1, true, true);
		}
		return;
	}

	if (cur.pit() != 0) {
		// steps into the paragraph above
		setCursor(cur, cur.pit() - 1, getPar(cur.pit() - 1).size());
	}
}


void LyXText::cursorRight(LCursor & cur)
{
	if (false && cur.boundary()) {
		setCursor(cur, cur.pit(), cur.pos(), true, false);
		return;
	}

	if (cur.pos() != cur.lastpos()) {
		if (!checkAndActivateInset(cur, true)) {
			setCursor(cur, cur.pit(), cur.pos() + 1, true, false);
			if (false && bidi.isBoundary(cur.buffer(), cur.paragraph(),
							 cur.pos()))
				setCursor(cur, cur.pit(), cur.pos(), true, true);
		}
		return;
	}

	if (cur.pit() != cur.lastpit())
		setCursor(cur, cur.pit() + 1, 0);
}


void LyXText::cursorUp(LCursor & cur)
{
	Paragraph const & par = cur.paragraph();
	int const row = par.pos2row(cur.pos());
	int const x = cur.targetX();

	if (!cur.selection()) {
		int const y = bv_funcs::getPos(cur).y_;
		editXY(cur, x, y - par.rows()[row].ascent() - 1);
		return;
	}

	if (row > 0) {
		setCursor(cur, cur.pit(), x2pos(cur.pit(), row - 1, x));
	} else if (cur.pit() > 0) {
		--cur.pit();
		setCursor(cur, cur.pit(), x2pos(cur.pit(), cur.paragraph().rows().size() - 1, x));

	}

	cur.x_target() = x;
}


void LyXText::cursorDown(LCursor & cur)
{


	Paragraph const & par = cur.paragraph();
	int const row = par.pos2row(cur.pos());
	int const x = cur.targetX();

	if (!cur.selection()) {
		int const y = bv_funcs::getPos(cur).y_;
		editXY(cur, x, y + par.rows()[row].descent() + 1);
		return;
	}

	if (row + 1 < int(par.rows().size())) {
		setCursor(cur, cur.pit(), x2pos(cur.pit(), row + 1, x));
	} else if (cur.pit() + 1 < int(paragraphs().size())) {
		++cur.pit();
		setCursor(cur, cur.pit(), x2pos(cur.pit(), 0, x));
	}

	cur.x_target() = x;
}


void LyXText::cursorUpParagraph(LCursor & cur)
{
	if (cur.pos() > 0)
		setCursor(cur, cur.pit(), 0);
	else if (cur.pit() != 0)
		setCursor(cur, cur.pit() - 1, 0);
}


void LyXText::cursorDownParagraph(LCursor & cur)
{
	if (cur.pit() != cur.lastpit())
		setCursor(cur, cur.pit() + 1, 0);
	else
		setCursor(cur, cur.pit(), cur.lastpos());
}


// fix the cursor `cur' after a characters has been deleted at `where'
// position. Called by deleteEmptyParagraphMechanism
void LyXText::fixCursorAfterDelete(CursorSlice & cur, CursorSlice const & where)
{
	// do notheing if cursor is not in the paragraph where the
	// deletion occured,
	if (cur.pit() != where.pit())
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
	Paragraph const & oldpar = pars_[old.pit()];

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
	if (old.pit() != cur.pit() || old.pos() != cur.pos()) {

		// Only if the cursor has really moved.
		if (old.pos() > 0
		    && old.pos() < oldpar.size()
		    && oldpar.isLineSeparator(old.pos())
		    && oldpar.isLineSeparator(old.pos() - 1)) {
			pars_[old.pit()].erase(old.pos() - 1);
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
	if (old.pit() == cur.pit())
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
		deleted = true;

		bool selection_position_was_oldcursor_position =
			cur.anchor().pit() == old.pit() && cur.anchor().pos() == old.pos();

		// This is a bit of a overkill. We change the old and the cur par
		// at max, certainly not everything in between...
		recUndo(old.pit(), cur.pit());

		// Delete old par.
		pars_.erase(pars_.begin() + old.pit());

		// Update cursor par offset if necessary.
		// Some 'iterator registration' would be nice that takes care of
		// such events. Maybe even signal/slot?
		if (cur.pit() > old.pit())
			--cur.pit();
#ifdef WITH_WARNINGS
#warning DEPM, look here
#endif
//		if (cur.anchor().pit() > old.pit())
//			--cur.anchor().pit();

		if (selection_position_was_oldcursor_position) {
			// correct selection
			cur.resetAnchor();
		}
	}

	if (deleted)
		return true;

	if (pars_[old.pit()].stripLeadingSpaces())
		cur.resetAnchor();

	return false;
}


ParagraphList & LyXText::paragraphs() const
{
	return const_cast<ParagraphList &>(pars_);
}


void LyXText::recUndo(pit_type first, pit_type last) const
{
	recordUndo(bv()->cursor(), Undo::ATOMIC, first, last);
}


void LyXText::recUndo(pit_type par) const
{
	recordUndo(bv()->cursor(), Undo::ATOMIC, par, par);
}


int defaultRowHeight()
{
	return int(font_metrics::maxHeight(LyXFont(LyXFont::ALL_SANE)) *  1.2);
}
