/**
 * \file insettext.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insettext.h"
#include "insetnewline.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "errorlist.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "intl.h"
#include "LColor.h"
#include "lyxfind.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "output_docbook.h"
#include "output_latex.h"
#include "output_linuxdoc.h"
#include "output_plaintext.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
#include "lyxrow.h"
#include "sgml.h"
#include "texrow.h"
#include "undo.h"

#include "frontends/Alert.h"
#include "frontends/font_metrics.h"
#include "frontends/Painter.h"

#include "support/lyxalgo.h" // lyx::count

#include <boost/bind.hpp>
#include <boost/current_function.hpp>

using lyx::pos_type;

using lyx::graphics::PreviewLoader;

using lyx::support::isStrUnsignedInt;

using boost::bind;
using boost::ref;

using std::endl;
using std::for_each;
using std::max;
using std::string;
using std::auto_ptr;
using std::ostream;
using std::vector;


int InsetText::border_ = 2;


InsetText::InsetText(BufferParams const & bp)
	: drawFrame_(false), frame_color_(LColor::insetframe), text_(0)
{
	paragraphs().push_back(Paragraph());
	paragraphs().back().layout(bp.getLyXTextClass().defaultLayout());
	if (bp.tracking_changes)
		paragraphs().back().trackChanges();
	// Dispose of the infamous L-shaped cursor.
	text_.current_font.setLanguage(bp.language);
	text_.real_current_font.setLanguage(bp.language);
	init();
}


InsetText::InsetText(InsetText const & in)
	: InsetOld(in), text_(in.text_.bv_owner)
{
	text_.autoBreakRows_ = in.text_.autoBreakRows_;
	drawFrame_ = in.drawFrame_;
	frame_color_ = in.frame_color_;
	text_.paragraphs() = in.text_.paragraphs();
	// Hand current buffer language down to "cloned" textinsets
	// e.g. tabular cells
	text_.current_font = in.text_.current_font;
	text_.real_current_font = in.text_.real_current_font;
	init();
}


InsetText::InsetText()
	: text_(0)
{}


void InsetText::init()
{
	for_each(paragraphs().begin(), paragraphs().end(),
		 bind(&Paragraph::setInsetOwner, _1, this));
}


void InsetText::markErased(bool erased)
{
	ParagraphList & pars = paragraphs();
	for_each(pars.begin(), pars.end(),
		 bind(&Paragraph::markErased, _1, erased));
}


void InsetText::clear()
{
	ParagraphList & pars = paragraphs();

	// This is a gross hack...
	LyXLayout_ptr old_layout = pars.begin()->layout();

	pars.clear();
	pars.push_back(Paragraph());
	pars.begin()->setInsetOwner(this);
	pars.begin()->layout(old_layout);
}


auto_ptr<InsetBase> InsetText::doClone() const
{
	return auto_ptr<InsetBase>(new InsetText(*this));
}


void InsetText::write(Buffer const & buf, ostream & os) const
{
	os << "Text\n";
	text_.write(buf, os);
}


void InsetText::read(Buffer const & buf, LyXLex & lex)
{
	clear();

#ifdef WITH_WARNINGS
#warning John, look here. Doesnt make much sense.
#endif
	if (buf.params().tracking_changes)
		paragraphs().begin()->trackChanges();

	// delete the initial paragraph
	Paragraph oldpar = *paragraphs().begin();
	paragraphs().clear();
	ErrorList errorList;
	bool res = text_.read(buf, lex, errorList);
	init();

	if (!res) {
		lex.printError("Missing \\end_inset at this point. "
					   "Read: `$$Token'");
	}

	// sanity check
	// ensure we have at least one paragraph.
	if (paragraphs().empty())
		paragraphs().push_back(oldpar);
}


void InsetText::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetText::metrics: width: " << mi.base.textwidth << endl;
	setViewCache(mi.base.bv);
	mi.base.textwidth -= 2 * border_;
	font_ = mi.base.font;
	// Hand font through to contained lyxtext:
	text_.font_ = mi.base.font;
	text_.metrics(mi, dim);
	dim.asc += border_;
	dim.des += border_;
	dim.wid += 2 * border_;
	mi.base.textwidth += 2 * border_;
	dim_ = dim;
}


void InsetText::draw(PainterInfo & pi, int x, int y) const
{
	BOOST_ASSERT(!text_.paragraphs().front().rows().empty());
	// update our idea of where we are
	setPosCache(pi, x, y);

	text_.background_color_ = backgroundColor();
	text_.draw(pi, x + border_, y);

	if (drawFrame_) {
		int const w = text_.width() + 2 * border_;
		int const a = text_.ascent() + border_;
		int const h = a + text_.descent() + border_;
		pi.pain.rectangle(x, y - a, (Wide() ? text_.maxwidth_ : w), h, 
			frameColor());
	}
}


void InsetText::drawSelection(PainterInfo & pi, int x, int y) const
{
	int const w = text_.width() + 2 * border_;
	int const a = text_.ascent() + border_;
	int const h = a + text_.descent() + border_;
	pi.pain.fillRectangle(x, y - a, (Wide() ? text_.maxwidth_ : w), h, 
		backgroundColor());
	text_.drawSelection(pi, x, y);
}


string const InsetText::editMessage() const
{
	return _("Opened Text Inset");
}


void InsetText::edit(LCursor & cur, bool left)
{
	//lyxerr << "InsetText: edit left/right" << endl;
	setViewCache(&cur.bv());
	int const pit = left ? 0 : paragraphs().size() - 1;
	int const pos = left ? 0 : paragraphs().back().size();
	text_.setCursor(cur.top(), pit, pos);
	cur.clearSelection();
	finishUndo();
}


InsetBase * InsetText::editXY(LCursor & cur, int x, int y)
{
	return text_.editXY(cur, x, y);
}


void InsetText::forceParagraphsToDefault(LCursor & cur)
{
	BufferParams const & bp = cur.buffer().params();
	LyXLayout_ptr const layout =
		bp.getLyXTextClass().defaultLayout();
	ParagraphList::iterator const end = paragraphs().end();
	for (ParagraphList::iterator par = paragraphs().begin();
			par != end; ++par)
		par->layout(layout);
}


void InsetText::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	lyxerr[Debug::DEBUG] << BOOST_CURRENT_FUNCTION
			     << " [ cmd.action = "
			     << cmd.action << ']' << endl;
	setViewCache(&cur.bv());

	switch (cmd.action) {

	case LFUN_CHAR_DELETE_FORWARD: {
		if (!cur.selection() && cur.depth() > 1
		    && cur.pit() == cur.lastpit()
		    && cur.pos() == cur.lastpos())
			// Merge inset with owner
			cmd = FuncRequest(LFUN_INSET_DISSOLVE);
		text_.dispatch(cur, cmd);
		break;
	}

	case LFUN_CHAR_DELETE_BACKWARD: {
		if (cur.depth() > 1 && cur.pit() == 0 && cur.pos() == 0)
			// Merge inset with owner
			cmd = FuncRequest(LFUN_INSET_DISSOLVE);
		text_.dispatch(cur, cmd);
		break;
	}
	
	default:
		text_.dispatch(cur, cmd);
		break;
	}
}


bool InsetText::getStatus(LCursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {

	case LFUN_CHAR_DELETE_FORWARD:
	case LFUN_CHAR_DELETE_BACKWARD:
		status.enabled(true);
		return true;

	default:
		return text_.getStatus(cur, cmd, status);
	}
}


int InsetText::latex(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	TexRow texrow;
	latexParagraphs(buf, paragraphs(), os, texrow, runparams);
	return texrow.rows();
}


int InsetText::plaintext(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	ParagraphList::const_iterator it = beg;
	bool ref_printed = false;
	for (; it != end; ++it)
		asciiParagraph(buf, *it, os, runparams, ref_printed);

	// FIXME: Give the total numbers of lines
	return 1;
}


int InsetText::linuxdoc(Buffer const & buf, ostream & os,
			OutputParams const & runparams) const
{
	linuxdocParagraphs(buf, paragraphs(), os, runparams);
	return 0;
}


int InsetText::docbook(Buffer const & buf, ostream & os,
		       OutputParams const & runparams) const
{
	docbookParagraphs(paragraphs(), buf, os, runparams);
	return 0;
}


void InsetText::validate(LaTeXFeatures & features) const
{
	for_each(paragraphs().begin(), paragraphs().end(),
		 bind(&Paragraph::validate, _1, ref(features)));
}


void InsetText::cursorPos
	(CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	x = text_.cursorX(sl, boundary) + border_;
	y = text_.cursorY(sl, boundary);
}


bool InsetText::showInsetDialog(BufferView *) const
{
	return false;
}


void InsetText::markNew(bool track_changes)
{
	ParagraphList::iterator pit = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (; pit != end; ++pit) {
		if (track_changes)
			pit->trackChanges();
		else // no-op when not tracking
			pit->cleanChanges();
	}
}


void InsetText::setText(string const & data, LyXFont const & font)
{
	clear();
	Paragraph & first = paragraphs().front();
	for (unsigned int i = 0; i < data.length(); ++i)
		first.insertChar(i, data[i], font);
}


void InsetText::setAutoBreakRows(bool flag)
{
	if (flag == text_.autoBreakRows_)
		return;

	text_.autoBreakRows_ = flag;
	if (flag)
		return;

	// remove previously existing newlines
	ParagraphList::iterator it = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (; it != end; ++it)
		for (int i = 0; i < it->size(); ++i)
			if (it->isNewline(i))
				it->erase(i);
}


void InsetText::setDrawFrame(bool flag)
{
	drawFrame_ = flag;
}


LColor_color InsetText::frameColor() const
{
	return LColor::color(frame_color_);
}


void InsetText::setFrameColor(LColor_color col)
{
	frame_color_ = col;
}


void InsetText::setViewCache(BufferView const * bv) const
{
	if (bv && bv != text_.bv_owner) {
		//lyxerr << "setting view cache from "
		//	<< text_.bv_owner << " to " << bv << "\n";
		text_.bv_owner = const_cast<BufferView *>(bv);
	}
}


void InsetText::appendParagraphs(Buffer * buffer, ParagraphList & plist)
{
#ifdef WITH_WARNINGS
#warning FIXME Check if Changes stuff needs changing here. (Lgb)
// And it probably does. You have to take a look at this John. (Lgb)
#warning John, have a look here. (Lgb)
#endif
	ParagraphList & pl = paragraphs();

	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = pl.insert(pl.end(), *pit);
	++pit;
	mergeParagraph(buffer->params(), pl,
		       std::distance(pl.begin(), ins) - 1);

	for_each(pit, plist.end(),
		 bind(&ParagraphList::push_back, ref(pl), _1));
}


void InsetText::addPreview(PreviewLoader & loader) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator pend = paragraphs().end();

	for (; pit != pend; ++pit) {
		InsetList::const_iterator it  = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; it != end; ++it)
			it->inset->addPreview(loader);
	}
}


//FIXME: instead of this hack, which only works by chance,
// cells should have their own insetcell type, which returns CELL_CODE!
bool InsetText::neverIndent() const
{
	// this is only true for tabular cells
	return !text_.isMainText() && lyxCode() == TEXT_CODE;
}


ParagraphList const & InsetText::paragraphs() const
{
	return text_.paragraphs();
}


ParagraphList & InsetText::paragraphs()
{
	return text_.paragraphs();
}
