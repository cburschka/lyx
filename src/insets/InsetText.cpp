/**
 * \file InsetText.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetText.h"

#include "insets/InsetArgument.h"
#include "insets/InsetLayout.h"

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CompletionList.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetList.h"
#include "Intl.h"
#include "Language.h"
#include "Layout.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "lyxfind.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "output_docbook.h"
#include "output_latex.h"
#include "output_xhtml.h"
#include "OutputParams.h"
#include "output_plaintext.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "Row.h"
#include "sgml.h"
#include "TexRow.h"
#include "texstream.h"
#include "TextClass.h"
#include "Text.h"
#include "TextMetrics.h"
#include "TocBackend.h"

#include "frontends/alert.h"
#include "frontends/Painter.h"

#include "support/bind.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/RefChanger.h"

#include <algorithm>


using namespace std;
using namespace lyx::support;


namespace lyx {

using graphics::PreviewLoader;


/////////////////////////////////////////////////////////////////////

InsetText::InsetText(Buffer * buf, UsePlain type)
	: Inset(buf), drawFrame_(false), frame_color_(Color_insetframe),
	text_(this, type == DefaultLayout)
{
}


InsetText::InsetText(InsetText const & in)
	: Inset(in), text_(this, in.text_)
{
	drawFrame_ = in.drawFrame_;
	frame_color_ = in.frame_color_;
}


void InsetText::setBuffer(Buffer & buf)
{
	ParagraphList::iterator end = paragraphs().end();
	for (ParagraphList::iterator it = paragraphs().begin(); it != end; ++it)
		it->setInsetBuffers(buf);
	Inset::setBuffer(buf);
}


void InsetText::setMacrocontextPositionRecursive(DocIterator const & pos)
{
	text_.setMacrocontextPosition(pos);

	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator pend = paragraphs().end();
	for (; pit != pend; ++pit) {
		InsetList::const_iterator iit = pit->insetList().begin();
		InsetList::const_iterator end = pit->insetList().end();
		for (; iit != end; ++iit) {
			if (InsetText * txt = iit->inset->asInsetText()) {
				DocIterator ppos(pos);
				ppos.push_back(CursorSlice(*txt));
				iit->inset->asInsetText()->setMacrocontextPositionRecursive(ppos);
			}
		}
	}
}


void InsetText::clear()
{
	ParagraphList & pars = paragraphs();
	LBUFERR(!pars.empty());

	// This is a gross hack...
	Layout const & old_layout = pars.begin()->layout();

	pars.clear();
	pars.push_back(Paragraph());
	pars.begin()->setInsetOwner(this);
	pars.begin()->setLayout(old_layout);
}


Dimension const InsetText::dimensionHelper(BufferView const & bv) const
{
	TextMetrics const & tm = bv.textMetrics(&text_);
	Dimension dim = tm.dim();
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.asc += TEXT_TO_INSET_OFFSET;
	return dim;
}


void InsetText::write(ostream & os) const
{
	os << "Text\n";
	text_.write(os);
}


void InsetText::read(Lexer & lex)
{
	clear();

	// delete the initial paragraph
	Paragraph oldpar = *paragraphs().begin();
	paragraphs().clear();
	ErrorList errorList;
	lex.setContext("InsetText::read");
	bool res = text_.read(lex, errorList, this);

	if (!res)
		lex.printError("Missing \\end_inset at this point. ");

	// sanity check
	// ensure we have at least one paragraph.
	if (paragraphs().empty())
		paragraphs().push_back(oldpar);
	// Force default font, if so requested
	// This avoids paragraphs in buffer language that would have a
	// foreign language after a document language change, and it ensures
	// that all new text in ERT and similar gets the "latex" language,
	// since new text inherits the language from the last position of the
	// existing text.  As a side effect this makes us also robust against
	// bugs in LyX that might lead to font changes in ERT in .lyx files.
	fixParagraphsFont();
}


void InsetText::metrics(MetricsInfo & mi, Dimension & dim) const
{
	TextMetrics & tm = mi.base.bv->textMetrics(&text_);

	//lyxerr << "InsetText::metrics: width: " << mi.base.textwidth << endl;

	// Hand font through to contained lyxtext:
	tm.font_.fontInfo() = mi.base.font;
	mi.base.textwidth -= 2 * TEXT_TO_INSET_OFFSET;

	// This can happen when a layout has a left and right margin,
	// and the view is made very narrow. We can't do better than
	// to draw it partly out of view (bug 5890).
	if (mi.base.textwidth < 1)
		mi.base.textwidth = 1;

	if (hasFixedWidth())
		tm.metrics(mi, dim, mi.base.textwidth);
	else
		tm.metrics(mi, dim);
	mi.base.textwidth += 2 * TEXT_TO_INSET_OFFSET;
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
}


void InsetText::draw(PainterInfo & pi, int x, int y) const
{
	TextMetrics & tm = pi.base.bv->textMetrics(&text_);

	int const w = tm.width() + TEXT_TO_INSET_OFFSET;
	int const yframe = y - TEXT_TO_INSET_OFFSET - tm.ascent();
	int const h = tm.height() + 2 * TEXT_TO_INSET_OFFSET;
	int const xframe = x + TEXT_TO_INSET_OFFSET / 2;
	bool change_drawn = false;
	if (pi.full_repaint)
			pi.pain.fillRectangle(xframe, yframe, w, h,
				pi.backgroundColor(this));

	{
		Changer dummy = make_change(pi.background_color,
		                            pi.backgroundColor(this, false));
		// The change tracking cue must not be inherited
		Changer dummy2 = make_change(pi.change_, Change());
		tm.draw(pi, x + TEXT_TO_INSET_OFFSET, y);
	}

	if (drawFrame_) {
		// Change color of the frame in tracked changes, like for tabulars.
		// Only do so if the color is not custom. But do so even if RowPainter
		// handles the strike-through already.
		Color c;
		if (pi.change_.changed()
		    // Originally, these are the colors with role Text, from role() in
		    // ColorCache.cpp.  The code is duplicated to avoid depending on Qt
		    // types, and also maybe it need not match in the future.
		    && (frameColor() == Color_foreground
		        || frameColor() == Color_cursor
		        || frameColor() == Color_preview
		        || frameColor() == Color_tabularline
		        || frameColor() == Color_previewframe)) {
			c = pi.change_.color();
			change_drawn = true;
		} else
			c = frameColor();
		pi.pain.rectangle(xframe, yframe, w, h, c);
	}

	if (canPaintChange(*pi.base.bv) && (!change_drawn || pi.change_.deleted()))
		// Do not draw the change tracking cue if already done by RowPainter and
		// do not draw the cue for INSERTED if the information is already in the
		// color of the frame
		pi.change_.paintCue(pi, xframe, yframe, xframe + w, yframe + h);
}


void InsetText::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	pit_type const pit = front ? 0 : paragraphs().size() - 1;
	pos_type pos = front ? 0 : paragraphs().back().size();

	// if visual information is not to be ignored, move to extreme right/left
	if (entry_from != ENTRY_DIRECTION_IGNORE) {
		Cursor temp_cur = cur;
		temp_cur.pit() = pit;
		temp_cur.pos() = pos;
		temp_cur.posVisToRowExtremity(entry_from == ENTRY_DIRECTION_LEFT);
		pos = temp_cur.pos();
	}

	cur.top().setPitPos(pit, pos);
	cur.finishUndo();
}


Inset * InsetText::editXY(Cursor & cur, int x, int y)
{
	return cur.bv().textMetrics(&text_).editXY(cur, x, y);
}


void InsetText::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::ACTION, "InsetText::doDispatch(): cmd: " << cmd);

	// See bug #9042, for instance.
	if (isPassThru()) {
		// Force any new text to latex_language FIXME: This
		// should only be necessary in constructor, but new
		// paragraphs that are created by pressing enter at
		// the start of an existing paragraph get the buffer
		// language and not latex_language, so we take this
		// brute force approach.
		cur.current_font.setLanguage(latex_language);
		cur.real_current_font.setLanguage(latex_language);
	}

	switch (cmd.action()) {
	case LFUN_PASTE:
	case LFUN_CLIPBOARD_PASTE:
	case LFUN_SELECTION_PASTE:
	case LFUN_PRIMARY_SELECTION_PASTE:
		text_.dispatch(cur, cmd);
		// If we we can only store plain text, we must reset all
		// attributes.
		// FIXME: Change only the pasted paragraphs
		fixParagraphsFont();
		break;

	case LFUN_INSET_DISSOLVE: {
		bool const main_inset = text_.isMainText();
		bool const target_inset = cmd.argument().empty()
			|| cmd.getArg(0) == insetName(lyxCode());
		// cur.inset() is the tabular when this is a single cell (bug #9954)
		bool const one_cell = cur.inset().nargs() == 1;

		if (!main_inset && target_inset && one_cell) {
			// Text::dissolveInset assumes that the cursor
			// is inside the Inset.
			if (&cur.inset() != this)
				cur.pushBackward(*this);
			cur.beginUndoGroup();
			text_.dispatch(cur, cmd);
			cur.endUndoGroup();
		} else
			cur.undispatched();
		break;
	}

	default:
		text_.dispatch(cur, cmd);
	}

	if (!cur.result().dispatched())
		Inset::doDispatch(cur, cmd);
}


bool InsetText::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	case LFUN_INSET_DISSOLVE: {
		bool const main_inset = text_.isMainText();
		bool const target_inset = cmd.argument().empty()
			|| cmd.getArg(0) == insetName(lyxCode());
		// cur.inset() is the tabular when this is a single cell (bug #9954)
		bool const one_cell = cur.inset().nargs() == 1;

		if (target_inset)
			status.setEnabled(!main_inset && one_cell);
		return target_inset;
	}

	case LFUN_ARGUMENT_INSERT: {
		string const arg = cmd.getArg(0);
		if (arg.empty()) {
			status.setEnabled(false);
			return true;
		}
		if (text_.isMainText() || !cur.paragraph().layout().args().empty())
			return text_.getStatus(cur, cmd, status);

		Layout::LaTeXArgMap args = getLayout().args();
		Layout::LaTeXArgMap::const_iterator const lait = args.find(arg);
		if (lait != args.end()) {
			status.setEnabled(true);
			for (Paragraph const & par : paragraphs())
				for (auto const & table : par.insetList())
					if (InsetArgument const * ins = table.inset->asInsetArgument())
						if (ins->name() == arg) {
							// we have this already
							status.setEnabled(false);
							return true;
						}
		} else
			status.setEnabled(false);
		return true;
	}

	default:
		// Dispatch only to text_ if the cursor is inside
		// the text_. It is not for context menus (bug 5797).
		bool ret = false;
		if (cur.text() == &text_)
			ret = text_.getStatus(cur, cmd, status);

		if (!ret)
			ret = Inset::getStatus(cur, cmd, status);
		return ret;
	}
}


void InsetText::fixParagraphsFont()
{
	Font font(inherit_font, buffer().params().language);
	font.setLanguage(latex_language);
	ParagraphList::iterator par = paragraphs().begin();
	ParagraphList::iterator const end = paragraphs().end();
	while (par != end) {
		if (par->isPassThru())
			par->resetFonts(font);
		if (!par->allowParagraphCustomization())
			par->params().clear();
		++par;
	}
}


void InsetText::setChange(Change const & change)
{
	ParagraphList::iterator pit = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (; pit != end; ++pit) {
		pit->setChange(change);
	}
}


void InsetText::acceptChanges()
{
	text_.acceptChanges();
}


void InsetText::rejectChanges()
{
	text_.rejectChanges();
}


void InsetText::validate(LaTeXFeatures & features) const
{
	features.useInsetLayout(getLayout());
	for (Paragraph const & p : paragraphs())
		p.validate(features);
}


void InsetText::latex(otexstream & os, OutputParams const & runparams) const
{
	// This implements the standard way of handling the LaTeX
	// output of a text inset, either a command or an
	// environment. Standard collapsible insets should not
	// redefine this, non-standard ones may call this.
	InsetLayout const & il = getLayout();
	if (il.forceOwnlines())
		os << breakln;
	bool needendgroup = false;
	if (!il.latexname().empty()) {
		if (il.latextype() == InsetLayout::COMMAND) {
			// FIXME UNICODE
			// FIXME \protect should only be used for fragile
			//    commands, but we do not provide this information yet.
			if (hasCProtectContent(runparams.moving_arg)) {
				if (contains(runparams.active_chars, '^')) {
					// cprotect relies on ^ being ignored
					os << "\\begingroup\\catcode`\\^=9";
					needendgroup = true;
				}
				os << "\\cprotect";
			} else if (runparams.moving_arg)
				os << "\\protect";
			os << '\\' << from_utf8(il.latexname());
			if (!il.latexargs().empty())
				getArgs(os, runparams);
			if (!il.latexparam().empty())
				os << from_utf8(il.latexparam());
			os << '{';
		} else if (il.latextype() == InsetLayout::ENVIRONMENT) {
			if (il.isDisplay())
				os << breakln;
			else
				os << safebreakln;
			if (runparams.lastid != -1)
				os.texrow().start(runparams.lastid,
						  runparams.lastpos);
			os << "\\begin{" << from_utf8(il.latexname()) << "}";
			if (!il.latexargs().empty())
				getArgs(os, runparams);
			if (!il.latexparam().empty())
				os << from_utf8(il.latexparam());
			os << '\n';
		}
	} else {
		if (!il.latexargs().empty())
			getArgs(os, runparams);
		if (!il.latexparam().empty())
			os << from_utf8(il.latexparam());
	}

	if (!il.leftdelim().empty())
		os << il.leftdelim();

	OutputParams rp = runparams;
	if (isPassThru())
		rp.pass_thru = true;
	if (il.isNeedProtect())
		rp.moving_arg = true;
	if (il.isNeedMBoxProtect())
		++rp.inulemcmd;
	if (!il.passThruChars().empty())
		rp.pass_thru_chars += il.passThruChars();
	if (!il.newlineCmd().empty())
		rp.newlinecmd = il.newlineCmd();
	rp.par_begin = 0;
	rp.par_end = paragraphs().size();

	// Output the contents of the inset
	latexParagraphs(buffer(), text_, os, rp);
	runparams.encoding = rp.encoding;
	// Pass the post_macros upstream
	runparams.post_macro = rp.post_macro;

	if (!il.rightdelim().empty())
		os << il.rightdelim();

	if (!il.latexname().empty()) {
		if (il.latextype() == InsetLayout::COMMAND) {
			os << "}";
			if (!il.postcommandargs().empty())
				getArgs(os, runparams, true);
			if (needendgroup)
				os << "\\endgroup";
		} else if (il.latextype() == InsetLayout::ENVIRONMENT) {
			// A comment environment doesn't need a % before \n\end
			if (il.isDisplay() || runparams.inComment)
				os << breakln;
			else
				os << safebreakln;
			os << "\\end{" << from_utf8(il.latexname()) << "}" << breakln;
			if (!il.isDisplay())
				os.protectSpace(true);
		}
	}
	if (il.forceOwnlines())
		os << breakln;
}


int InsetText::plaintext(odocstringstream & os,
        OutputParams const & runparams, size_t max_length) const
{
	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	ParagraphList::const_iterator it = beg;
	bool ref_printed = false;
	int len = 0;
	for (; it != end; ++it) {
		if (it != beg) {
			os << '\n';
			if (runparams.linelen > 0)
				os << '\n';
		}
		odocstringstream oss;
		writePlaintextParagraph(buffer(), *it, oss, runparams, ref_printed, max_length);
		docstring const str = oss.str();
		os << str;
		// FIXME: len is not computed fully correctly; in principle,
		// we have to count the characters after the last '\n'
		len = str.size();
		if (os.str().size() >= max_length)
			break;
	}

	return len;
}


int InsetText::docbook(odocstream & os, OutputParams const & runparams) const
{
	ParagraphList::const_iterator const beg = paragraphs().begin();

	if (!undefined())
		sgml::openTag(os, getLayout().latexname(),
			      beg->getID(buffer(), runparams) + getLayout().latexparam());

	docbookParagraphs(text_, buffer(), os, runparams);

	if (!undefined())
		sgml::closeTag(os, getLayout().latexname());

	return 0;
}


docstring InsetText::xhtml(XHTMLStream & xs, OutputParams const & runparams) const
{
	return insetAsXHTML(xs, runparams, WriteEverything);
}


// FIXME XHTML
// There are cases where we may need to close open fonts and such
// and then re-open them when we are done. This would be the case, e.g.,
// if we were otherwise about to write:
//		<em>word <div class='foot'>footnote text.</div> emph</em>
// The problem isn't so much that the footnote text will get emphasized:
// we can handle that with CSS. The problem is that this is invalid XHTML.
// One solution would be to make the footnote <span>, but the problem is
// completely general, and so we'd have to make absolutely everything into
// span. What I think will work is to check if we're about to write "div" and,
// if so, try to close fonts, etc.
// There are probably limits to how well we can do here, though, and we will
// have to rely upon users not putting footnotes inside noun-type insets.
docstring InsetText::insetAsXHTML(XHTMLStream & xs, OutputParams const & rp,
                                  XHTMLOptions opts) const
{
	// we will always want to output all our paragraphs when we are
	// called this way.
	OutputParams runparams = rp;
	runparams.par_begin = 0;
	runparams.par_end = text().paragraphs().size();

	if (undefined()) {
		xs.startDivision(false);
		xhtmlParagraphs(text_, buffer(), xs, runparams);
		xs.endDivision();
		return docstring();
	}

	InsetLayout const & il = getLayout();
	if (opts & WriteOuterTag)
		xs << html::StartTag(il.htmltag(), il.htmlattr());

	if ((opts & WriteLabel) && !il.counter().empty()) {
		BufferParams const & bp = buffer().masterBuffer()->params();
		Counters & cntrs = bp.documentClass().counters();
		cntrs.step(il.counter(), OutputUpdate);
		// FIXME: translate to paragraph language
		if (!il.htmllabel().empty()) {
			docstring const lbl =
				cntrs.counterLabel(from_utf8(il.htmllabel()), bp.language->code());
			// FIXME is this check necessary?
			if (!lbl.empty()) {
				xs << html::StartTag(il.htmllabeltag(), il.htmllabelattr());
				xs << lbl;
				xs << html::EndTag(il.htmllabeltag());
			}
		}
	}

	if (opts & WriteInnerTag)
		xs << html::StartTag(il.htmlinnertag(), il.htmlinnerattr());

	// we will eventually lose information about the containing inset
	if (!allowMultiPar() || opts == JustText)
		runparams.html_make_pars = false;
	if (il.isPassThru())
		runparams.pass_thru = true;

	xs.startDivision(false);
	xhtmlParagraphs(text_, buffer(), xs, runparams);
	xs.endDivision();

	if (opts & WriteInnerTag)
		xs << html::EndTag(il.htmlinnertag());

	if (opts & WriteOuterTag)
		xs << html::EndTag(il.htmltag());

	return docstring();
}


void InsetText::getArgs(otexstream & os, OutputParams const & runparams_in,
			bool const post) const
{
	OutputParams runparams = runparams_in;
	runparams.local_font =
		&paragraphs()[0].getFirstFontSettings(buffer().masterBuffer()->params());
	if (isPassThru())
		runparams.pass_thru = true;
	if (post)
		latexArgInsetsForParent(paragraphs(), os, runparams,
		                        getLayout().postcommandargs(), "post:");
	else
		latexArgInsetsForParent(paragraphs(), os, runparams,
		                        getLayout().latexargs());
}


void InsetText::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	x = bv.textMetrics(&text_).cursorX(sl, boundary) + TEXT_TO_INSET_OFFSET;
	y = bv.textMetrics(&text_).cursorY(sl, boundary);
}


void InsetText::setText(docstring const & data, Font const & font, bool trackChanges)
{
	clear();
	Paragraph & first = paragraphs().front();
	for (unsigned int i = 0; i < data.length(); ++i)
		first.insertChar(i, data[i], font, trackChanges);
}


void InsetText::setDrawFrame(bool flag)
{
	drawFrame_ = flag;
}


ColorCode InsetText::frameColor() const
{
	return frame_color_;
}


void InsetText::setFrameColor(ColorCode col)
{
	frame_color_ = col;
}


void InsetText::appendParagraphs(ParagraphList & plist)
{
	// There is little we can do here to keep track of changes.
	// As of 2006/10/20, appendParagraphs is used exclusively by
	// LyXTabular::setMultiColumn. In this context, the paragraph break
	// is lost irreversibly and the appended text doesn't really change

	ParagraphList & pl = paragraphs();

	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = pl.insert(pl.end(), *pit);
	++pit;
	mergeParagraph(buffer().params(), pl,
		       distance(pl.begin(), ins) - 1);

	for_each(pit, plist.end(),
		 bind(&ParagraphList::push_back, ref(pl), _1));
}


void InsetText::addPreview(DocIterator const & text_inset_pos,
	PreviewLoader & loader) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator pend = paragraphs().end();
	int pidx = 0;

	DocIterator inset_pos = text_inset_pos;
	inset_pos.push_back(CursorSlice(*const_cast<InsetText *>(this)));

	for (; pit != pend; ++pit, ++pidx) {
		InsetList::const_iterator it  = pit->insetList().begin();
		InsetList::const_iterator end = pit->insetList().end();
		inset_pos.pit() = pidx;
		for (; it != end; ++it) {
			inset_pos.pos() = it->pos;
			it->inset->addPreview(inset_pos, loader);
		}
	}
}


ParagraphList const & InsetText::paragraphs() const
{
	return text_.paragraphs();
}


ParagraphList & InsetText::paragraphs()
{
	return text_.paragraphs();
}


bool InsetText::hasCProtectContent(bool fragile) const
{
	fragile |= getLayout().isNeedProtect();
	ParagraphList const & pars = paragraphs();
	pit_type pend = pit_type(paragraphs().size());
	for (pit_type pit = 0; pit != pend; ++pit) {
		Paragraph const & par = pars[size_type(pit)];
		if (par.needsCProtection(fragile))
			return true;
	}
	return false;
}


bool InsetText::insetAllowed(InsetCode code) const
{
	switch (code) {
	// Arguments and (plain) quotes are also allowed in PassThru insets
	case ARG_CODE:
	case QUOTE_CODE:
		return true;
	default:
		return !isPassThru();
	}
}


void InsetText::updateBuffer(ParIterator const & it, UpdateType utype)
{
	ParIterator it2 = it;
	it2.forwardPos();
	LASSERT(&it2.inset() == this && it2.pit() == 0, return);
	if (producesOutput()) {
		InsetLayout const & il = getLayout();
		bool const save_layouts = utype == OutputUpdate && il.htmlisblock();
		Counters & cnt = buffer().masterBuffer()->params().documentClass().counters();
		if (save_layouts) {
			// LYXERR0("Entering " << name());
			cnt.clearLastLayout();
			// FIXME cnt.saveLastCounter()?
		}
		buffer().updateBuffer(it2, utype);
		if (save_layouts) {
			// LYXERR0("Exiting " << name());
			cnt.restoreLastLayout();
			// FIXME cnt.restoreLastCounter()?
		}
	} else {
		DocumentClass const & tclass = buffer().masterBuffer()->params().documentClass();
		// Note that we do not need to call:
		//	tclass.counters().clearLastLayout()
		// since we are saving and restoring the existing counters, etc.
		Counters savecnt = tclass.counters();
		tclass.counters().reset();
		// we need float information even in note insets (#9760)
		tclass.counters().current_float(savecnt.current_float());
		tclass.counters().isSubfloat(savecnt.isSubfloat());
		buffer().updateBuffer(it2, utype);
		tclass.counters() = move(savecnt);
	}
}


void InsetText::toString(odocstream & os) const
{
	os << text().asString(0, 1, AS_STR_LABEL | AS_STR_INSETS);
}


void InsetText::forOutliner(docstring & os, size_t const maxlen,
							bool const shorten) const
{
	if (!getLayout().isInToc())
		return;
	text().forOutliner(os, maxlen, shorten);
}


void InsetText::addToToc(DocIterator const & cdit, bool output_active,
						 UpdateType utype, TocBackend & backend) const
{
	DocIterator dit = cdit;
	dit.push_back(CursorSlice(const_cast<InsetText &>(*this)));
	iterateForToc(dit, output_active, utype, backend);
}


void InsetText::iterateForToc(DocIterator const & cdit, bool output_active,
							  UpdateType utype, TocBackend & backend) const
{
	DocIterator dit = cdit;
	// This also ensures that any document has a table of contents
	shared_ptr<Toc> toc = backend.toc("tableofcontents");

	BufferParams const & bufparams = buffer_->params();
	int const min_toclevel = bufparams.documentClass().min_toclevel();
	// we really should have done this before we got here, but it
	// can't hurt too much to do it again
	bool const doing_output = output_active && producesOutput();

	// For each paragraph,
	// * Add a toc item for the paragraph if it is AddToToc--merging adjacent
	//   paragraphs as needed.
	// * Traverse its insets and let them add their toc items
	// * Compute the main table of contents (this is hardcoded)
	// * Add the list of changes
	ParagraphList const & pars = paragraphs();
	pit_type pend = paragraphs().size();
	// Record pairs {start,end} of where a toc item was opened for a paragraph
	// and where it must be closed
	stack<pair<pit_type, pit_type>> addtotoc_stack;

	for (pit_type pit = 0; pit != pend; ++pit) {
		Paragraph const & par = pars[pit];
		dit.pit() = pit;
		dit.pos() = 0;

		// Custom AddToToc in paragraph layouts (i.e. theorems)
		if (par.layout().addToToc() && text().isFirstInSequence(pit)) {
			pit_type end =
				openAddToTocForParagraph(pit, dit, output_active, backend);
			addtotoc_stack.push({pit, end});
		}

		// If we find an InsetArgument that is supposed to provide the TOC caption,
		// we'll save it for use later.
		InsetArgument const * arginset = nullptr;
		for (auto const & table : par.insetList()) {
			dit.pos() = table.pos;
			table.inset->addToToc(dit, doing_output, utype, backend);
			if (InsetArgument const * x = table.inset->asInsetArgument())
				if (x->isTocCaption())
					arginset = x;
		}

		// End custom AddToToc in paragraph layouts
		while (!addtotoc_stack.empty() && addtotoc_stack.top().second == pit) {
			// execute the closing function
			closeAddToTocForParagraph(addtotoc_stack.top().first,
			                          addtotoc_stack.top().second, backend);
			addtotoc_stack.pop();
		}

		// now the toc entry for the paragraph in the main table of contents
		int const toclevel = text().getTocLevel(pit);
		if (toclevel != Layout::NOT_IN_TOC && toclevel >= min_toclevel) {
			// insert this into the table of contents
			docstring tocstring;
			int const length = (doing_output && utype == OutputUpdate) ?
				INT_MAX : TOC_ENTRY_LENGTH;
			if (arginset) {
				tocstring = par.labelString();
				if (!tocstring.empty())
					tocstring += ' ';
				arginset->text().forOutliner(tocstring, length);
			} else
				par.forOutliner(tocstring, length);
			dit.pos() = 0;
			toc->push_back(TocItem(dit, toclevel - min_toclevel,
			                       tocstring, doing_output));
		}

		// And now the list of changes.
		par.addChangesToToc(dit, buffer(), doing_output, backend);
	}
}


pit_type InsetText::openAddToTocForParagraph(pit_type pit,
                                             DocIterator const & dit,
                                             bool output_active,
                                             TocBackend & backend) const
{
	Paragraph const & par = paragraphs()[pit];
	TocBuilder & b = backend.builder(par.layout().tocType());
	docstring const label = par.labelString();
	b.pushItem(dit, label + (label.empty() ? "" : " "), output_active);
	return text().lastInSequence(pit);
}


void InsetText::closeAddToTocForParagraph(pit_type start, pit_type end,
                                          TocBackend & backend) const
{
	Paragraph const & par = paragraphs()[start];
	TocBuilder & b = backend.builder(par.layout().tocType());
	if (par.layout().isTocCaption()) {
		docstring str;
		text().forOutliner(str, TOC_ENTRY_LENGTH, start, end);
		b.argumentItem(str);
	}
	b.pop();
}


bool InsetText::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	if (buffer().isClean())
		return Inset::notifyCursorLeaves(old, cur);

	// find text inset in old cursor
	Cursor insetCur = old;
	int scriptSlice	= insetCur.find(this);
	// we can try to continue here. returning true means
	// the cursor is "now" invalid. which it was.
	LASSERT(scriptSlice != -1, return true);
	insetCur.cutOff(scriptSlice);
	LASSERT(&insetCur.inset() == this, return true);

	// update the old paragraph's words
	insetCur.paragraph().updateWords();

	return Inset::notifyCursorLeaves(old, cur);
}


bool InsetText::completionSupported(Cursor const & cur) const
{
	//LASSERT(&cur.bv().cursor().inset() == this, return false);
	return text_.completionSupported(cur);
}


bool InsetText::inlineCompletionSupported(Cursor const & cur) const
{
	return completionSupported(cur);
}


bool InsetText::automaticInlineCompletion() const
{
	return lyxrc.completion_inline_text;
}


bool InsetText::automaticPopupCompletion() const
{
	return lyxrc.completion_popup_text;
}


bool InsetText::showCompletionCursor() const
{
	return lyxrc.completion_cursor_text;
}


CompletionList const * InsetText::createCompletionList(Cursor const & cur) const
{
	return completionSupported(cur) ? text_.createCompletionList(cur) : 0;
}


docstring InsetText::completionPrefix(Cursor const & cur) const
{
	if (!completionSupported(cur))
		return docstring();
	return text_.completionPrefix(cur);
}


bool InsetText::insertCompletion(Cursor & cur, docstring const & s,
	bool finished)
{
	if (!completionSupported(cur))
		return false;

	return text_.insertCompletion(cur, s, finished);
}


void InsetText::completionPosAndDim(Cursor const & cur, int & x, int & y,
	Dimension & dim) const
{
	TextMetrics const & tm = cur.bv().textMetrics(&text_);
	tm.completionPosAndDim(cur, x, y, dim);
}


string InsetText::contextMenu(BufferView const &, int, int) const
{
	string context_menu = contextMenuName();
	if (context_menu != InsetText::contextMenuName())
		context_menu += ";" + InsetText::contextMenuName();
	return context_menu;
}


string InsetText::contextMenuName() const
{
	return "context-edit";
}


docstring InsetText::toolTipText(docstring prefix, size_t const len) const
{
	OutputParams rp(&buffer().params().encoding());
	rp.for_tooltip = true;
	odocstringstream oss;
	oss << prefix;

	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	ParagraphList::const_iterator it = beg;
	bool ref_printed = false;

	for (; it != end; ++it) {
		if (it != beg)
			oss << '\n';
		if ((*it).isRTL(buffer().params()))
			oss << "<div dir=\"rtl\">";
		writePlaintextParagraph(buffer(), *it, oss, rp, ref_printed, len);
		if ((*it).isRTL(buffer().params()))
			oss << "</div>";
		if (oss.tellp() >= 0 && size_t(oss.tellp()) > len)
			break;
	}
	docstring str = oss.str();
	support::truncateWithEllipsis(str, len);
	return str;
}


InsetText::XHTMLOptions operator|(InsetText::XHTMLOptions a1, InsetText::XHTMLOptions a2)
{
	return static_cast<InsetText::XHTMLOptions>((int)a1 | (int)a2);
}


bool InsetText::needsCProtection(bool const maintext, bool const fragile) const
{
	// Nested cprotect content needs \cprotect
	// on each level
	if (producesOutput() && hasCProtectContent(fragile))
		return true;

	// Environments generally need cprotection in fragile context
	if (fragile && getLayout().latextype() == InsetLayout::ENVIRONMENT)
		return true;

	if (!getLayout().needsCProtect())
		return false;

	// Environments and "no latex" types (e.g., knitr chunks)
	// need cprotection regardless the content
	if (!maintext && getLayout().latextype() != InsetLayout::COMMAND)
		return true;

	// If the inset does not produce output (e.g. Note or Branch),
	// we can ignore the contained paragraphs
	if (!producesOutput())
		return false;

	// Commands need cprotection if they contain specific chars
	int const nchars_escape = 9;
	static char_type const chars_escape[nchars_escape] = {
		'&', '_', '$', '%', '#', '^', '{', '}', '\\'};

	ParagraphList const & pars = paragraphs();
	pit_type pend = pit_type(paragraphs().size());

	for (pit_type pit = 0; pit != pend; ++pit) {
		Paragraph const & par = pars[size_type(pit)];
		if (par.needsCProtection(fragile))
			return true;
		docstring const pars = par.asString();
		for (int k = 0; k < nchars_escape; k++) {
			if (contains(pars, chars_escape[k]))
				return true;
		}
	}
	return false;
}

} // namespace lyx
