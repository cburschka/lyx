/*
 * \file CutAndPaste.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "CutAndPaste.h"

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "errorlist.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lfuns.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "lyxtextclasslist.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParagraphList_fwd.h"
#include "pariterator.h"
#include "undo.h"

#include "insets/insettabular.h"

#include "mathed/math_data.h"
#include "mathed/math_inset.h"
#include "mathed/math_support.h"

#include "support/lstrings.h"

#include <boost/tuple/tuple.hpp>

using lyx::pos_type;
using lyx::pit_type;
using lyx::textclass_type;

using lyx::support::bformat;

using std::endl;
using std::for_each;
using std::make_pair;
using std::pair;
using std::vector;
using std::string;


namespace {

typedef std::pair<lyx::pit_type, int> PitPosPair;

typedef limited_stack<pair<ParagraphList, textclass_type> > CutStack;

CutStack theCuts(10);

struct resetOwnerAndChanges : public std::unary_function<Paragraph, void> {
	void operator()(Paragraph & p) const {
		p.cleanChanges();
		p.setInsetOwner(0);
	}
};


void region(CursorSlice const & i1, CursorSlice const & i2,
	InsetBase::row_type & r1, InsetBase::row_type & r2,
	InsetBase::col_type & c1, InsetBase::col_type & c2)
{
	InsetBase & p = i1.inset();
	c1 = p.col(i1.idx());
	c2 = p.col(i2.idx());
	if (c1 > c2)
		std::swap(c1, c2);
	r1 = p.row(i1.idx());
	r2 = p.row(i2.idx());
	if (r1 > r2)
		std::swap(r1, r2);
}


bool checkPastePossible(int index)
{
	return size_t(index) < theCuts.size() && !theCuts[index].first.empty();
}


pair<PitPosPair, pit_type>
pasteSelectionHelper(Buffer const & buffer, ParagraphList & pars,
	pit_type pit, int pos,
	textclass_type tc, size_t cut_index, ErrorList & errorlist)
{
	if (!checkPastePossible(cut_index))
		return make_pair(PitPosPair(pit, pos), pit);

	BOOST_ASSERT (pos <= pars[pit].size());

	// Make a copy of the CaP paragraphs.
	ParagraphList insertion = theCuts[cut_index].first;
	textclass_type const textclass = theCuts[cut_index].second;

	// Now remove all out of the pars which is NOT allowed in the
	// new environment and set also another font if that is required.

	// Make sure there is no class difference.
	lyx::cap::SwitchLayoutsBetweenClasses(textclass, tc, insertion,
				    errorlist);

	ParagraphList::iterator tmpbuf = insertion.begin();
	int depth_delta = pars[pit].params().depth() - tmpbuf->params().depth();

	Paragraph::depth_type max_depth = pars[pit].getMaxDepthAfter();

	for (; tmpbuf != insertion.end(); ++tmpbuf) {
		// If we have a negative jump so that the depth would
		// go below 0 depth then we have to redo the delta to
		// this new max depth level so that subsequent
		// paragraphs are aligned correctly to this paragraph
		// at level 0.
		if (int(tmpbuf->params().depth()) + depth_delta < 0)
			depth_delta = 0;

		// Set the right depth so that we are not too deep or shallow.
		tmpbuf->params().depth(tmpbuf->params().depth() + depth_delta);
		if (tmpbuf->params().depth() > max_depth)
			tmpbuf->params().depth(max_depth);

		// Only set this from the 2nd on as the 2nd depends
		// for maxDepth still on pit.
		if (tmpbuf != insertion.begin())
			max_depth = tmpbuf->getMaxDepthAfter();

		// Set the inset owner of this paragraph.
		tmpbuf->setInsetOwner(pars[pit].inInset());
		for (pos_type i = 0; i < tmpbuf->size(); ++i) {
			if (tmpbuf->getChar(i) == Paragraph::META_INSET) {
				if (!pars[pit].insetAllowed(tmpbuf->getInset(i)->lyxCode()))
					tmpbuf->erase(i--);
			}
		}
	}

	// Make the buf exactly the same layout as the cursor paragraph.
	insertion.begin()->makeSameLayout(pars[pit]);

	// Prepare the paragraphs and insets for insertion.
	// A couple of insets store buffer references so need updating.
	InsetText in;
	std::swap(in.paragraphs(), insertion);

	ParIterator fpit = par_iterator_begin(in);
	ParIterator fend = par_iterator_end(in);

	for (; fpit != fend; ++fpit) {
		InsetList::iterator lit = fpit->insetlist.begin();
		InsetList::iterator eit = fpit->insetlist.end();

		for (; lit != eit; ++lit) {
			switch (lit->inset->lyxCode()) {
			case InsetBase::TABULAR_CODE: {
				InsetTabular * it = static_cast<InsetTabular*>(lit->inset);
				it->buffer(&buffer);
				break;
			}

			default:
				break; // nothing
			}
		}
	}
	std::swap(in.paragraphs(), insertion);

	// Split the paragraph for inserting the buf if necessary.
	bool did_split = false;
	if (pars[pit].size() || pit + 1 == pit_type(pars.size())) {
		breakParagraphConservative(buffer.params(), pars, pit, pos);
		did_split = true;
	}

	// Paste it!
	pars.insert(pars.begin() + pit + 1, insertion.begin(), insertion.end());
	mergeParagraph(buffer.params(), pars, pit);

	pit_type last_paste = pit + insertion.size() - 1;

	// Store the new cursor position.
	pit = last_paste;
	pos = pars[last_paste].size();

	// Maybe some pasting.
	if (did_split && last_paste + 1 != pit_type(pars.size())) {
		if (pars[last_paste + 1].hasSameLayout(pars[last_paste])) {
			mergeParagraph(buffer.params(), pars, last_paste);
		} else if (pars[last_paste + 1].empty()) {
			pars[last_paste + 1].makeSameLayout(pars[last_paste]);
			mergeParagraph(buffer.params(), pars, last_paste);
		} else if (pars[last_paste].empty()) {
			pars[last_paste].makeSameLayout(pars[last_paste + 1]);
			mergeParagraph(buffer.params(), pars, last_paste);
		} else {
			pars[last_paste + 1].stripLeadingSpaces();
			++last_paste;
		}
	}

	return make_pair(PitPosPair(pit, pos), last_paste + 1);
}


PitPosPair eraseSelectionHelper(BufferParams const & params,
	ParagraphList & pars,
	pit_type startpit, pit_type endpit,
	int startpos, int endpos, bool doclear)
{
	if (startpit == pit_type(pars.size()) ||
	    (startpos > pars[startpit].size()))
		return PitPosPair(endpit, endpos);

	if (endpit == pit_type(pars.size()) ||
	    startpit == endpit) {
		endpos -= pars[startpit].erase(startpos, endpos);
		return PitPosPair(endpit, endpos);
	}

	// clear end/begin fragments of the first/last par in selection
	bool all_erased = true;

	pars[startpit].erase(startpos, pars[startpit].size());
	if (pars[startpit].size() != startpos)
		all_erased = false;

	endpos -= pars[endpit].erase(0, endpos);
	if (endpos != 0)
		all_erased = false;

	// Loop through the deleted pars if any, erasing as needed
	for (pit_type pit = startpit + 1; pit != endpit;) {
		// "erase" the contents of the par
		pars[pit].erase(0, pars[pit].size());
		if (!pars[pit].size()) {
			// remove the par if it's now empty
			pars.erase(pars.begin() + pit);
			--endpit;
		} else {
			++pit;
			all_erased = false;
		}
	}

#if 0 // FIXME: why for cut but not copy ?
	// the cut selection should begin with standard layout
	if (realcut) {
		buf->params().clear();
		buf->bibkey = 0;
		buf->layout(textclasslist[buffer->params.textclass].defaultLayoutName());
	}
#endif

	if (startpit + 1 == pit_type(pars.size()))
		return PitPosPair(endpit, endpos);

	if (doclear) {
		pars[startpit + 1].stripLeadingSpaces();
	}

	// paste the paragraphs again, if possible
	if (all_erased &&
	    (pars[startpit].hasSameLayout(pars[startpit + 1]) ||
	     pars[startpit + 1].empty())) {
		mergeParagraph(params, pars, startpit);
		// this because endpar gets deleted here!
		endpit = startpit;
		endpos = startpos;
	}

	return PitPosPair(endpit, endpos);

}


void copySelectionHelper(ParagraphList & pars,
	pit_type startpit, pit_type endpit,
	int start, int end, textclass_type tc)
{
	BOOST_ASSERT(0 <= start && start <= pars[startpit].size());
	BOOST_ASSERT(0 <= end && end <= pars[endpit].size());
	BOOST_ASSERT(startpit != endpit || start <= end);

	// Clone the paragraphs within the selection.
	ParagraphList paragraphs(pars.begin() + startpit, pars.begin() + endpit + 1);
	for_each(paragraphs.begin(), paragraphs.end(), resetOwnerAndChanges());

	// Cut out the end of the last paragraph.
	Paragraph & back = paragraphs.back();
	back.erase(end, back.size());

	// Cut out the begin of the first paragraph
	Paragraph & front = paragraphs.front();
	front.erase(0, start);

	theCuts.push(make_pair(paragraphs, tc));
}



PitPosPair cutSelectionHelper(BufferParams const & params,
	ParagraphList & pars, pit_type startpit, pit_type endpit,
	int startpos, int endpos, textclass_type tc, bool doclear)
{
	copySelectionHelper(pars, startpit, endpit, startpos, endpos, tc);
	return eraseSelectionHelper(params, pars, startpit, endpit,
		startpos, endpos, doclear);
}


} // namespace anon




namespace lyx {
namespace cap {

string grabAndEraseSelection(LCursor & cur)
{
	if (!cur.selection())
		return string();
	string res = grabSelection(cur);
	eraseSelection(cur);
	cur.selection() = false;
	return res;
}


int SwitchLayoutsBetweenClasses(textclass_type c1, textclass_type c2,
	ParagraphList & pars, ErrorList & errorlist)
{
	BOOST_ASSERT(!pars.empty());
	int ret = 0;
	if (c1 == c2)
		return ret;

	LyXTextClass const & tclass1 = textclasslist[c1];
	LyXTextClass const & tclass2 = textclasslist[c2];

	InsetText in;
	std::swap(in.paragraphs(), pars);

	ParIterator end = par_iterator_end(in);
	for (ParIterator it = par_iterator_begin(in); it != end; ++it) {
		string const name = it->layout()->name();
		bool hasLayout = tclass2.hasLayout(name);

		if (hasLayout)
			it->layout(tclass2[name]);
		else
			it->layout(tclass2.defaultLayout());

		if (!hasLayout && name != tclass1.defaultLayoutName()) {
			++ret;
			string const s = bformat(
				_("Layout had to be changed from\n%1$s to %2$s\n"
				"because of class conversion from\n%3$s to %4$s"),
			 name, it->layout()->name(), tclass1.name(), tclass2.name());
			// To warn the user that something had to be done.
			errorlist.push_back(ErrorItem("Changed Layout", s,
						      it->id(), 0,
						      it->size()));
		}
	}
	std::swap(in.paragraphs(), pars);
	return ret;
}


std::vector<string> const availableSelections(Buffer const & buffer)
{
	vector<string> selList;

	CutStack::const_iterator cit = theCuts.begin();
	CutStack::const_iterator end = theCuts.end();
	for (; cit != end; ++cit) {
		// we do not use cit-> here because gcc 2.9x does not
		// like it (JMarc)
		ParagraphList const & pars = (*cit).first;
		string asciiSel;
		ParagraphList::const_iterator pit = pars.begin();
		ParagraphList::const_iterator pend = pars.end();
		for (; pit != pend; ++pit) {
			asciiSel += pit->asString(buffer, false);
			if (asciiSel.size() > 25) {
				asciiSel.replace(22, string::npos, "...");
				break;
			}
		}

		selList.push_back(asciiSel);
	}

	return selList;
}


int nrOfParagraphs()
{
	return theCuts.empty() ? 0 : theCuts[0].first.size();
}


void cutSelection(LCursor & cur, bool doclear, bool realcut)
{
	if (cur.inTexted()) {
		LyXText * text = cur.text();
		BOOST_ASSERT(text);
		// Stuff what we got on the clipboard. Even if there is no selection.

		// There is a problem with having the stuffing here in that the
		// larger the selection the slower LyX will get. This can be
		// solved by running the line below only when the selection has
		// finished. The solution used currently just works, to make it
		// faster we need to be more clever and probably also have more
		// calls to stuffClipboard. (Lgb)
		cur.bv().stuffClipboard(cur.selectionAsString(true));

		// This doesn't make sense, if there is no selection
		if (!cur.selection())
			return;

		// OK, we have a selection. This is always between cur.selBegin()
		// and cur.selEnd()

		// make sure that the depth behind the selection are restored, too
		recordUndoSelection(cur);
		pit_type begpit = cur.selBegin().pit();
		pit_type endpit = cur.selEnd().pit();

		int endpos = cur.selEnd().pos();

		BufferParams const & bp = cur.buffer().params();
		if (realcut) {
			copySelectionHelper(text->paragraphs(),
				begpit, endpit,
				cur.selBegin().pos(), endpos,
				bp.textclass);
		}

		boost::tie(endpit, endpos) =
			eraseSelectionHelper(bp,
				text->paragraphs(),
				begpit, endpit,
				cur.selBegin().pos(), endpos,
				doclear);

		// sometimes necessary
		if (doclear)
			text->paragraphs()[begpit].stripLeadingSpaces();

		// cutSelection can invalidate the cursor so we need to set
		// it anew. (Lgb)
		// we prefer the end for when tracking changes
		cur.pos() = endpos;
		cur.pit() = endpit;

		// need a valid cursor. (Lgb)
		cur.clearSelection();
		text->updateCounters();
	}

	if (cur.inMathed()) {
		lyxerr << "cutSelection in mathed" << endl;
		LCursor tmp = cur;
		copySelection(cur);
		cur.selection() = false;
		eraseSelection(tmp);
	}
}


void copySelection(LCursor & cur)
{
	// stuff the selection onto the X clipboard, from an explicit copy request
	cur.bv().stuffClipboard(cur.selectionAsString(true));

	// this doesn't make sense, if there is no selection
	if (!cur.selection())
		return;

	if (cur.inTexted()) {
		LyXText * text = cur.text();
		BOOST_ASSERT(text);
		// ok we have a selection. This is always between cur.selBegin()
		// and sel_end cursor

		// copy behind a space if there is one
		ParagraphList & pars = text->paragraphs();
		pos_type pos = cur.selBegin().pos();
		pit_type par = cur.selBegin().pit();
		while (pos < pars[par].size()
					 && pars[par].isLineSeparator(pos)
					 && (par != cur.selEnd().pit() || pos < cur.selEnd().pos()))
			++pos;

		copySelectionHelper(pars, par, cur.selEnd().pit(),
			pos, cur.selEnd().pos(), cur.buffer().params().textclass);
	}

	if (cur.inMathed()) {
		lyxerr << "copySelection in mathed" << endl;
		ParagraphList pars;
		pars.push_back(Paragraph());
		BufferParams const & bp = cur.buffer().params();
		pars.back().layout(bp.getLyXTextClass().defaultLayout());
		for_each(pars.begin(), pars.end(), resetOwnerAndChanges());
		pars.back().insert(0, grabSelection(cur), LyXFont());
		theCuts.push(make_pair(pars, bp.textclass));
	}
}


std::string getSelection(Buffer const & buf, size_t sel_index)
{
	return sel_index < theCuts.size()
		? theCuts[sel_index].first.back().asString(buf, false)
		: string();
}


void pasteSelection(LCursor & cur, size_t sel_index)
{
	// this does not make sense, if there is nothing to paste
	lyxerr << "#### pasteSelection " << sel_index << endl;
	if (!checkPastePossible(sel_index))
		return;

	if (cur.inTexted()) {
		LyXText * text = cur.text();
		BOOST_ASSERT(text);

		recordUndo(cur);

		pit_type endpit;
		PitPosPair ppp;

		ErrorList el;

		boost::tie(ppp, endpit) =
			pasteSelectionHelper(cur.buffer(),
								text->paragraphs(),
								cur.pit(), cur.pos(),
								cur.buffer().params().textclass,
								sel_index, el);
		bufferErrors(cur.buffer(), el);
		cur.bv().showErrorList(_("Paste"));

		cur.clearSelection();
		cur.resetAnchor();
		text->setCursor(cur, ppp.first, ppp.second);
		cur.setSelection();
		text->updateCounters();
	}

	if (cur.inMathed()) {
		lyxerr << "### should be handled in MathNest/GridInset" << endl;
	}
}


void setSelectionRange(LCursor & cur, pos_type length)
{
	LyXText * text = cur.text();
	BOOST_ASSERT(text);
	if (!length)
		return;
	cur.resetAnchor();
	while (length--)
		text->cursorRight(cur);
	cur.setSelection();
}


// simple replacing. The font of the first selected character is used
void replaceSelectionWithString(LCursor & cur, string const & str)
{
	LyXText * text = cur.text();
	BOOST_ASSERT(text);
	recordUndo(cur);

	// Get font setting before we cut
	pos_type pos = cur.selEnd().pos();
	Paragraph & par = text->getPar(cur.selEnd().pit());
	LyXFont const font =
		par.getFontSettings(cur.buffer().params(), cur.selBegin().pos());

	// Insert the new string
	string::const_iterator cit = str.begin();
	string::const_iterator end = str.end();
	for (; cit != end; ++cit, ++pos)
		par.insertChar(pos, (*cit), font);

	// Cut the selection
	cutSelection(cur, true, false);
}


void replaceSelection(LCursor & cur)
{
	if (cur.selection())
		cutSelection(cur, true, false);
}


// only used by the spellchecker
void replaceWord(LCursor & cur, string const & replacestring)
{
	LyXText * text = cur.text();
	BOOST_ASSERT(text);

	replaceSelectionWithString(cur, replacestring);
	setSelectionRange(cur, replacestring.length());

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i)
		text->cursorLeft(cur);
}


void eraseSelection(LCursor & cur)
{
	//lyxerr << "LCursor::eraseSelection begin: " << cur << endl;
	CursorSlice const & i1 = cur.selBegin();
	CursorSlice const & i2 = cur.selEnd();
	if (i1.inset().asMathInset()) {
		if (i1.idx() == i2.idx()) {
			i1.cell().erase(i1.pos(), i2.pos());
		} else {
			MathInset * p = i1.asMathInset();
			InsetBase::row_type r1, r2;
			InsetBase::col_type c1, c2;
			region(i1, i2, r1, r2, c1, c2);
			for (InsetBase::row_type row = r1; row <= r2; ++row)
				for (InsetBase::col_type col = c1; col <= c2; ++col)
					p->cell(p->index(row, col)).clear();
		}
		cur.back() = i1;
		cur.pos() = 0; // We've deleted the whole cell. Only pos 0 is valid.
		cur.resetAnchor();
	} else {
		lyxerr << "can't erase this selection 1" << endl;
	}
	//lyxerr << "LCursor::eraseSelection end: " << cur << endl;
}


void selDel(LCursor & cur)
{
	//lyxerr << "LCursor::selDel" << endl;
	if (cur.selection()) {
		eraseSelection(cur);
		cur.selection() = false;
	}
}


void selClearOrDel(LCursor & cur)
{
	//lyxerr << "LCursor::selClearOrDel" << endl;
	if (lyxrc.auto_region_delete)
		selDel(cur);
	else
		cur.selection() = false;
}


string grabSelection(LCursor & cur)
{
	if (!cur.selection())
		return string();

	CursorSlice i1 = cur.selBegin();
	CursorSlice i2 = cur.selEnd();

	if (i1.idx() == i2.idx()) {
		if (i1.inset().asMathInset()) {
			MathArray::const_iterator it = i1.cell().begin();
			return asString(MathArray(it + i1.pos(), it + i2.pos()));
		} else {
			return "unknown selection 1";
		}
	}

	InsetBase::row_type r1, r2;
	InsetBase::col_type c1, c2;
	region(i1, i2, r1, r2, c1, c2);

	string data;
	if (i1.inset().asMathInset()) {
		for (InsetBase::row_type row = r1; row <= r2; ++row) {
			if (row > r1)
				data += "\\\\";
			for (InsetBase::col_type col = c1; col <= c2; ++col) {
				if (col > c1)
					data += '&';
				data += asString(i1.asMathInset()->
					cell(i1.asMathInset()->index(row, col)));
			}
		}
	} else {
		data = "unknown selection 2";
	}
	return data;
}


} // namespace cap
} // namespace lyx
