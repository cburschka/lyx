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
#include "errorlist.h"
#include "gettext.h"
#include "lyxtext.h"
#include "lyxtextclasslist.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParagraphList_fwd.h"
#include "pariterator.h"
#include "undo.h"

#include "insets/insettabular.h"

#include "support/lstrings.h"

#include <boost/tuple/tuple.hpp>

using lyx::pos_type;
using lyx::par_type;
using lyx::textclass_type;

using lyx::support::bformat;

using std::for_each;
using std::make_pair;
using std::pair;
using std::vector;
using std::string;


namespace {

typedef std::pair<lyx::par_type, int> PitPosPair;

typedef limited_stack<pair<ParagraphList, textclass_type> > CutStack;

CutStack cuts(10);

struct resetOwnerAndChanges : public std::unary_function<Paragraph, void> {
	void operator()(Paragraph & p) const {
		p.cleanChanges();
		p.setInsetOwner(0);
	}
};

} // namespace anon


namespace lyx {
namespace cap {


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
	
	ParIterator end = ParIterator(in, pars.size());
	for (ParIterator it = ParIterator(in, 0); it != end; ++it) {
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

	CutStack::const_iterator cit = cuts.begin();
	CutStack::const_iterator end = cuts.end();
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


PitPosPair eraseSelection(BufferParams const & params, ParagraphList & pars,
	par_type startpit, par_type endpit,
	int startpos, int endpos, bool doclear)
{
	if (startpit == par_type(pars.size()) ||
	    (startpos > pars[startpit].size()))
		return PitPosPair(endpit, endpos);

	if (endpit == par_type(pars.size()) ||
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
	for (par_type pit = startpit + 1; pit != endpit;) {
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

	if (startpit + 1 == par_type(pars.size()))
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


bool copySelection(ParagraphList & pars,
	par_type startpit, par_type endpit,
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

	cuts.push(make_pair(paragraphs, tc));

	return true;
}


PitPosPair cutSelection(BufferParams const & params, ParagraphList & pars,
	par_type startpit, par_type endpit,
	int startpos, int endpos, textclass_type tc, bool doclear)
{
	copySelection(pars, startpit, endpit, startpos, endpos, tc);
	return eraseSelection(params, pars, startpit, endpit, startpos,
			      endpos, doclear);
}


pair<PitPosPair, par_type>
pasteSelection(Buffer const & buffer, ParagraphList & pars,
	par_type pit, int pos,
	textclass_type tc, size_t cut_index, ErrorList & errorlist)
{
	if (!checkPastePossible())
		return make_pair(PitPosPair(pit, pos), pit);

	BOOST_ASSERT (pos <= pars[pit].size());

	// Make a copy of the CaP paragraphs.
	ParagraphList insertion = cuts[cut_index].first;
	textclass_type const textclass = cuts[cut_index].second;

	// Now remove all out of the pars which is NOT allowed in the
	// new environment and set also another font if that is required.

	// Make sure there is no class difference.
	SwitchLayoutsBetweenClasses(textclass, tc, insertion,
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

	ParIterator fpit(in, 0);
	ParIterator fend(in, insertion.size());

	for (; fpit != fend; ++fpit) {
		InsetList::iterator lit = fpit->insetlist.begin();
		InsetList::iterator eit = fpit->insetlist.end();

		for (; lit != eit; ++lit) {
			switch (lit->inset->lyxCode()) {
			case InsetOld::TABULAR_CODE: {
				InsetTabular * it = static_cast<InsetTabular*>(lit->inset);
				it->buffer(const_cast<Buffer*>(&buffer));
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
	if (pars[pit].size() || pit + 1 == par_type(pars.size())) {
		breakParagraphConservative(buffer.params(), pars, pit, pos);
		did_split = true;
	}

	// Paste it!
	pars.insert(pars.begin() + pit + 1, insertion.begin(), insertion.end());
	mergeParagraph(buffer.params(), pars, pit);

	par_type last_paste = pit + insertion.size() - 1;
	
	// Store the new cursor position.
	pit = last_paste;
	pos = pars[last_paste].size();

	// Maybe some pasting.
	if (did_split && last_paste + 1 != par_type(pars.size())) {
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


pair<PitPosPair, par_type>
pasteSelection(Buffer const & buffer, ParagraphList & pars,
	par_type pit, int pos, textclass_type tc, ErrorList & errorlist)
{
	return pasteSelection(buffer, pars, pit, pos, tc, 0, errorlist);
}


int nrOfParagraphs()
{
	return cuts.empty() ? 0 : cuts[0].first.size();
}


bool checkPastePossible()
{
	return !cuts.empty() && !cuts[0].first.empty();
}


void cutSelection(LCursor & cur, bool doclear, bool realcut)
{
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
	par_type begpit = cur.selBegin().par();
	par_type endpit = cur.selEnd().par();

	int endpos = cur.selEnd().pos();

	BufferParams const & bufparams = cur.bv().buffer()->params();
	boost::tie(endpit, endpos) = realcut ?
		cutSelection(bufparams,
					  text->paragraphs(),
					  begpit, endpit,
					  cur.selBegin().pos(), endpos,
					  bufparams.textclass,
					  doclear)
		: eraseSelection(bufparams,
					      text->paragraphs(),
					      begpit, endpit,
					      cur.selBegin().pos(), endpos,
					      doclear);
	// sometimes necessary
	if (doclear)
		text->paragraphs()[begpit].stripLeadingSpaces();

	text->redoParagraphs(begpit, begpit + 1);
	// cutSelection can invalidate the cursor so we need to set
	// it anew. (Lgb)
	// we prefer the end for when tracking changes
	cur.pos() = endpos;
	cur.par() = endpit;

	// need a valid cursor. (Lgb)
	cur.clearSelection();
	text->updateCounters();
}


void copySelection(LCursor & cur)
{
	LyXText * text = cur.text();
	BOOST_ASSERT(text);
	// stuff the selection onto the X clipboard, from an explicit copy request
	cur.bv().stuffClipboard(cur.selectionAsString(true));

	// this doesn't make sense, if there is no selection
	if (!cur.selection())
		return;

	// ok we have a selection. This is always between cur.selBegin()
	// and sel_end cursor

	// copy behind a space if there is one
	ParagraphList & pars = text->paragraphs();
	pos_type pos = cur.selBegin().pos();
	par_type par = cur.selBegin().par();
	while (pos < pars[par].size()
	       && pars[par].isLineSeparator(pos)
	       && (par != cur.selEnd().par() || pos < cur.selEnd().pos()))
		++pos;

	copySelection(pars, par, cur.selEnd().par(),
		pos, cur.selEnd().pos(), cur.bv().buffer()->params().textclass);
}


void pasteSelection(LCursor & cur, size_t sel_index)
{
	LyXText * text = cur.text();
	BOOST_ASSERT(text);
	// this does not make sense, if there is nothing to paste
	if (!checkPastePossible())
		return;

	recordUndo(cur);

	par_type endpit;
	PitPosPair ppp;

	ErrorList el;

	boost::tie(ppp, endpit) =
		pasteSelection(*cur.bv().buffer(),
					    text->paragraphs(),
					    cur.par(), cur.pos(),
					    cur.bv().buffer()->params().textclass,
					    sel_index, el);
	bufferErrors(*cur.bv().buffer(), el);
	text->bv()->showErrorList(_("Paste"));

	text->redoParagraphs(cur.par(), endpit);

	cur.clearSelection();
	cur.resetAnchor();
	text->setCursor(cur, ppp.first, ppp.second);
	cur.setSelection();
	text->updateCounters();
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
	LyXFont const font = text->getPar(cur.selBegin().par()).
		getFontSettings(cur.bv().buffer()->params(), cur.selBegin().pos());

	// Insert the new string
	string::const_iterator cit = str.begin();
	string::const_iterator end = str.end();
	for (; cit != end; ++cit, ++pos)
		text->getPar(cur.selEnd().par()).insertChar(pos, (*cit), font);

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


} // namespace cap
} // namespace lyx
