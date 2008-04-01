/*
 * \file CutAndPaste.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "CutAndPaste.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "debug.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "InsetIterator.h"
#include "Language.h"
#include "lfuns.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Text.h"
#include "TextClassList.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "Undo.h"

#include "insets/InsetCharStyle.h"
#include "insets/InsetTabular.h"

#include "mathed/MathData.h"
#include "mathed/InsetMath.h"
#include "mathed/MathSupport.h"

#include "support/lstrings.h"

#include "frontends/Clipboard.h"
#include "frontends/Selection.h"

#include <boost/current_function.hpp>
#include <boost/tuple/tuple.hpp>

#include <string>

using std::endl;
using std::for_each;
using std::make_pair;
using std::pair;
using std::vector;
using std::string;


namespace lyx {

using support::bformat;
using frontend::Clipboard;

namespace {

typedef std::pair<pit_type, int> PitPosPair;

typedef limited_stack<pair<ParagraphList, textclass_type> > CutStack;

CutStack theCuts(10);
// persistent selection, cleared until the next selection
CutStack selectionBuffer(1);

// store whether the tabular stack is newer than the normal copy stack
// FIXME: this is a workaround for bug 1919. Should be removed for 1.5,
// when we (hopefully) have a one-for-all paste mechanism.
bool dirty_tabular_stack_ = false;


bool checkPastePossible(int index)
{
	return size_t(index) < theCuts.size() && !theCuts[index].first.empty();
}


pair<PitPosPair, pit_type>
pasteSelectionHelper(Cursor & cur, ParagraphList const & parlist,
		     textclass_type textclass, ErrorList & errorlist)
{
	Buffer const & buffer = cur.buffer();
	pit_type pit = cur.pit();
	pos_type pos = cur.pos();
	ParagraphList & pars = cur.text()->paragraphs();

	if (parlist.empty())
		return make_pair(PitPosPair(pit, pos), pit);

	BOOST_ASSERT (pos <= pars[pit].size());

	// Make a copy of the CaP paragraphs.
	ParagraphList insertion = parlist;
	textclass_type const tc = buffer.params().textclass;

	// Now remove all out of the pars which is NOT allowed in the
	// new environment and set also another font if that is required.

	// Convert newline to paragraph break in ERT inset.
	// This should not be here!
	if (pars[pit].inInset() &&
	    (pars[pit].inInset()->lyxCode() == Inset::ERT_CODE ||
		pars[pit].inInset()->lyxCode() == Inset::LISTINGS_CODE)) {
		for (ParagraphList::size_type i = 0; i < insertion.size(); ++i) {
			for (pos_type j = 0; j < insertion[i].size(); ++j) {
				if (insertion[i].isNewline(j)) {
					// do not track deletion of newline
					insertion[i].eraseChar(j, false);
					breakParagraphConservative(
							buffer.params(),
							insertion, i, j);
				}
			}
		}
	}

	// If we are in an inset which returns forceDefaultParagraphs,
	// set the paragraphs to default
	if (cur.inset().forceDefaultParagraphs(cur.idx())) {
		Layout_ptr const layout =
			buffer.params().getTextClass().defaultLayout();
		ParagraphList::iterator const end = insertion.end();
		for (ParagraphList::iterator par = insertion.begin();
				par != end; ++par)
			par->layout(layout);
	}

	// Make sure there is no class difference.
	InsetText in;
	// This works without copying any paragraph data because we have
	// a specialized swap method for ParagraphList. This is important
	// since we store pointers to insets at some places and we don't
	// want to invalidate them.
	insertion.swap(in.paragraphs());
	cap::switchBetweenClasses(textclass, tc, in, errorlist);
	insertion.swap(in.paragraphs());

	ParagraphList::iterator tmpbuf = insertion.begin();
	int depth_delta = pars[pit].params().depth() - tmpbuf->params().depth();

	depth_type max_depth = pars[pit].getMaxDepthAfter();

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
			if (tmpbuf->getChar(i) == Paragraph::META_INSET &&
			    !pars[pit].insetAllowed(tmpbuf->getInset(i)->lyxCode()))
				// do not track deletion of invalid insets
				tmpbuf->eraseChar(i--, false);
		}

		tmpbuf->setChange(Change(buffer.params().trackChanges ?
					 Change::INSERTED : Change::UNCHANGED));
	}

	bool const empty = pars[pit].empty();
	if (!empty) {
		// Make the buf exactly the same layout as the cursor
		// paragraph.
		insertion.begin()->makeSameLayout(pars[pit]);
	}

	// Prepare the paragraphs and insets for insertion.
	// A couple of insets store buffer references so need updating.
	insertion.swap(in.paragraphs());

	ParIterator fpit = par_iterator_begin(in);
	ParIterator fend = par_iterator_end(in);

	for (; fpit != fend; ++fpit) {
		InsetList::const_iterator lit = fpit->insetlist.begin();
		InsetList::const_iterator eit = fpit->insetlist.end();

		for (; lit != eit; ++lit) {
			switch (lit->inset->lyxCode()) {
			case Inset::TABULAR_CODE: {
				InsetTabular * it = static_cast<InsetTabular*>(lit->inset);
				it->buffer(&buffer);
				break;
			}

			default:
				break; // nothing
			}
		}
	}
	insertion.swap(in.paragraphs());

	// Split the paragraph for inserting the buf if necessary.
	if (!empty)
		breakParagraphConservative(buffer.params(), pars, pit, pos);

	// Paste it!
	if (empty) {
		pars.insert(boost::next(pars.begin(), pit),
			    insertion.begin(),
			    insertion.end());

		// merge the empty par with the last par of the insertion
		mergeParagraph(buffer.params(), pars,
			       pit + insertion.size() - 1);
	} else {
		pars.insert(boost::next(pars.begin(), pit + 1),
			    insertion.begin(),
			    insertion.end());

		// merge the first par of the insertion with the current par
		mergeParagraph(buffer.params(), pars, pit);
	}

	pit_type last_paste = pit + insertion.size() - 1;

	// Store the new cursor position.
	pit = last_paste;
	pos = pars[last_paste].size();

	// Join (conditionally) last pasted paragraph with next one, i.e.,
	// the tail of the spliced document paragraph
	if (!empty && last_paste + 1 != pit_type(pars.size())) {
		if (pars[last_paste + 1].hasSameLayout(pars[last_paste])) {
			mergeParagraph(buffer.params(), pars, last_paste);
		} else if (pars[last_paste + 1].empty()) {
			pars[last_paste + 1].makeSameLayout(pars[last_paste]);
			mergeParagraph(buffer.params(), pars, last_paste);
		} else if (pars[last_paste].empty()) {
			pars[last_paste].makeSameLayout(pars[last_paste + 1]);
			mergeParagraph(buffer.params(), pars, last_paste);
		} else {
			pars[last_paste + 1].stripLeadingSpaces(buffer.params().trackChanges);
			++last_paste;
		}
	}

	return make_pair(PitPosPair(pit, pos), last_paste + 1);
}


PitPosPair eraseSelectionHelper(BufferParams const & params,
	ParagraphList & pars,
	pit_type startpit, pit_type endpit,
	int startpos, int endpos)
{
	// Start of selection is really invalid.
	if (startpit == pit_type(pars.size()) ||
	    (startpos > pars[startpit].size()))
		return PitPosPair(endpit, endpos);

	// Start and end is inside same paragraph
	if (endpit == pit_type(pars.size()) || startpit == endpit) {
		endpos -= pars[startpit].eraseChars(startpos, endpos, params.trackChanges);
		return PitPosPair(endpit, endpos);
	}

	for (pit_type pit = startpit; pit != endpit + 1;) {
		pos_type const left  = (pit == startpit ? startpos : 0);
		pos_type right = (pit == endpit ? endpos : pars[pit].size() + 1);
		bool const merge = pars[pit].isMergedOnEndOfParDeletion(params.trackChanges);

		// Logically erase only, including the end-of-paragraph character
		pars[pit].eraseChars(left, right, params.trackChanges);

		// Separate handling of paragraph break:
		if (merge && pit != endpit &&
		    (pit + 1 != endpit 
		     || pars[pit].hasSameLayout(pars[endpit])
		     || pars[endpit].size() == endpos)) {
			if (pit + 1 == endpit)
				endpos += pars[pit].size();
			mergeParagraph(params, pars, pit);
			--endpit;
		} else
			++pit;
	}

	// Ensure legal cursor pos:
	endpit = startpit;
	endpos = startpos;
	return PitPosPair(endpit, endpos);
}


void putClipboard(ParagraphList const & paragraphs, textclass_type textclass,
		  docstring const & plaintext)
{
	// For some strange reason gcc 3.2 and 3.3 do not accept
	// Buffer buffer(string(), false);
	Buffer buffer("", false);
	buffer.setUnnamed(true);
	buffer.paragraphs() = paragraphs;
	buffer.params().textclass = textclass;
	std::ostringstream lyx;
	if (buffer.write(lyx))
		theClipboard().put(lyx.str(), plaintext);
	else
		theClipboard().put(string(), plaintext);
}


void copySelectionHelper(Buffer const & buf, ParagraphList & pars,
	pit_type startpit, pit_type endpit,
	int start, int end, textclass_type tc, CutStack & cutstack)
{
	BOOST_ASSERT(0 <= start && start <= pars[startpit].size());
	BOOST_ASSERT(0 <= end && end <= pars[endpit].size());
	BOOST_ASSERT(startpit != endpit || start <= end);

	// Clone the paragraphs within the selection.
	ParagraphList copy_pars(boost::next(pars.begin(), startpit),
				boost::next(pars.begin(), endpit + 1));

	// Remove the end of the last paragraph; afterwards, remove the
	// beginning of the first paragraph. Keep this order - there may only
	// be one paragraph!  Do not track deletions here; this is an internal
	// action not visible to the user

	Paragraph & back = copy_pars.back();
	back.eraseChars(end, back.size(), false);
	Paragraph & front = copy_pars.front();
	front.eraseChars(0, start, false);

	ParagraphList::iterator it = copy_pars.begin();
	ParagraphList::iterator it_end = copy_pars.end();

	for (; it != it_end; it++) {
		// ERT paragraphs have the Language latex_language.
		// This is invalid outside of ERT, so we need to change it
		// to the buffer language.
		if (it->ownerCode() == Inset::ERT_CODE || it->ownerCode() == Inset::LISTINGS_CODE) {
			it->changeLanguage(buf.params(), latex_language,
					   buf.getLanguage());
		}
		it->setInsetOwner(0);
	}

	// do not copy text (also nested in insets) which is marked as deleted
	acceptChanges(copy_pars, buf.params());

	cutstack.push(make_pair(copy_pars, tc));
}

} // namespace anon




namespace cap {

void region(CursorSlice const & i1, CursorSlice const & i2,
	Inset::row_type & r1, Inset::row_type & r2,
	Inset::col_type & c1, Inset::col_type & c2)
{
	Inset & p = i1.inset();
	c1 = p.col(i1.idx());
	c2 = p.col(i2.idx());
	if (c1 > c2)
		std::swap(c1, c2);
	r1 = p.row(i1.idx());
	r2 = p.row(i2.idx());
	if (r1 > r2)
		std::swap(r1, r2);
}


docstring grabAndEraseSelection(Cursor & cur)
{
	if (!cur.selection())
		return docstring();
	docstring res = grabSelection(cur);
	eraseSelection(cur);
	return res;
}


bool multipleCellsSelected(Cursor const & cur)
{
	if (!cur.selection() || !cur.inMathed())
		return false;
	
	CursorSlice i1 = cur.selBegin();
	CursorSlice i2 = cur.selEnd();
	if (!i1.inset().asInsetMath())
		return false;
	
	if (i1.idx() == i2.idx())
		return false;
	
	return true;
}


void switchBetweenClasses(textclass_type c1, textclass_type c2,
	InsetText & in, ErrorList & errorlist)
{
	errorlist.clear();

	BOOST_ASSERT(!in.paragraphs().empty());
	if (c1 == c2)
		return;

	TextClass const & tclass1 = textclasslist[c1];
	TextClass const & tclass2 = textclasslist[c2];

	// layouts
	ParIterator end = par_iterator_end(in);
	for (ParIterator it = par_iterator_begin(in); it != end; ++it) {
		docstring const name = it->layout()->name();
		bool hasLayout = tclass2.hasLayout(name);

		if (hasLayout)
			it->layout(tclass2[name]);
		else
			it->layout(tclass2.defaultLayout());

		if (!hasLayout && name != tclass1.defaultLayoutName()) {
			docstring const s = bformat(
						 _("Layout had to be changed from\n%1$s to %2$s\n"
						"because of class conversion from\n%3$s to %4$s"),
			 name, it->layout()->name(),
			 from_utf8(tclass1.name()), from_utf8(tclass2.name()));
			// To warn the user that something had to be done.
			errorlist.push_back(ErrorItem(_("Changed Layout"), s,
						      it->id(), 0,
						      it->size()));
		}
	}

	// character styles
	InsetIterator const i_end = inset_iterator_end(in);
	for (InsetIterator it = inset_iterator_begin(in); it != i_end; ++it) {
		if (it->lyxCode() == Inset::CHARSTYLE_CODE) {
			InsetCharStyle & inset =
				static_cast<InsetCharStyle &>(*it);
			string const name = inset.params().type;
			CharStyles::iterator const found_cs =
				tclass2.charstyle(name);
			if (found_cs == tclass2.charstyles().end()) {
				// The character style is undefined in tclass2
				inset.setUndefined();
				docstring const s = bformat(_(
					"Character style %1$s is "
					"undefined because of class "
					"conversion from\n%2$s to %3$s"),
					 from_utf8(name), from_utf8(tclass1.name()),
					 from_utf8(tclass2.name()));
				// To warn the user that something had to be done.
				errorlist.push_back(ErrorItem(
					_("Undefined character style"),
					s, it.paragraph().id(),	it.pos(), it.pos() + 1));
			} else if (inset.undefined()) {
				// The character style is undefined in
				// tclass1 and is defined in tclass2
				inset.setDefined(found_cs);
			}
		}
	}
}


std::vector<docstring> const availableSelections(Buffer const & buffer)
{
	vector<docstring> selList;

	CutStack::const_iterator cit = theCuts.begin();
	CutStack::const_iterator end = theCuts.end();
	for (; cit != end; ++cit) {
		// we do not use cit-> here because gcc 2.9x does not
		// like it (JMarc)
		ParagraphList const & pars = (*cit).first;
		docstring asciiSel;
		ParagraphList::const_iterator pit = pars.begin();
		ParagraphList::const_iterator pend = pars.end();
		for (; pit != pend; ++pit) {
			asciiSel += pit->asString(buffer, false);
			if (asciiSel.size() > 25) {
				asciiSel.replace(22, docstring::npos,
						 from_ascii("..."));
				break;
			}
		}

		selList.push_back(asciiSel);
	}

	return selList;
}


size_type numberOfSelections()
{
	return theCuts.size();
}


void cutSelection(Cursor & cur, bool doclear, bool realcut)
{
	// This doesn't make sense, if there is no selection
	if (!cur.selection())
		return;

	// OK, we have a selection. This is always between cur.selBegin()
	// and cur.selEnd()

	if (cur.inTexted()) {
		Text * text = cur.text();
		BOOST_ASSERT(text);

		saveSelection(cur);

		// make sure that the depth behind the selection are restored, too
		recordUndoSelection(cur);
		pit_type begpit = cur.selBegin().pit();
		pit_type endpit = cur.selEnd().pit();

		int endpos = cur.selEnd().pos();

		BufferParams const & bp = cur.buffer().params();
		if (realcut) {
			copySelectionHelper(cur.buffer(),
				text->paragraphs(),
				begpit, endpit,
				cur.selBegin().pos(), endpos,
				bp.textclass, theCuts);
			// Stuff what we got on the clipboard.
			// Even if there is no selection.
			putClipboard(theCuts[0].first, theCuts[0].second,
				cur.selectionAsString(true));
		}

		boost::tie(endpit, endpos) =
			eraseSelectionHelper(bp,
				text->paragraphs(),
				begpit, endpit,
				cur.selBegin().pos(), endpos);

		// cutSelection can invalidate the cursor so we need to set
		// it anew. (Lgb)
		// we prefer the end for when tracking changes
		cur.pos() = endpos;
		cur.pit() = endpit;

		// sometimes necessary
		if (doclear
			&& text->paragraphs()[begpit].stripLeadingSpaces(bp.trackChanges))
			cur.fixIfBroken();

		// need a valid cursor. (Lgb)
		cur.clearSelection();
		updateLabels(cur.buffer());

		// tell tabular that a recent copy happened
		dirtyTabularStack(false);
	}

	if (cur.inMathed()) {
		if (cur.selBegin().idx() != cur.selEnd().idx()) {
			// The current selection spans more than one cell.
			// Record all cells
			recordUndoInset(cur);
		} else {
			// Record only the current cell to avoid a jumping
			// cursor after undo
			recordUndo(cur);
		}
		if (realcut)
			copySelection(cur);
		eraseSelection(cur);
	}
}


void copySelection(Cursor & cur)
{
	copySelection(cur, cur.selectionAsString(true));
}


namespace {

void copySelectionToStack(Cursor & cur, CutStack & cutstack)
{
	// this doesn't make sense, if there is no selection
	if (!cur.selection())
		return;

	// copySelection can not yet handle the case of cross idx selection
	if (cur.selBegin().idx() != cur.selEnd().idx())
		return;

	if (cur.inTexted()) {
		Text * text = cur.text();
		BOOST_ASSERT(text);
		// ok we have a selection. This is always between cur.selBegin()
		// and sel_end cursor

		// copy behind a space if there is one
		ParagraphList & pars = text->paragraphs();
		pos_type pos = cur.selBegin().pos();
		pit_type par = cur.selBegin().pit();
		while (pos < pars[par].size() &&
		       pars[par].isLineSeparator(pos) &&
		       (par != cur.selEnd().pit() || pos < cur.selEnd().pos()))
			++pos;

		copySelectionHelper(cur.buffer(), pars, par, cur.selEnd().pit(),
			pos, cur.selEnd().pos(), cur.buffer().params().textclass, cutstack);
		dirtyTabularStack(false);
	}

	if (cur.inMathed()) {
		//lyxerr << "copySelection in mathed" << endl;
		ParagraphList pars;
		Paragraph par;
		BufferParams const & bp = cur.buffer().params();
		par.layout(bp.getTextClass().defaultLayout());
		par.insert(0, grabSelection(cur), Font(), Change(Change::UNCHANGED));
		pars.push_back(par);
		cutstack.push(make_pair(pars, bp.textclass));
	}
}

}


void copySelectionToStack()
{
	if (!selectionBuffer.empty())
		theCuts.push(selectionBuffer[0]);
}


void copySelection(Cursor & cur, docstring const & plaintext)
{
	// In tablemode, because copy and paste actually use special table stack
	// we do not attemp to get selected paragraphs under cursor. Instead, a
	// paragraph with the plain text version is generated so that table cells
	// can be pasted as pure text somewhere else.
	if (cur.selBegin().idx() != cur.selEnd().idx()) {
		ParagraphList pars;
		Paragraph par;
		BufferParams const & bp = cur.buffer().params();
		par.layout(bp.getTextClass().defaultLayout());
		par.insert(0, plaintext, Font(), Change(Change::UNCHANGED));
		pars.push_back(par);
		theCuts.push(make_pair(pars, bp.textclass));
	} else
		copySelectionToStack(cur, theCuts);

	// stuff the selection onto the X clipboard, from an explicit copy request
	putClipboard(theCuts[0].first, theCuts[0].second, plaintext);
}


void saveSelection(Cursor & cur)
{
	// This function is called, not when a selection is formed, but when
	// a selection is cleared. Therefore, multiple keyboard selection
	// will not repeatively trigger this function (bug 3877).
	if (cur.selection() 
	    && cur.selBegin() == cur.bv().cursor().selBegin()
	    && cur.selEnd() == cur.bv().cursor().selEnd()) {
		LYXERR(Debug::ACTION) << BOOST_CURRENT_FUNCTION << ": `"
			   << to_utf8(cur.selectionAsString(true)) << "'."
			   << endl;
		copySelectionToStack(cur, selectionBuffer);
	}
}


bool selection()
{
	return !selectionBuffer.empty();
}


void clearSelection()
{
	selectionBuffer.clear();
}


void clearCutStack()
{
	theCuts.clear();
}


docstring getSelection(Buffer const & buf, size_t sel_index)
{
	return sel_index < theCuts.size()
		? theCuts[sel_index].first.back().asString(buf, false)
		: docstring();
}


void pasteParagraphList(Cursor & cur, ParagraphList const & parlist,
			textclass_type textclass, ErrorList & errorList)
{
	if (cur.inTexted()) {
		Text * text = cur.text();
		BOOST_ASSERT(text);

		pit_type endpit;
		PitPosPair ppp;

		boost::tie(ppp, endpit) =
			pasteSelectionHelper(cur, parlist,
					     textclass, errorList);
		updateLabels(cur.buffer());
		cur.clearSelection();
		text->setCursor(cur, ppp.first, ppp.second);
	}

	// mathed is handled in InsetMathNest/InsetMathGrid
	BOOST_ASSERT(!cur.inMathed());
}


void pasteFromStack(Cursor & cur, ErrorList & errorList, size_t sel_index)
{
	// this does not make sense, if there is nothing to paste
	if (!checkPastePossible(sel_index))
		return;

	recordUndo(cur);
	pasteParagraphList(cur, theCuts[sel_index].first,
			   theCuts[sel_index].second, errorList);
	cur.setSelection();
}


void pasteClipboard(Cursor & cur, ErrorList & errorList, bool asParagraphs)
{
	// Use internal clipboard if it is the most recent one
	if (theClipboard().isInternal()) {
		pasteFromStack(cur, errorList, 0);
		return;
	}

	// First try LyX format
	if (theClipboard().hasLyXContents()) {
		string lyx = theClipboard().getAsLyX();
		if (!lyx.empty()) {
			// For some strange reason gcc 3.2 and 3.3 do not accept
			// Buffer buffer(string(), false);
			Buffer buffer("", false);
			buffer.setUnnamed(true);
			if (buffer.readString(lyx)) {
				recordUndo(cur);
				pasteParagraphList(cur, buffer.paragraphs(),
					buffer.params().textclass, errorList);
				cur.setSelection();
				return;
			}
		}
	}

	// Then try plain text
	docstring const text = theClipboard().getAsText();
	if (text.empty())
		return;
	recordUndo(cur);
	if (asParagraphs)
		cur.text()->insertStringAsParagraphs(cur, text);
	else
		cur.text()->insertStringAsLines(cur, text);
}


void pasteSelection(Cursor & cur, ErrorList & errorList)
{
	if (selectionBuffer.empty())
		return;
	recordUndo(cur);
	pasteParagraphList(cur, selectionBuffer[0].first,
			   selectionBuffer[0].second, errorList);
}


void replaceSelectionWithString(Cursor & cur, docstring const & str, bool backwards)
{
	recordUndo(cur);
	DocIterator selbeg = cur.selectionBegin();

	// Get font setting before we cut
	Font const font =
		selbeg.paragraph().getFontSettings(cur.buffer().params(), selbeg.pos());

	// Insert the new string
	pos_type pos = cur.selEnd().pos();
	Paragraph & par = cur.selEnd().paragraph();
	docstring::const_iterator cit = str.begin();
	docstring::const_iterator end = str.end();
	for (; cit != end; ++cit, ++pos)
		par.insertChar(pos, *cit, font, cur.buffer().params().trackChanges);

	// Cut the selection
	cutSelection(cur, true, false);

	// select the replacement
	if (backwards) {
		selbeg.pos() += str.length();
		cur.setSelection(selbeg, -int(str.length()));
	} else
		cur.setSelection(selbeg, str.length());
}


void replaceSelection(Cursor & cur)
{
	if (cur.selection())
		cutSelection(cur, true, false);
}


void eraseSelection(Cursor & cur)
{
	//lyxerr << "cap::eraseSelection begin: " << cur << endl;
	CursorSlice const & i1 = cur.selBegin();
	CursorSlice const & i2 = cur.selEnd();
	if (i1.inset().asInsetMath()) {
		saveSelection(cur);
		cur.top() = i1;
		if (i1.idx() == i2.idx()) {
			i1.cell().erase(i1.pos(), i2.pos());
			// We may have deleted i1.cell(cur.pos()).
			// Make sure that pos is valid.
			if (cur.pos() > cur.lastpos())
				cur.pos() = cur.lastpos();
		} else {
			InsetMath * p = i1.asInsetMath();
			Inset::row_type r1, r2;
			Inset::col_type c1, c2;
			region(i1, i2, r1, r2, c1, c2);
			for (Inset::row_type row = r1; row <= r2; ++row)
				for (Inset::col_type col = c1; col <= c2; ++col)
					p->cell(p->index(row, col)).clear();
			// We've deleted the whole cell. Only pos 0 is valid.
			cur.pos() = 0;
		}
		// need a valid cursor. (Lgb)
		cur.clearSelection();
	} else {
		lyxerr << "can't erase this selection 1" << endl;
	}
	//lyxerr << "cap::eraseSelection end: " << cur << endl;
}


void selDel(Cursor & cur)
{
	//lyxerr << "cap::selDel" << endl;
	if (cur.selection())
		eraseSelection(cur);
}


void selClearOrDel(Cursor & cur)
{
	//lyxerr << "cap::selClearOrDel" << endl;
	if (lyxrc.auto_region_delete)
		selDel(cur);
	else
		cur.selection() = false;
}


docstring grabSelection(Cursor const & cur)
{
	if (!cur.selection())
		return docstring();

	// FIXME: What is wrong with the following?
#if 0
	std::ostringstream os;
	for (DocIterator dit = cur.selectionBegin();
	     dit != cur.selectionEnd(); dit.forwardPos())
		os << asString(dit.cell());
	return os.str();
#endif

	CursorSlice i1 = cur.selBegin();
	CursorSlice i2 = cur.selEnd();

	if (i1.idx() == i2.idx()) {
		if (i1.inset().asInsetMath()) {
			MathData::const_iterator it = i1.cell().begin();
			return asString(MathData(it + i1.pos(), it + i2.pos()));
		} else {
			return from_ascii("unknown selection 1");
		}
	}

	Inset::row_type r1, r2;
	Inset::col_type c1, c2;
	region(i1, i2, r1, r2, c1, c2);

	docstring data;
	if (i1.inset().asInsetMath()) {
		for (Inset::row_type row = r1; row <= r2; ++row) {
			if (row > r1)
				data += "\\\\";
			for (Inset::col_type col = c1; col <= c2; ++col) {
				if (col > c1)
					data += '&';
				data += asString(i1.asInsetMath()->
					cell(i1.asInsetMath()->index(row, col)));
			}
		}
	} else {
		data = from_ascii("unknown selection 2");
	}
	return data;
}


void dirtyTabularStack(bool b)
{
	dirty_tabular_stack_ = b;
}


bool tabularStackDirty()
{
	return dirty_tabular_stack_;
}


} // namespace cap

} // namespace lyx
