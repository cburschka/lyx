/**
 * \file buffer_funcs.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 *
 */

#include <config.h>

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "DocIterator.h"
#include "Counters.h"
#include "ErrorList.h"
#include "Floating.h"
#include "FloatList.h"
#include "InsetList.h"
#include "Language.h"
#include "LaTeX.h"
#include "Layout.h"
#include "LyX.h"
#include "TextClass.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "TexRow.h"
#include "Text.h"
#include "TocBackend.h"

#include "frontends/alert.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetInclude.h"

#include "support/assert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;


Buffer * checkAndLoadLyXFile(FileName const & filename)
{
	// File already open?
	Buffer * checkBuffer = theBufferList().getBuffer(filename.absFilename());
	if (checkBuffer) {
		if (checkBuffer->isClean())
			return checkBuffer;
		docstring const file = makeDisplayPath(filename.absFilename(), 20);
		docstring text = bformat(_(
				"The document %1$s is already loaded and has unsaved changes.\n"
				"Do you want to abandon your changes and reload the version on disk?"), file);
		if (Alert::prompt(_("Reload saved document?"),
				text, 0, 1,  _("&Reload"), _("&Keep Changes")))
			return checkBuffer;

		// FIXME: should be LFUN_REVERT
		theBufferList().release(checkBuffer);
		// Load it again.
		return checkAndLoadLyXFile(filename);
	}

	if (filename.exists()) {
		if (!filename.isReadableFile()) {
			docstring text = bformat(_("The file %1$s exists but is not "
				"readable by the current user."),
				from_utf8(filename.absFilename()));
			Alert::error(_("File not readable!"), text);
			return 0;
		}
		Buffer * b = theBufferList().newBuffer(filename.absFilename());
		if (!b)
			// Buffer creation is not possible.
			return 0;
		if (!b->loadLyXFile(filename)) {
			theBufferList().release(b);
			return 0;
		}
		return b;
	}

	docstring text = bformat(_("The document %1$s does not yet "
		"exist.\n\nDo you want to create a new document?"),
		from_utf8(filename.absFilename()));
	if (!Alert::prompt(_("Create new document?"),
			text, 0, 1, _("&Create"), _("Cancel")))
		return newFile(filename.absFilename(), string(), true);

	return 0;
}


// FIXME newFile() should probably be a member method of Application...
Buffer * newFile(string const & filename, string const & templatename,
		 bool const isNamed)
{
	// get a free buffer
	Buffer * b = theBufferList().newBuffer(filename);
	if (!b)
		// Buffer creation is not possible.
		return 0;

	FileName tname;
	// use defaults.lyx as a default template if it exists.
	if (templatename.empty())
		tname = libFileSearch("templates", "defaults.lyx");
	else
		tname = makeAbsPath(templatename);

	if (!tname.empty()) {
		if (!b->readFile(tname)) {
			docstring const file = makeDisplayPath(tname.absFilename(), 50);
			docstring const text  = bformat(
				_("The specified document template\n%1$s\ncould not be read."),
				file);
			Alert::error(_("Could not read template"), text);
			theBufferList().release(b);
			return 0;
		}
	}

	if (!isNamed) {
		b->setUnnamed();
		b->setFileName(filename);
	}

	b->setReadonly(false);
	b->setFullyLoaded(true);

	return b;
}


Buffer * newUnnamedFile(string const & templatename, FileName const & path)
{
	static int newfile_number;

	string document_path = path.absFilename();
	string filename = addName(document_path,
		"newfile" + convert<string>(++newfile_number) + ".lyx");
	while (theBufferList().exists(filename)
		|| FileName(filename).isReadableFile()) {
		++newfile_number;
		filename = addName(document_path,
			"newfile" +	convert<string>(newfile_number) + ".lyx");
	}
	return newFile(filename, templatename, false);
}


int countWords(DocIterator const & from, DocIterator const & to)
{
	int count = 0;
	bool inword = false;
	for (DocIterator dit = from ; dit != to ; dit.forwardPos()) {
		// Copied and adapted from isLetter() in ControlSpellChecker
		if (dit.inTexted()
		    && dit.pos() != dit.lastpos()
		    && dit.paragraph().isLetter(dit.pos())
		    && !dit.paragraph().isDeleted(dit.pos())) {
			if (!inword) {
				++count;
				inword = true;
			}
		} else if (inword)
			inword = false;
	}

	return count;
}


int countChars(DocIterator const & from, DocIterator const & to, bool with_blanks)
{
	int chars = 0;
	int blanks = 0;
	for (DocIterator dit = from ; dit != to ; dit.forwardPos()) {

		if (!dit.inTexted()) continue;
		Paragraph const & par = dit.paragraph();
		pos_type const pos = dit.pos();

		if (pos != dit.lastpos() && !par.isDeleted(pos)) {
			if (Inset const * ins = par.getInset(pos)) {
				if (ins->isLetter())
					++chars;
				else if (with_blanks && ins->isSpace())
					++blanks;
			} else {
				char_type const c = par.getChar(pos);
				if (isPrintableNonspace(c))
					++chars;
				else if (isSpace(c) && with_blanks)
					++blanks;
			}
		}
	}

	return chars + blanks;
}


namespace {

depth_type getDepth(DocIterator const & it)
{
	depth_type depth = 0;
	for (size_t i = 0 ; i < it.depth() ; ++i)
		if (!it[i].inset().inMathed())
			depth += it[i].paragraph().getDepth() + 1;
	// remove 1 since the outer inset does not count
	return depth - 1;
}

depth_type getItemDepth(ParIterator const & it)
{
	Paragraph const & par = *it;
	LabelType const labeltype = par.layout().labeltype;

	if (labeltype != LABEL_ENUMERATE && labeltype != LABEL_ITEMIZE)
		return 0;

	// this will hold the lowest depth encountered up to now.
	depth_type min_depth = getDepth(it);
	ParIterator prev_it = it;
	while (true) {
		if (prev_it.pit())
			--prev_it.top().pit();
		else {
			// start of nested inset: go to outer par
			prev_it.pop_back();
			if (prev_it.empty()) {
				// start of document: nothing to do
				return 0;
			}
		}

		// We search for the first paragraph with same label
		// that is not more deeply nested.
		Paragraph & prev_par = *prev_it;
		depth_type const prev_depth = getDepth(prev_it);
		if (labeltype == prev_par.layout().labeltype) {
			if (prev_depth < min_depth)
				return prev_par.itemdepth + 1;
			if (prev_depth == min_depth)
				return prev_par.itemdepth;
		}
		min_depth = min(min_depth, prev_depth);
		// small optimization: if we are at depth 0, we won't
		// find anything else
		if (prev_depth == 0)
			return 0;
	}
}


bool needEnumCounterReset(ParIterator const & it)
{
	Paragraph const & par = *it;
	LASSERT(par.layout().labeltype == LABEL_ENUMERATE, /**/);
	depth_type const cur_depth = par.getDepth();
	ParIterator prev_it = it;
	while (prev_it.pit()) {
		--prev_it.top().pit();
		Paragraph const & prev_par = *prev_it;
		if (prev_par.getDepth() <= cur_depth)
			return  prev_par.layout().labeltype != LABEL_ENUMERATE;
	}
	// start of nested inset: reset
	return true;
}


// set the label of a paragraph. This includes the counters.
void setLabel(Buffer const & buf, ParIterator & it)
{
	DocumentClass const & textclass = buf.params().documentClass();
	Paragraph & par = it.paragraph();
	Layout const & layout = par.layout();
	Counters & counters = textclass.counters();

	if (par.params().startOfAppendix()) {
		// FIXME: only the counter corresponding to toplevel
		// sectionning should be reset
		counters.reset();
		counters.appendix(true);
	}
	par.params().appendix(counters.appendix());

	// Compute the item depth of the paragraph
	par.itemdepth = getItemDepth(it);

	if (layout.margintype == MARGIN_MANUAL) {
		if (par.params().labelWidthString().empty())
			par.params().labelWidthString(par.translateIfPossible(layout.labelstring(), buf.params()));
	} else {
		par.params().labelWidthString(docstring());
	}

	switch(layout.labeltype) {
	case LABEL_COUNTER:
		if (layout.toclevel <= buf.params().secnumdepth
		    && (layout.latextype != LATEX_ENVIRONMENT
			|| isFirstInSequence(it.pit(), it.plist()))) {
			counters.step(layout.counter);
			par.params().labelString(
				par.expandLabel(layout, buf.params()));
		} else
			par.params().labelString(docstring());
		break;

	case LABEL_ITEMIZE: {
		// At some point of time we should do something more
		// clever here, like:
		//   par.params().labelString(
		//    buf.params().user_defined_bullet(par.itemdepth).getText());
		// for now, use a simple hardcoded label
		docstring itemlabel;
		switch (par.itemdepth) {
		case 0:
			itemlabel = char_type(0x2022);
			break;
		case 1:
			itemlabel = char_type(0x2013);
			break;
		case 2:
			itemlabel = char_type(0x2217);
			break;
		case 3:
			itemlabel = char_type(0x2219); // or 0x00b7
			break;
		}
		par.params().labelString(itemlabel);
		break;
	}

	case LABEL_ENUMERATE: {
		// FIXME: Yes I know this is a really, really! bad solution
		// (Lgb)
		docstring enumcounter = from_ascii("enum");

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

		// Maybe we have to reset the enumeration counter.
		if (needEnumCounterReset(it))
			counters.reset(enumcounter);

		counters.step(enumcounter);

		string format;

		switch (par.itemdepth) {
		case 0:
			format = N_("\\arabic{enumi}.");
			break;
		case 1:
			format = N_("(\\alph{enumii})");
			break;
		case 2:
			format = N_("\\roman{enumiii}.");
			break;
		case 3:
			format = N_("\\Alph{enumiv}.");
			break;
		default:
			// not a valid enumdepth...
			break;
		}

		par.params().labelString(counters.counterLabel(
			par.translateIfPossible(from_ascii(format), buf.params())));

		break;
	}

	case LABEL_SENSITIVE: {
		string const & type = counters.current_float();
		docstring full_label;
		if (type.empty())
			full_label = buf.B_("Senseless!!! ");
		else {
			docstring name = buf.B_(textclass.floats().getType(type).name());
			if (counters.hasCounter(from_utf8(type))) {
				counters.step(from_utf8(type));
				full_label = bformat(from_ascii("%1$s %2$s:"), 
						     name, 
						     counters.theCounter(from_utf8(type)));
			} else
				full_label = bformat(from_ascii("%1$s #:"), name);	
		}
		par.params().labelString(full_label);	
		break;
	}

	case LABEL_NO_LABEL:
		par.params().labelString(docstring());
		break;

	case LABEL_MANUAL:
	case LABEL_TOP_ENVIRONMENT:
	case LABEL_CENTERED_TOP_ENVIRONMENT:
	case LABEL_STATIC:	
	case LABEL_BIBLIO:
		par.params().labelString(
			par.translateIfPossible(layout.labelstring(), 
						buf.params()));
		break;
	}
}

} // anon namespace

void updateLabels(Buffer const & buf, ParIterator & parit)
{
	LASSERT(parit.pit() == 0, /**/);

	// set the position of the text in the buffer to be able
	// to resolve macros in it. This has nothing to do with
	// labels, but by putting it here we avoid implementing
	// a whole bunch of traversal routines just for this call.
	parit.text()->setMacrocontextPosition(parit);

	depth_type maxdepth = 0;
	pit_type const lastpit = parit.lastpit();
	for ( ; parit.pit() <= lastpit ; ++parit.pit()) {
		// reduce depth if necessary
		parit->params().depth(min(parit->params().depth(), maxdepth));
		maxdepth = parit->getMaxDepthAfter();

		// set the counter for this paragraph
		setLabel(buf, parit);

		// Now the insets
		InsetList::const_iterator iit = parit->insetList().begin();
		InsetList::const_iterator end = parit->insetList().end();
		for (; iit != end; ++iit) {
			parit.pos() = iit->pos;
			iit->inset->updateLabels(parit);
		}
	}
}


// FIXME: buf should should be const because updateLabels() modifies
// the contents of the paragraphs.
void updateLabels(Buffer const & buf, bool childonly)
{
	Buffer const * const master = buf.masterBuffer();
	// Use the master text class also for child documents
	DocumentClass const & textclass = master->params().documentClass();

	if (!childonly) {
		// If this is a child document start with the master
		if (master != &buf) {
			updateLabels(*master);
			return;
		}

		// start over the counters
		textclass.counters().reset();
		buf.clearReferenceCache();
		buf.updateMacros();
	}

	Buffer & cbuf = const_cast<Buffer &>(buf);

	if (buf.text().empty()) {
		// FIXME: we don't call continue with updateLabels()
		// here because it crashes on newly created documents.
		// But the TocBackend needs to be initialised
		// nonetheless so we update the tocBackend manually.
		cbuf.tocBackend().update();
		return;
	}

	// do the real work
	ParIterator parit = par_iterator_begin(buf.inset());
	updateLabels(buf, parit);

	cbuf.tocBackend().update();
	if (!childonly)
		cbuf.structureChanged();
}


} // namespace lyx
