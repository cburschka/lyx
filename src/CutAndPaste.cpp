/**
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

#include "Author.h"
#include "BranchList.h"
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "Cursor.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "FuncCode.h"
#include "FuncRequest.h"
#include "InsetIterator.h"
#include "Language.h"
#include "LyX.h"
#include "LyXRC.h"
#include "Text.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "TextClass.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetBranch.h"
#include "insets/InsetCitation.h"
#include "insets/InsetCommand.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetGraphicsParams.h"
#include "insets/InsetInclude.h"
#include "insets/InsetLabel.h"
#include "insets/InsetTabular.h"

#include "mathed/MathData.h"
#include "mathed/InsetMath.h"
#include "mathed/InsetMathHull.h"
#include "mathed/InsetMathRef.h"
#include "mathed/MathSupport.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/limited_stack.h"
#include "support/lstrings.h"
#include "support/TempFile.h"
#include "support/unique_ptr.h"

#include "frontends/alert.h"
#include "frontends/Clipboard.h"

#include <string>
#include <tuple>

using namespace std;
using namespace lyx::support;
using lyx::frontend::Clipboard;

namespace lyx {

namespace {

typedef pair<pit_type, int> PitPosPair;
typedef pair<DocumentClassConstPtr, AuthorList > DocInfoPair;

typedef limited_stack<pair<ParagraphList, DocInfoPair > > CutStack;

CutStack theCuts(10);
// persistent selection, cleared until the next selection
CutStack selectionBuffer(1);
// temporary scratch area
CutStack tempCut(1);

// store whether the tabular stack is newer than the normal copy stack
// FIXME: this is a workaround for bug 1919. Should be removed for 1.5,
// when we (hopefully) have a one-for-all paste mechanism.
bool dirty_tabular_stack_ = false;


bool checkPastePossible(int index)
{
	return size_t(index) < theCuts.size() && !theCuts[index].first.empty();
}


struct PasteReturnValue {
	PasteReturnValue(pit_type r_pit, pos_type r_pos, bool r_nu) :
	  pit(r_pit), pos(r_pos), needupdate(r_nu)
	{}

	pit_type pit;
	pos_type pos;
	bool needupdate;
};

PasteReturnValue
pasteSelectionHelper(DocIterator const & cur, ParagraphList const & parlist,
                     DocumentClassConstPtr oldDocClass, cap::BranchAction branchAction,
                     ErrorList & errorlist)
{
	Buffer const & buffer = *cur.buffer();
	pit_type pit = cur.pit();
	pos_type pos = cur.pos();
	bool need_update = false;

	if (parlist.empty())
		return PasteReturnValue(pit, pos, need_update);

	// Check whether we paste into an inset that does not
	// produce output (needed for label duplicate check)
	bool in_active_inset = cur.paragraph().inInset().producesOutput();
	if (in_active_inset) {
		for (size_type sl = 0 ; sl < cur.depth() ; ++sl) {
			Paragraph const & outer_par = cur[sl].paragraph();
			if (!outer_par.inInset().producesOutput()) {
				in_active_inset = false;
				break;
			}
		}
	}

	InsetText * target_inset = cur.inset().asInsetText();
	if (!target_inset) {
		InsetTabular * it = cur.inset().asInsetTabular();
		target_inset = it ? it->cell(cur.idx())->asInsetText() : nullptr;
	}
	LASSERT(target_inset, return PasteReturnValue(pit, pos, need_update));

	ParagraphList & pars = target_inset->paragraphs();
	LASSERT(pos <= pars[pit].size(),
			return PasteReturnValue(pit, pos, need_update));

	// Make a copy of the CaP paragraphs.
	ParagraphList insertion = parlist;

	// Now remove all out of the pars which is NOT allowed in the
	// new environment and set also another font if that is required.

	// Merge paragraphs that are to be pasted into a text inset
	// that does not allow multiple pars.
	InsetText * inset_text = target_inset->asInsetText();
	if (inset_text && !inset_text->allowMultiPar()) {
		while (insertion.size() > 1)
			mergeParagraph(buffer.params(), insertion, 0);
	}

	// Convert newline to paragraph break in ParbreakIsNewline
	if (target_inset->getLayout().parbreakIsNewline()
	    || pars[pit].layout().parbreak_is_newline) {
		for (size_t i = 0; i != insertion.size(); ++i) {
			for (pos_type j = 0; j != insertion[i].size(); ++j) {
				if (insertion[i].isNewline(j)) {
					// do not track deletion of newline
					insertion[i].eraseChar(j, false);
					insertion[i].setInsetOwner(target_inset);
					breakParagraphConservative(
							buffer.params(),
							insertion, i, j);
					break;
				}
			}
		}
	}

	// Prevent to paste uncodable characters in verbatim and ERT.
	// The encoding is inherited from the context here.
	docstring uncodable_content;
	if (target_inset->getLayout().isPassThru() && cur.getEncoding()) {
		odocstringstream res;
		Encoding const * e = cur.getEncoding();
		for (size_t i = 0; i != insertion.size(); ++i) {
			pos_type end = insertion[i].size();
			for (pos_type j = 0; j != end; ++j) {
				// skip insets
				if (insertion[i].isInset(j))
					continue;
				char_type const c = insertion[i].getChar(j);
				if (!e->encodable(c)) {
					// do not track deletion
					res.put(c);
					insertion[i].eraseChar(j, false);
					--end;
					--j;
				}
			}
		}
		docstring const uncodable = res.str();
		if (!uncodable.empty()) {
			if (uncodable.size() == 1)
				uncodable_content = bformat(_("The character \"%1$s\" is uncodable in this verbatim context "
						      "and thus has not been pasted."),
						    uncodable);
			else
				uncodable_content = bformat(_("The characters \"%1$s\" are uncodable in this verbatim context "
						      "and thus have not been pasted."),
						    uncodable);
		}
	}

	// set the paragraphs to plain layout if necessary
	DocumentClassConstPtr newDocClass = buffer.params().documentClassPtr();
	Layout const & plainLayout = newDocClass->plainLayout();
	Layout const & defaultLayout = newDocClass->defaultLayout();
	if (cur.inset().usePlainLayout()) {
		bool forcePlainLayout = target_inset->forcePlainLayout();
		for (auto & par : insertion) {
			Layout const & parLayout = par.layout();
			if (forcePlainLayout || parLayout == defaultLayout)
				par.setLayout(plainLayout);
		}
	} else {
		// check if we need to reset from plain layout
		for (auto & par : insertion) {
			Layout const & parLayout = par.layout();
			if (parLayout == plainLayout)
				par.setLayout(defaultLayout);
		}
	}

	InsetText in(cur.buffer());
	// Make sure there is no class difference.
	in.paragraphs().clear();
	// This works without copying any paragraph data because we have
	// a specialized swap method for ParagraphList. This is important
	// since we store pointers to insets at some places and we don't
	// want to invalidate them.
	insertion.swap(in.paragraphs());
	cap::switchBetweenClasses(oldDocClass, newDocClass, in, errorlist);
	// Do this here since switchBetweenClasses clears the errorlist
	if (!uncodable_content.empty())
		errorlist.push_back(ErrorItem(_("Uncodable content"), uncodable_content));
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

		// Set max_depth for the next paragraph
		max_depth = tmpbuf->getMaxDepthAfter();

		// Set the inset owner of this paragraph.
		tmpbuf->setInsetOwner(target_inset);
		for (pos_type i = 0; i < tmpbuf->size(); ++i) {
			// do not track deletion of invalid insets
			if (Inset * inset = tmpbuf->getInset(i))
				if (!target_inset->insetAllowed(inset->lyxCode()))
					tmpbuf->eraseChar(i--, false);
		}

		if (lyxrc.ct_markup_copied) {
			// Only change to inserted if ct is active,
			// otherwise leave markup as is
			if (buffer.params().track_changes)
				tmpbuf->setChange(Change(Change::INSERTED));
		} else
			// Resolve all markup to inserted or unchanged
			tmpbuf->setChange(Change(buffer.params().track_changes ?
						 Change::INSERTED : Change::UNCHANGED));
	}

	bool const empty = pars[pit].empty();
	if (!empty) {
		// Make the buf exactly the same layout as the cursor
		// paragraph.
		insertion.begin()->makeSameLayout(pars[pit]);
	}

	// Prepare the paragraphs and insets for insertion.
	insertion.swap(in.paragraphs());

	InsetIterator const i_end = end(in);
	for (InsetIterator it = begin(in); it != i_end; ++it) {
		// Even though this will also be done later, it has to be done here
		// since some inset might try to access the buffer() member.
		it->setBuffer(const_cast<Buffer &>(buffer));
		switch (it->lyxCode()) {

		case MATH_HULL_CODE: {
			// check for equation labels and resolve duplicates
			InsetMathHull * ins = it->asInsetMath()->asHullInset();
			std::vector<InsetLabel *> labels = ins->getLabels();
			for (size_t i = 0; i != labels.size(); ++i) {
				if (!labels[i])
					continue;
				InsetLabel * lab = labels[i];
				docstring const oldname = lab->getParam("name");
				lab->updateLabel(oldname, in_active_inset);
				// We need to update the buffer reference cache.
				need_update = true;
				docstring const newname = lab->getParam("name");
				if (oldname == newname)
					continue;
				// adapt the references
				for (InsetIterator itt = begin(in);
				      itt != i_end; ++itt) {
					if (itt->lyxCode() == REF_CODE) {
						InsetCommand * ref = itt->asInsetCommand();
						if (ref->getParam("reference") == oldname)
							ref->setParam("reference", newname);
					} else if (itt->lyxCode() == MATH_REF_CODE) {
						InsetMathRef * mi = itt->asInsetMath()->asRefInset();
						// this is necessary to prevent an uninitialized
						// buffer when the RefInset is in a MathBox.
						// FIXME audit setBuffer calls
						mi->setBuffer(const_cast<Buffer &>(buffer));
						if (mi->getTarget() == oldname)
							mi->changeTarget(newname);
					}
				}
			}
			break;
		}

		case LABEL_CODE: {
			// check for duplicates
			InsetLabel & lab = static_cast<InsetLabel &>(*it);
			docstring const oldname = lab.getParam("name");
			lab.updateLabel(oldname, in_active_inset);
			// We need to update the buffer reference cache.
			need_update = true;
			docstring const newname = lab.getParam("name");
			if (oldname == newname)
				break;
			// adapt the references
			for (InsetIterator itt = begin(in); itt != i_end; ++itt) {
				if (itt->lyxCode() == REF_CODE) {
					InsetCommand & ref = static_cast<InsetCommand &>(*itt);
					if (ref.getParam("reference") == oldname)
						ref.setParam("reference", newname);
				} else if (itt->lyxCode() == MATH_REF_CODE) {
					InsetMathRef * mi = itt->asInsetMath()->asRefInset();
					// this is necessary to prevent an uninitialized
					// buffer when the RefInset is in a MathBox.
					// FIXME audit setBuffer calls
					mi->setBuffer(const_cast<Buffer &>(buffer));
					if (mi->getTarget() == oldname)
						mi->changeTarget(newname);
				}
			}
			break;
		}

		case INCLUDE_CODE: {
			InsetInclude & inc = static_cast<InsetInclude &>(*it);
			inc.updateCommand();
			// We need to update the list of included files.
			need_update = true;
			break;
		}

		case CITE_CODE: {
			InsetCitation & cit = static_cast<InsetCitation &>(*it);
			// This actually only needs to be done if the cite engine
			// differs, but we do it in general.
			cit.redoLabel();
			// We need to update the list of citations.
			need_update = true;
			break;
		}

		case BIBITEM_CODE: {
			// check for duplicates
			InsetBibitem & bib = static_cast<InsetBibitem &>(*it);
			docstring const oldkey = bib.getParam("key");
			bib.updateCommand(oldkey, false);
			// We need to update the buffer reference cache.
			need_update = true;
			docstring const newkey = bib.getParam("key");
			if (oldkey == newkey)
				break;
			// adapt the references
			for (InsetIterator itt = begin(in);
			     itt != i_end; ++itt) {
				if (itt->lyxCode() == CITE_CODE) {
					InsetCommand * ref = itt->asInsetCommand();
					if (ref->getParam("key") == oldkey)
						ref->setParam("key", newkey);
				}
			}
			break;
		}

		case BRANCH_CODE: {
			// check if branch is known to target buffer
			// or its master
			InsetBranch & br = static_cast<InsetBranch &>(*it);
			docstring const name = br.branch();
			if (name.empty())
				break;
			bool const is_child = (&buffer != buffer.masterBuffer());
			BranchList branchlist = buffer.params().branchlist();
			if ((!is_child && branchlist.find(name))
			    || (is_child && (branchlist.find(name)
			        || buffer.masterBuffer()->params().branchlist().find(name))))
				break;
			switch(branchAction) {
			case cap::BRANCH_ADD: {
				// This is for a temporary buffer, so simply create the branch.
				// Must not use lyx::dispatch(), since tmpbuffer has no view.
				DispatchResult dr;
				const_cast<Buffer&>(buffer).dispatch(FuncRequest(LFUN_BRANCH_ADD, name), dr);
				break;
			}
			case cap::BRANCH_ASK: {
				docstring text = bformat(
					_("The pasted branch \"%1$s\" is undefined.\n"
					  "Do you want to add it to the document's branch list?"),
					name);
				if (frontend::Alert::prompt(_("Unknown branch"),
					  text, 0, 1, _("&Add"), _("&Don't Add")) != 0)
					break;
				lyx::dispatch(FuncRequest(LFUN_BRANCH_ADD, name));
				break;
			}
			case cap::BRANCH_IGNORE:
				break;
			}
			// We need to update the list of branches.
			need_update = true;
			break;
		}

		default:
			break; // nothing
		}
	}
	insertion.swap(in.paragraphs());

	// Split the paragraph for inserting the buf if necessary.
	if (!empty)
		breakParagraphConservative(buffer.params(), pars, pit, pos);

	// Paste it!
	if (empty) {
		pars.insert(pars.iterator_at(pit),
		            insertion.begin(), insertion.end());

		// merge the empty par with the last par of the insertion
		mergeParagraph(buffer.params(), pars,
			       pit + insertion.size() - 1);
	} else {
		pars.insert(pars.iterator_at(pit + 1),
		            insertion.begin(), insertion.end());

		// merge the first par of the insertion with the current par
		mergeParagraph(buffer.params(), pars, pit);
	}

	// Store the new cursor position.
	pit_type last_paste = pit + insertion.size() - 1;
	pit_type startpit = pit;
	pit = last_paste;
	pos = pars[last_paste].size();

	// FIXME Should we do it here, or should we let updateBuffer() do it?
	// Set paragraph buffers. It's important to do this right away
	// before something calls Inset::buffer() and causes a crash.
	for (pit_type p = startpit; p <= pit; ++p)
		pars[p].setInsetBuffers(const_cast<Buffer &>(buffer));

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
			pars[last_paste + 1].stripLeadingSpaces(buffer.params().track_changes);
			++last_paste;
		}
	}

	return PasteReturnValue(pit, pos, need_update);
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
		endpos -= pars[startpit].eraseChars(startpos, endpos, params.track_changes);
		return PitPosPair(endpit, endpos);
	}

	for (pit_type pit = startpit; pit != endpit + 1;) {
		pos_type const left  = (pit == startpit ? startpos : 0);
		pos_type right = (pit == endpit ? endpos : pars[pit].size() + 1);
		bool const merge = pars[pit].isMergedOnEndOfParDeletion(params.track_changes);

		// Logically erase only, including the end-of-paragraph character
		pars[pit].eraseChars(left, right, params.track_changes);

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


Buffer * copyToTempBuffer(ParagraphList const & paragraphs, DocumentClassConstPtr docclass)
{
	// This used to need to be static to avoid a memory leak. It no longer needs
	// to be so, but the alternative is to construct a new one of these (with a
	// new temporary directory, etc) every time, and then to destroy it. So maybe
	// it's worth just keeping this one around.
	static TempFile tempfile("clipboard.internal");
	tempfile.setAutoRemove(false);
	// The initialization of staticbuffer is thread-safe. Using a lambda
	// guarantees that the properties are set only once.
	static Buffer * staticbuffer = [&](){
		Buffer * b =
			theBufferList().newInternalBuffer(tempfile.name().absFileName());
		b->setUnnamed(true);
		b->inset().setBuffer(*b);
		//initialize staticbuffer with b
		return b;
	}();
	// Use a clone for the complicated stuff so that we do not need to clean
	// up in order to avoid a crash.
	Buffer * buffer = staticbuffer->cloneBufferOnly();
	LASSERT(buffer, return nullptr);

	// This needs doing every time.
	// Since setDocumentClass() causes deletion of the old document class
	// we need to reset all layout pointers in paragraphs (otherwise they
	// would be dangling).
	ParIterator const end = buffer->par_iterator_end();
	for (ParIterator it = buffer->par_iterator_begin(); it != end; ++it) {
		docstring const name = it->layout().name();
		if (docclass->hasLayout(name))
			it->setLayout((*docclass)[name]);
		else
			it->setPlainOrDefaultLayout(*docclass);
	}
	buffer->params().setDocumentClass(docclass);

	// we will use pasteSelectionHelper to copy the paragraphs into the
	// temporary Buffer, since it does a lot of things to fix them up.
	DocIterator dit = doc_iterator_begin(buffer, &buffer->inset());
	ErrorList el;
	pasteSelectionHelper(dit, paragraphs, docclass, cap::BRANCH_ADD, el);

	return buffer;
}


void putClipboard(ParagraphList const & paragraphs,
		  DocInfoPair docinfo, docstring const & plaintext,
		  BufferParams const & bp)
{
	Buffer * buffer = copyToTempBuffer(paragraphs, docinfo.first);
	if (!buffer) // already asserted in copyToTempBuffer()
		return;

	// We don't want to produce images that are not used. Therefore,
	// output formulas as MathML. Even if this is not understood by all
	// applications, the number that can parse it should go up in the future.
	buffer->params().html_math_output = BufferParams::MathML;

	// Copy authors to the params. We need those pointers.
	for (Author const & a : bp.authors())
		buffer->params().authors().record(a);

	// Make sure MarkAsExporting is deleted before buffer is
	{
		// The Buffer is being used to export. This is necessary so that the
		// updateMacros call will record the needed information.
		MarkAsExporting mex(buffer);

		buffer->updateBuffer(Buffer::UpdateMaster, OutputUpdate);
		buffer->updateMacros();
		buffer->updateMacroInstances(OutputUpdate);

		// LyX's own format
		string lyx;
		ostringstream oslyx;
		if (buffer->write(oslyx))
			lyx = oslyx.str();

		// XHTML format
		odocstringstream oshtml;
		OutputParams runparams(encodings.fromLyXName("utf8"));
		// We do not need to produce images, etc.
		runparams.dryrun = true;
		// We are not interested in errors (bug 8866)
		runparams.silent = true;
		buffer->writeLyXHTMLSource(oshtml, runparams, Buffer::FullSource);

		theClipboard().put(lyx, oshtml.str(), plaintext);
	}

	// Save that memory
	delete buffer;
}


/// return true if the whole ParagraphList is deleted
static bool isFullyDeleted(ParagraphList const & pars)
{
	pit_type const pars_size = static_cast<pit_type>(pars.size());

	// check all paragraphs
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		if (!pars[pit].empty())   // prevent assertion failure
			if (!pars[pit].isDeleted(0, pars[pit].size()))
				return false;
	}
	return true;
}


void copySelectionHelper(Buffer const & buf, Text const & text,
	pit_type startpit, pit_type endpit,
	int start, int end, DocumentClassConstPtr dc, CutStack & cutstack)
{
	ParagraphList const & pars = text.paragraphs();

	// In most of these cases, we can try to recover.
	LASSERT(0 <= start, start = 0);
	LASSERT(start <= pars[startpit].size(), start = pars[startpit].size());
	LASSERT(0 <= end, end = 0);
	LASSERT(end <= pars[endpit].size(), end = pars[endpit].size());
	LASSERT(startpit != endpit || start <= end, return);

	// Clone the paragraphs within the selection.
	ParagraphList copy_pars(pars.iterator_at(startpit),
				pars.iterator_at(endpit + 1));

	// Remove the end of the last paragraph; afterwards, remove the
	// beginning of the first paragraph. Keep this order - there may only
	// be one paragraph!  Do not track deletions here; this is an internal
	// action not visible to the user

	Paragraph & back = copy_pars.back();
	back.eraseChars(end, back.size(), false);
	Paragraph & front = copy_pars.front();
	front.eraseChars(0, start, false);

	for (auto & par : copy_pars) {
		// Since we have a copy of the paragraphs, the insets
		// do not have a proper buffer reference. It makes
		// sense to add them temporarily, because the
		// operations below depend on that (acceptChanges included).
		par.setInsetBuffers(const_cast<Buffer &>(buf));
		// PassThru paragraphs have the Language
		// latex_language. This is invalid for others, so we
		// need to change it to the buffer language.
		if (par.isPassThru())
			par.changeLanguage(buf.params(),
					   latex_language, buf.language());
	}

	// do not copy text (also nested in insets) which is marked as
	// deleted, unless the whole selection was deleted
	if (!lyxrc.ct_markup_copied) {
		if (!isFullyDeleted(copy_pars))
			acceptChanges(copy_pars, buf.params());
		else
			rejectChanges(copy_pars, buf.params());
	}


	// do some final cleanup now, to make sure that the paragraphs
	// are not linked to something else.
	for (auto & par : copy_pars) {
		par.resetBuffer();
		par.setInsetOwner(nullptr);
	}

	cutstack.push(make_pair(copy_pars, make_pair(dc, buf.params().authors())));
}

} // namespace


namespace cap {

void region(CursorSlice const & i1, CursorSlice const & i2,
			row_type & r1, row_type & r2,
			col_type & c1, col_type & c2)
{
	Inset const & p = i1.inset();
	c1 = p.col(i1.idx());
	c2 = p.col(i2.idx());
	if (c1 > c2)
		swap(c1, c2);
	r1 = p.row(i1.idx());
	r2 = p.row(i2.idx());
	if (r1 > r2)
		swap(r1, r2);
}


docstring grabAndEraseSelection(Cursor & cur)
{
	if (!cur.selection())
		return docstring();
	docstring res = grabSelection(cur);
	eraseSelection(cur);
	return res;
}


bool reduceSelectionToOneCell(CursorData & cur)
{
	if (!cur.selection() || !cur.inMathed())
		return false;

	CursorSlice i1 = cur.selBegin();
	CursorSlice i2 = cur.selEnd();
	if (!i1.inset().asInsetMath())
		return false;

	// the easy case: do nothing if only one cell is selected
	if (i1.idx() == i2.idx())
		return true;

	cur.top().pos() = 0;
	cur.resetAnchor();
	cur.top().pos() = cur.top().lastpos();

	return true;
}


bool multipleCellsSelected(CursorData const & cur)
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


void switchBetweenClasses(DocumentClassConstPtr oldone,
		DocumentClassConstPtr newone, InsetText & in, ErrorList & errorlist)
{
	errorlist.clear();

	LBUFERR(!in.paragraphs().empty());
	if (oldone == newone)
		return;

	DocumentClass const & oldtc = *oldone;
	DocumentClass const & newtc = *newone;

	// layouts
	ParIterator it = par_iterator_begin(in);
	ParIterator pend = par_iterator_end(in);
	// for remembering which layouts we've had to add
	set<docstring> newlayouts;
	for (; it != pend; ++it) {
		docstring const name = it->layout().name();

		// the pasted text will keep their own layout name. If this layout does
		// not exist in the new document, it will behave like a standard layout.
		bool const added_one = newtc.addLayoutIfNeeded(name);
		if (added_one)
			newlayouts.insert(name);

		if (added_one || newlayouts.find(name) != newlayouts.end()) {
			// Warn the user.
			docstring const s = bformat(_("Layout `%1$s' was not found."), name);
			errorlist.push_back(ErrorItem(_("Layout Not Found"), s,
			                              {it->id(), 0}, {it->id(), -1}));
		}

		if (in.usePlainLayout())
			it->setLayout(newtc.plainLayout());
		else
			it->setLayout(newtc[name]);
	}

	// character styles and hidden table cells
	InsetIterator const i_end = end(in);
	for (InsetIterator iit = begin(in); iit != i_end; ++iit) {
		InsetCode const code = iit->lyxCode();
		if (code == FLEX_CODE) {
			// FIXME: Should we verify all InsetCollapsible?
			docstring const layoutName = iit->layoutName();
			docstring const & n = newone->insetLayout(layoutName).name();
			bool const is_undefined = n.empty() ||
				n == DocumentClass::plainInsetLayout().name();
			if (!is_undefined)
				continue;

			// The flex inset is undefined in newtc
			docstring const oldname = from_utf8(oldtc.name());
			docstring const newname = from_utf8(newtc.name());
			docstring s;
			if (oldname == newname)
				s = bformat(_("Flex inset %1$s is undefined after "
					"reloading `%2$s' layout."), layoutName, oldname);
			else
				s = bformat(_("Flex inset %1$s is undefined because of "
					"conversion from `%2$s' layout to `%3$s'."),
					layoutName, oldname, newname);
			// To warn the user that something had to be done.
			errorlist.push_back(ErrorItem(
			                              _("Undefined flex inset"), s,
						      {iit.paragraph().id(), iit.pos()},
						      {iit.paragraph().id(), iit.pos() + 1}));
		} else if (code == TABULAR_CODE) {
			// The recursion above does not catch paragraphs in "hidden" cells,
			// i.e., ones that are part of a multirow or multicolum. So we need
			// to handle those separately.
			// This is the cause of bug #9049.
			InsetTabular * table = iit->asInsetTabular();
			table->setLayoutForHiddenCells(newtc);
		}
	}
}


vector<docstring> availableSelections(Buffer const * buf)
{
	vector<docstring> selList;
	if (!buf)
		return selList;

	for (auto const & cut : theCuts) {
		ParagraphList const & pars = cut.first;
		docstring textSel;
		for (auto const & para : pars) {
			Paragraph par(para, 0, 46);
			// adapt paragraph to current buffer.
			par.setInsetBuffers(const_cast<Buffer &>(*buf));
			textSel += par.asString(AS_STR_INSETS);
			if (textSel.size() > 45) {
				support::truncateWithEllipsis(textSel,45);
				break;
			}
		}
		selList.push_back(textSel);
	}

	return selList;
}


size_type numberOfSelections()
{
	return theCuts.size();
}

namespace {

void cutSelectionHelper(Cursor & cur, CutStack & cuts, bool realcut, bool putclip)
{
	// This doesn't make sense, if there is no selection
	if (!cur.selection())
		return;

	// OK, we have a selection. This is always between cur.selBegin()
	// and cur.selEnd()

	if (cur.inTexted()) {
		Text * text = cur.text();
		LBUFERR(text);

		saveSelection(cur);

		// make sure that the depth behind the selection are restored, too
		cur.recordUndoSelection();
		pit_type begpit = cur.selBegin().pit();
		pit_type endpit = cur.selEnd().pit();

		int endpos = cur.selEnd().pos();

		BufferParams const & bp = cur.buffer()->params();
		if (realcut) {
			copySelectionHelper(*cur.buffer(),
				*text,
				begpit, endpit,
				cur.selBegin().pos(), endpos,
				bp.documentClassPtr(), cuts);
			// Stuff what we got on the clipboard.
			// Even if there is no selection.
			if (putclip)
				putClipboard(cuts[0].first, cuts[0].second,
				             cur.selectionAsString(true, true), bp);
		}

		if (begpit != endpit)
			cur.screenUpdateFlags(Update::Force | Update::FitCursor);

		tie(endpit, endpos) =
			eraseSelectionHelper(bp, text->paragraphs(), begpit, endpit,
			                     cur.selBegin().pos(), endpos);

		// cutSelection can invalidate the cursor so we need to set
		// it anew. (Lgb)
		// we prefer the end for when tracking changes
		cur.pos() = endpos;
		cur.pit() = endpit;

		// need a valid cursor. (Lgb)
		cur.clearSelection();

		// After a cut operation, we must make sure that the Buffer is updated
		// because some further operation might need updated label information for
		// example. So we cannot just use "cur.forceBufferUpdate()" here.
		// This fixes #7071.
		cur.buffer()->updateBuffer();

		// tell tabular that a recent copy happened
		dirtyTabularStack(false);
	}

	if (cur.inMathed()) {
		if (cur.selBegin().idx() != cur.selEnd().idx()) {
			// The current selection spans more than one cell.
			// Record all cells
			cur.recordUndoInset();
		} else {
			// Record only the current cell to avoid a jumping
			// cursor after undo
			cur.recordUndo();
		}
		if (realcut)
			copySelection(cur);
		eraseSelection(cur);
	}
}

} // namespace

void cutSelection(Cursor & cur, bool realcut)
{
	cutSelectionHelper(cur, theCuts, realcut, true);
}


void cutSelectionToTemp(Cursor & cur, bool realcut)
{
	cutSelectionHelper(cur, tempCut, realcut, false);
}


void copySelection(Cursor const & cur)
{
	copySelection(cur, cur.selectionAsString(true, true));
}


void copyInset(Cursor const & cur, Inset * inset, docstring const & plaintext)
{
	ParagraphList pars;
	Paragraph par;
	BufferParams const & bp = cur.buffer()->params();
	par.setLayout(bp.documentClass().plainLayout());
	Font font(inherit_font, bp.language);
	par.insertInset(0, inset, font, Change(Change::UNCHANGED));
	pars.push_back(par);
	theCuts.push(make_pair(pars, make_pair(bp.documentClassPtr(), bp.authors())));

	// stuff the selection onto the X clipboard, from an explicit copy request
	putClipboard(theCuts[0].first, theCuts[0].second, plaintext, bp);
}


namespace {

void copySelectionToStack(CursorData const & cur, CutStack & cutstack)
{
	// this doesn't make sense, if there is no selection
	if (!cur.selection())
		return;

	// copySelection can not yet handle the case of cross idx selection
	if (cur.selBegin().idx() != cur.selEnd().idx())
		return;

	if (cur.inTexted()) {
		Text * text = cur.text();
		LBUFERR(text);
		// ok we have a selection. This is always between cur.selBegin()
		// and sel_end cursor
		copySelectionHelper(*cur.buffer(), *text,
		                    cur.selBegin().pit(), cur.selEnd().pit(),
		                    cur.selBegin().pos(), cur.selEnd().pos(),
		                    cur.buffer()->params().documentClassPtr(),
		                    cutstack);
		// Reset the dirty_tabular_stack_ flag only when something
		// is copied to the clipboard (not to the selectionBuffer).
		if (&cutstack == &theCuts)
			dirtyTabularStack(false);
	}

	if (cur.inMathed()) {
		//lyxerr << "copySelection in mathed" << endl;
		ParagraphList pars;
		Paragraph par;
		BufferParams const & bp = cur.buffer()->params();
		// FIXME This should be the plain layout...right?
		par.setLayout(bp.documentClass().plainLayout());
		// For pasting into text, we set the language to the paragraph language
		// (rather than the default_language which is always English; see #2596)
		par.insert(0, grabSelection(cur), Font(sane_font, par.getParLanguage(bp)),
			   Change(Change::UNCHANGED));
		pars.push_back(par);
		cutstack.push(make_pair(pars, make_pair(bp.documentClassPtr(), bp.authors())));
	}
}

} // namespace


void copySelectionToStack()
{
	if (!selectionBuffer.empty())
		theCuts.push(selectionBuffer[0]);
}


void copySelectionToTemp(Cursor const & cur)
{
	copySelectionToStack(cur, tempCut);
}


void copySelection(Cursor const & cur, docstring const & plaintext)
{
	// In tablemode, because copy and paste actually use a special table stack,
	// we need to go through the cells and collect the paragraphs.
	// In math matrices, we generate a plain text version.
	if (cur.selBegin().idx() != cur.selEnd().idx()) {
		ParagraphList pars;
		BufferParams const & bp = cur.buffer()->params();
		if (cur.inMathed()) {
			Paragraph par;
			par.setLayout(bp.documentClass().plainLayout());
			// Replace (column-separating) tabs by space (#4449)
			docstring const clean_text = subst(plaintext, '\t', ' ');
			// For pasting into text, we set the language to the paragraph language
			// (rather than the default_language which is always English; see #11898)
			par.insert(0, clean_text, Font(sane_font, par.getParLanguage(bp)),
				   Change(Change::UNCHANGED));
			pars.push_back(par);
		} else {
			// Get paragraphs from all cells
			InsetTabular * table = cur.inset().asInsetTabular();
			LASSERT(table, return);
			ParagraphList tplist = table->asParList(cur.selBegin().idx(), cur.selEnd().idx());
			for (auto & cpar : tplist) {
				cpar.setLayout(bp.documentClass().plainLayout());
				pars.push_back(cpar);
				// since the pars are merged later, we separate them by blank
				Paragraph epar;
				epar.insert(0, from_ascii(" "), Font(sane_font, epar.getParLanguage(bp)),
					    Change(Change::UNCHANGED));
				pars.push_back(epar);
			}
			// remove last empty par
			pars.pop_back();
			// merge all paragraphs to one
			while (pars.size() > 1)
				mergeParagraph(bp, pars, 0);
		}
		theCuts.push(make_pair(pars, make_pair(bp.documentClassPtr(), bp.authors())));
	} else {
		copySelectionToStack(cur, theCuts);
	}

	// stuff the selection onto the X clipboard, from an explicit copy request
	putClipboard(theCuts[0].first, theCuts[0].second, plaintext,
			cur.buffer()->params());
}


void saveSelection(Cursor const & cur)
{
	// This function is called, not when a selection is formed, but when
	// a selection is cleared. Therefore, multiple keyboard selection
	// will not repeatively trigger this function (bug 3877).
	if (cur.selection()
	    && cur.selBegin() == cur.bv().cursor().selBegin()
	    && cur.selEnd() == cur.bv().cursor().selEnd()) {
		LYXERR(Debug::SELECTION, "saveSelection: '" << cur.selectionAsString(true, true) << "'");
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
	tempCut.clear();
}


docstring selection(size_t sel_index, DocInfoPair docinfo, bool for_math)
{
	if (sel_index >= theCuts.size())
		return docstring();

	unique_ptr<Buffer> buffer(copyToTempBuffer(theCuts[sel_index].first,
	                                           docinfo.first));
	if (!buffer)
		return docstring();

	int options = AS_STR_INSETS | AS_STR_NEWLINES;
	if (for_math)
		options |= AS_STR_MATHED;

	return buffer->paragraphs().back().asString(options);
}


void pasteParagraphList(Cursor & cur, ParagraphList const & parlist,
			DocumentClassConstPtr docclass, AuthorList const & authors,
			ErrorList & errorList,
			cap::BranchAction branchAction)
{
	// Copy authors to the params. We need those pointers.
	for (Author const & a : authors)
		cur.buffer()->params().authors().record(a);
	
	if (cur.inTexted()) {
		Text * text = cur.text();
		LBUFERR(text);

		PasteReturnValue prv =
			pasteSelectionHelper(cur, parlist, docclass, branchAction, errorList);
		cur.forceBufferUpdate();
		cur.clearSelection();
		text->setCursor(cur, prv.pit, prv.pos);
	}

	// mathed is handled in InsetMathNest/InsetMathGrid
	LATTEST(!cur.inMathed());
}


bool pasteFromStack(Cursor & cur, ErrorList & errorList, size_t sel_index)
{
	// this does not make sense, if there is nothing to paste
	if (!checkPastePossible(sel_index))
		return false;

	cur.recordUndo();
	pasteParagraphList(cur, theCuts[sel_index].first,
	                   theCuts[sel_index].second.first, theCuts[sel_index].second.second,
			   errorList, BRANCH_ASK);
	return true;
}


bool pasteFromTemp(Cursor & cur, ErrorList & errorList)
{
	// this does not make sense, if there is nothing to paste
	if (tempCut.empty() || tempCut[0].first.empty())
		return false;

	cur.recordUndo();
	pasteParagraphList(cur, tempCut[0].first,
			tempCut[0].second.first, tempCut[0].second.second,
			errorList, BRANCH_IGNORE);
	return true;
}


bool pasteClipboardText(Cursor & cur, ErrorList & errorList, bool asParagraphs,
                        Clipboard::TextType type)
{
	// Use internal clipboard if it is the most recent one
	// This overrides asParagraphs and type on purpose!
	if (theClipboard().isInternal())
		return pasteFromStack(cur, errorList, 0);

	// First try LyX format
	if ((type == Clipboard::LyXTextType ||
	     type == Clipboard::LyXOrPlainTextType ||
	     type == Clipboard::AnyTextType) &&
	    theClipboard().hasTextContents(Clipboard::LyXTextType)) {
		string lyx = theClipboard().getAsLyX();
		if (!lyx.empty()) {
			Buffer buffer(string(), false);
			buffer.setUnnamed(true);
			if (buffer.readString(lyx)) {
				cur.recordUndo();
				pasteParagraphList(cur, buffer.paragraphs(),
						   buffer.params().documentClassPtr(),
						   buffer.params().authors(),
						   errorList);
				return true;
			}
		}
	}

	// Then try TeX and HTML
	Clipboard::TextType types[2] = {Clipboard::HtmlTextType, Clipboard::LaTeXTextType};
	string names[2] = {"html", "latexclipboard"};
	for (int i = 0; i < 2; ++i) {
		if (type != types[i] && type != Clipboard::AnyTextType)
			continue;
		bool available = theClipboard().hasTextContents(types[i]);

		// If a specific type was explicitly requested, try to
		// interpret plain text: The user told us that the clipboard
		// contents is in the desired format
		if (!available && type == types[i]) {
			types[i] = Clipboard::PlainTextType;
			available = theClipboard().hasTextContents(types[i]);
		}

		if (available) {
			docstring text = theClipboard().getAsText(types[i]);
			available = !text.empty();
			if (available) {
				Buffer buffer(string(), false);
				buffer.setUnnamed(true);
				available = buffer.importString(names[i], text, errorList);
				if (available)
					available = !buffer.paragraphs().empty();
				if (available && !buffer.paragraphs()[0].empty()) {
					// TeX2lyx (also used in the HTML chain) assumes English as document language
					// if no language is explicitly set (as is the case here).
					// We thus reset the temp buffer's language to the context language
					buffer.changeLanguage(buffer.language(), cur.getFont().language());
					cur.recordUndo();
					pasteParagraphList(cur, buffer.paragraphs(),
							   buffer.params().documentClassPtr(),
							   buffer.params().authors(),
							   errorList);
					return true;
				}
			}
		}
	}

	// Then try plain text
	docstring const text = theClipboard().getAsText(Clipboard::PlainTextType);
	if (text.empty())
		return false;
	cur.recordUndo();
	if (asParagraphs)
		cur.text()->insertStringAsParagraphs(cur, text, cur.current_font);
	else
		cur.text()->insertStringAsLines(cur, text, cur.current_font);
	cur.forceBufferUpdate();
	return true;
}


void pasteSimpleText(Cursor & cur, bool asParagraphs)
{
	docstring text;
	// Use internal clipboard if it is the most recent one
	if (theClipboard().isInternal()) {
		if (!checkPastePossible(0))
			return;

		ParagraphList const & pars = theCuts[0].first;
		ParagraphList::const_iterator it = pars.begin();
		for (; it != pars.end(); ++it) {
			if (it != pars.begin())
				text += "\n";
			text += (*it).asString();
		}
		asParagraphs = false;
	} else {
		// Then try plain text
		text = theClipboard().getAsText(Clipboard::PlainTextType);
	}

	if (text.empty())
		return;

	cur.recordUndo();
	cutSelection(cur, false);
	if (asParagraphs)
		cur.text()->insertStringAsParagraphs(cur, text, cur.current_font);
	else
		cur.text()->insertStringAsLines(cur, text, cur.current_font);
}


void pasteClipboardGraphics(Cursor & cur, ErrorList & /* errorList */,
			    Clipboard::GraphicsType preferedType)
{
	LASSERT(theClipboard().hasGraphicsContents(preferedType), return);

	// get picture from clipboard
	FileName filename = theClipboard().getAsGraphics(cur, preferedType);
	if (filename.empty())
		return;

	// create inset for graphic
	InsetGraphics * inset = new InsetGraphics(cur.buffer());
	InsetGraphicsParams params;
	params.filename = support::DocFileName(filename.absFileName(), false);
	inset->setParams(params);
	cur.recordUndo();
	cur.insert(inset);
}


void pasteSelection(Cursor & cur, ErrorList & errorList)
{
	if (selectionBuffer.empty())
		return;
	cur.recordUndo();
	pasteParagraphList(cur, selectionBuffer[0].first,
			selectionBuffer[0].second.first,
			selectionBuffer[0].second.second,
			errorList);
}


void replaceSelectionWithString(Cursor & cur, docstring const & str)
{
	cur.recordUndo();
	DocIterator selbeg = cur.selectionBegin();

	// Get font setting before we cut, we need a copy here, not a bare reference.
	Font const font =
		selbeg.paragraph().getFontSettings(cur.buffer()->params(), selbeg.pos());

	// Insert the new string
	pos_type pos = cur.selEnd().pos();
	Paragraph & par = cur.selEnd().paragraph();
	for (auto const & c : str) {
		par.insertChar(pos, c, font, cur.buffer()->params().track_changes);
		++pos;
	}

	// Cut the selection
	cutSelection(cur, false);
}


void replaceSelection(Cursor & cur)
{
	if (cur.selection())
		cutSelection(cur, false);
}


void eraseSelection(Cursor & cur)
{
	//lyxerr << "cap::eraseSelection begin: " << cur << endl;
	CursorSlice const & i1 = cur.selBegin();
	CursorSlice const & i2 = cur.selEnd();
	if (!i1.asInsetMath()) {
		LYXERR0("Can't erase this selection");
		return;
	}

	saveSelection(cur);
	cur.top() = i1;
	InsetMath * p = i1.asInsetMath();
	if (i1.idx() == i2.idx()) {
		i1.cell().erase(i1.pos(), i2.pos());
		// We may have deleted i1.cell(cur.pos()).
		// Make sure that pos is valid.
		if (cur.pos() > cur.lastpos())
			cur.pos() = cur.lastpos();
	} else if (p->nrows() > 0 && p->ncols() > 0) {
		// This is a grid, delete a nice square region
		row_type r1, r2;
		col_type c1, c2;
		region(i1, i2, r1, r2, c1, c2);
		for (row_type row = r1; row <= r2; ++row)
			for (col_type col = c1; col <= c2; ++col)
				p->cell(p->index(row, col)).clear();
		// We've deleted the whole cell. Only pos 0 is valid.
		cur.pos() = 0;
	} else {
		idx_type idx1 = i1.idx();
		idx_type idx2 = i2.idx();
		if (idx1 > idx2)
			swap(idx1, idx2);
		for (idx_type idx = idx1 ; idx <= idx2; ++idx)
			p->cell(idx).clear();
		// We've deleted the whole cell. Only pos 0 is valid.
		cur.pos() = 0;
	}

	// need a valid cursor. (Lgb)
	cur.clearSelection();
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
		cur.selection(false);
}


docstring grabSelection(CursorData const & cur)
{
	if (!cur.selection())
		return docstring();

#if 0
	// grab selection by glueing multiple cells together. This is not what
	// we want because selections spanning multiple cells will get "&" and "\\"
	// seperators.
	ostringstream os;
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
			Buffer * buf = cur.buffer();
			return asString(MathData(buf, it + i1.pos(), it + i2.pos()));
		} else {
			return from_ascii("unknown selection 1");
		}
	}

	row_type r1, r2;
	col_type c1, c2;
	region(i1, i2, r1, r2, c1, c2);

	docstring data;
	if (i1.inset().asInsetMath()) {
		for (row_type row = r1; row <= r2; ++row) {
			if (row > r1)
				data += "\\\\";
			for (col_type col = c1; col <= c2; ++col) {
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
