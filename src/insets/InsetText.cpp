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
#include "InsetCaption.h"
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
#include "TextClass.h"
#include "Text.h"
#include "TextMetrics.h"
#include "TocBackend.h"

#include "frontends/alert.h"
#include "frontends/Painter.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "support/bind.h"
#include "support/lassert.h"

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
		it->setBuffer(buf);
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


Dimension const InsetText::dimension(BufferView const & bv) const
{
	TextMetrics const & tm = bv.textMetrics(&text_);
	Dimension dim = tm.dimension();
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

	if (drawFrame_ || pi.full_repaint) {
		int const w = tm.width() + TEXT_TO_INSET_OFFSET;
		int const yframe = y - TEXT_TO_INSET_OFFSET - tm.ascent();
		int const h = tm.height() + 2 * TEXT_TO_INSET_OFFSET;
		int const xframe = x + TEXT_TO_INSET_OFFSET / 2;
		if (pi.full_repaint)
			pi.pain.fillRectangle(xframe, yframe, w, h,
				pi.backgroundColor(this));

		if (drawFrame_)
			pi.pain.rectangle(xframe, yframe, w, h, frameColor());
	}
	ColorCode const old_color = pi.background_color;
	pi.background_color = pi.backgroundColor(this, false);

	tm.draw(pi, x + TEXT_TO_INSET_OFFSET, y);

	pi.background_color = old_color;
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

	text_.setCursor(cur.top(), pit, pos);
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
	if (isPassThru() && lyxCode() != ARG_CODE) {
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
			ParagraphList::const_iterator pit = paragraphs().begin();
			for (; pit != paragraphs().end(); ++pit) {
				InsetList::const_iterator it = pit->insetList().begin();
				InsetList::const_iterator end = pit->insetList().end();
				for (; it != end; ++it) {
					if (it->inset->lyxCode() == ARG_CODE) {
						InsetArgument const * ins =
							static_cast<InsetArgument const *>(it->inset);
						if (ins->name() == arg) {
							// we have this already
							status.setEnabled(false);
							return true;
						}
					}
				}
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
	for_each(paragraphs().begin(), paragraphs().end(),
		 bind(&Paragraph::validate, _1, ref(features)));
}


void InsetText::latex(otexstream & os, OutputParams const & runparams) const
{
	// This implements the standard way of handling the LaTeX
	// output of a text inset, either a command or an
	// environment. Standard collapsable insets should not
	// redefine this, non-standard ones may call this.
	InsetLayout const & il = getLayout();
	if (il.forceOwnlines())
		os << breakln;
	if (!il.latexname().empty()) {
		if (il.latextype() == InsetLayout::COMMAND) {
			// FIXME UNICODE
			// FIXME \protect should only be used for fragile
			//    commands, but we do not provide this information yet.
			if (runparams.moving_arg)
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
	if (!il.passThruChars().empty())
		rp.pass_thru_chars += il.passThruChars();
	rp.par_begin = 0;
	rp.par_end = paragraphs().size();

	// Output the contents of the inset
	latexParagraphs(buffer(), text_, os, rp);
	runparams.encoding = rp.encoding;

	if (!il.rightdelim().empty())
		os << il.rightdelim();

	if (!il.latexname().empty()) {
		if (il.latextype() == InsetLayout::COMMAND) {
			os << "}";
			if (!il.postcommandargs().empty())
				getArgs(os, runparams, true);
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
		xhtmlParagraphs(text_, buffer(), xs, runparams);
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

	xhtmlParagraphs(text_, buffer(), xs, runparams);

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
		latexArgInsets(paragraphs(), paragraphs().begin(), os, runparams, getLayout().postcommandargs(), "post:");
	else
		latexArgInsets(paragraphs(), paragraphs().begin(), os, runparams, getLayout().latexargs());
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


bool InsetText::insetAllowed(InsetCode code) const
{
	switch (code) {
	// Arguments are also allowed in PassThru insets
	case ARG_CODE:
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
		Counters const savecnt = tclass.counters();
		tclass.counters().reset();
		// we need float information even in note insets (#9760)
		tclass.counters().current_float(savecnt.current_float());
		tclass.counters().isSubfloat(savecnt.isSubfloat());
		buffer().updateBuffer(it2, utype);
		tclass.counters() = savecnt;
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
						 UpdateType utype) const
{
	DocIterator dit = cdit;
	dit.push_back(CursorSlice(const_cast<InsetText &>(*this)));
	iterateForToc(dit, output_active, utype);
}


void InsetText::iterateForToc(DocIterator const & cdit, bool output_active,
							  UpdateType utype) const
{
	DocIterator dit = cdit;
	// This also ensures that any document has a table of contents
	shared_ptr<Toc> toc = buffer().tocBackend().toc("tableofcontents");

	BufferParams const & bufparams = buffer_->params();
	int const min_toclevel = bufparams.documentClass().min_toclevel();
	// we really should have done this before we got here, but it
	// can't hurt too much to do it again
	bool const doing_output = output_active && producesOutput();

	// For each paragraph, traverse its insets and let them add
	// their toc items
	ParagraphList const & pars = paragraphs();
	pit_type pend = paragraphs().size();
	for (pit_type pit = 0; pit != pend; ++pit) {
		Paragraph const & par = pars[pit];
		dit.pit() = pit;
		// if we find an optarg, we'll save it for use later.
		InsetText const * arginset = 0;
		InsetList::const_iterator it  = par.insetList().begin();
		InsetList::const_iterator end = par.insetList().end();
		for (; it != end; ++it) {
			Inset & inset = *it->inset;
			dit.pos() = it->pos;
			//lyxerr << (void*)&inset << " code: " << inset.lyxCode() << std::endl;
			inset.addToToc(dit, doing_output, utype);
			if (inset.lyxCode() == ARG_CODE)
				arginset = inset.asInsetText();
		}
		// now the toc entry for the paragraph
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
								  tocstring, doing_output, tocstring));
		}
		
		// And now the list of changes.
		par.addChangesToToc(dit, buffer(), doing_output);
	}
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


docstring InsetText::toolTipText(docstring prefix,
		size_t numlines, size_t len) const
{
	size_t const max_length = numlines * len;
	OutputParams rp(&buffer().params().encoding());
	rp.for_tooltip = true;
	odocstringstream oss;
	oss << prefix;

	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	ParagraphList::const_iterator it = beg;
	bool ref_printed = false;
	docstring str;

	for (; it != end; ++it) {
		if (it != beg)
			oss << '\n';
		writePlaintextParagraph(buffer(), *it, oss, rp, ref_printed, max_length);
		str = oss.str();
		if (str.length() >= max_length)
			break;
	}
	return support::wrapParas(str, 4, len, numlines);
}


InsetCaption const * InsetText::getCaptionInset() const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	for (; pit != paragraphs().end(); ++pit) {
		InsetList::const_iterator it = pit->insetList().begin();
		for (; it != pit->insetList().end(); ++it) {
			Inset & inset = *it->inset;
			if (inset.lyxCode() == CAPTION_CODE) {
				InsetCaption const * ins =
					static_cast<InsetCaption const *>(it->inset);
				return ins;
			}
		}
	}
	return 0;
}


docstring InsetText::getCaptionText(OutputParams const & runparams) const
{
	InsetCaption const * ins = getCaptionInset();
	if (ins == 0)
		return docstring();

	odocstringstream ods;
	ins->getCaptionAsPlaintext(ods, runparams);
	return ods.str();
}


docstring InsetText::getCaptionHTML(OutputParams const & runparams) const
{
	InsetCaption const * ins = getCaptionInset();
	if (ins == 0)
		return docstring();

	odocstringstream ods;
	XHTMLStream xs(ods);
	docstring def = ins->getCaptionAsHTML(xs, runparams);
	if (!def.empty())
		// should already have been escaped
		xs << XHTMLStream::ESCAPE_NONE << def << '\n';
	return ods.str();
}


InsetText::XHTMLOptions operator|(InsetText::XHTMLOptions a1, InsetText::XHTMLOptions a2)
{
	return static_cast<InsetText::XHTMLOptions>((int)a1 | (int)a2);
}

} // namespace lyx
