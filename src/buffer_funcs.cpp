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
#include "debug.h"
#include "DocIterator.h"
#include "Counters.h"
#include "ErrorList.h"
#include "Floating.h"
#include "FloatList.h"
#include "gettext.h"
#include "InsetIterator.h"
#include "Language.h"
#include "LaTeX.h"
#include "Layout.h"
#include "LyX.h"
#include "lyxlayout_ptr_fwd.h"
#include "TextClass.h"
#include "TextClassList.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "LyXVC.h"
#include "TexRow.h"
#include "Text.h"
#include "TocBackend.h"
#include "VCBackend.h"

#include "frontends/alert.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetInclude.h"

#include "support/filetools.h"
#include "support/fs_extras.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

using std::min;
using std::string;


namespace lyx {

using namespace std;

using support::bformat;
using support::FileName;
using support::libFileSearch;
using support::makeAbsPath;
using support::makeDisplayPath;
using support::onlyFilename;
using support::onlyPath;
using support::unlink;

namespace Alert = frontend::Alert;
namespace fs = boost::filesystem;

namespace {

bool readFile(Buffer * const b, FileName const & s)
{
	BOOST_ASSERT(b);

	// File information about normal file
	if (!fs::exists(s.toFilesystemEncoding())) {
		docstring const file = makeDisplayPath(s.absFilename(), 50);
		docstring text = bformat(_("The specified document\n%1$s"
						     "\ncould not be read."), file);
		Alert::error(_("Could not read document"), text);
		return false;
	}

	// Check if emergency save file exists and is newer.
	FileName const e(s.absFilename() + ".emergency");

	if (fs::exists(e.toFilesystemEncoding()) &&
	    fs::exists(s.toFilesystemEncoding()) &&
	    fs::last_write_time(e.toFilesystemEncoding()) > fs::last_write_time(s.toFilesystemEncoding()))
	{
		docstring const file = makeDisplayPath(s.absFilename(), 20);
		docstring const text =
			bformat(_("An emergency save of the document "
				  "%1$s exists.\n\n"
					       "Recover emergency save?"), file);
		switch (Alert::prompt(_("Load emergency save?"), text, 0, 2,
				      _("&Recover"),  _("&Load Original"),
				      _("&Cancel")))
		{
		case 0:
			// the file is not saved if we load the emergency file.
			b->markDirty();
			return b->readFile(e);
		case 1:
			break;
		default:
			return false;
		}
	}

	// Now check if autosave file is newer.
	FileName const a(onlyPath(s.absFilename()) + '#' + onlyFilename(s.absFilename()) + '#');

	if (fs::exists(a.toFilesystemEncoding()) &&
	    fs::exists(s.toFilesystemEncoding()) &&
	    fs::last_write_time(a.toFilesystemEncoding()) > fs::last_write_time(s.toFilesystemEncoding()))
	{
		docstring const file = makeDisplayPath(s.absFilename(), 20);
		docstring const text =
			bformat(_("The backup of the document "
				  "%1$s is newer.\n\nLoad the "
					       "backup instead?"), file);
		switch (Alert::prompt(_("Load backup?"), text, 0, 2,
				      _("&Load backup"), _("Load &original"),
				      _("&Cancel") ))
		{
		case 0:
			// the file is not saved if we load the autosave file.
			b->markDirty();
			return b->readFile(a);
		case 1:
			// Here we delete the autosave
			unlink(a);
			break;
		default:
			return false;
		}
	}
	return b->readFile(s);
}


} // namespace anon



bool loadLyXFile(Buffer * b, FileName const & s)
{
	BOOST_ASSERT(b);

	if (fs::is_readable(s.toFilesystemEncoding())) {
		if (readFile(b, s)) {
			b->lyxvc().file_found_hook(s);
			if (!fs::is_writable(s.toFilesystemEncoding()))
				b->setReadonly(true);
			return true;
		}
	} else {
		docstring const file = makeDisplayPath(s.absFilename(), 20);
		// Here we probably should run
		if (LyXVC::file_not_found_hook(s)) {
			docstring const text =
				bformat(_("Do you want to retrieve the document"
						       " %1$s from version control?"), file);
			int const ret = Alert::prompt(_("Retrieve from version control?"),
				text, 0, 1, _("&Retrieve"), _("&Cancel"));

			if (ret == 0) {
				// How can we know _how_ to do the checkout?
				// With the current VC support it has to be,
				// a RCS file since CVS do not have special ,v files.
				RCS::retrieve(s);
				return loadLyXFile(b, s);
			}
		}
	}
	return false;
}


bool checkIfLoaded(FileName const & fn)
{
	return theBufferList().getBuffer(fn.absFilename());
}


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
		if (theBufferList().close(checkBuffer, false))
			// Load it again.
			return checkAndLoadLyXFile(filename);
		else
			// The file could not be closed.
			return 0;
	}

	if (isFileReadable(filename)) {
		Buffer * b = theBufferList().newBuffer(filename.absFilename());
		if (!lyx::loadLyXFile(b, filename)) {
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
	BOOST_ASSERT(b);

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
	b->fully_loaded(true);

	return b;
}


void bufferErrors(Buffer const & buf, TeXErrors const & terr,
				  ErrorList & errorList)
{
	TeXErrors::Errors::const_iterator cit = terr.begin();
	TeXErrors::Errors::const_iterator end = terr.end();

	for (; cit != end; ++cit) {
		int id_start = -1;
		int pos_start = -1;
		int errorrow = cit->error_in_line;
		bool found = buf.texrow().getIdFromRow(errorrow, id_start,
						       pos_start);
		int id_end = -1;
		int pos_end = -1;
		do {
			++errorrow;
			found = buf.texrow().getIdFromRow(errorrow, id_end,
							  pos_end);
		} while (found && id_start == id_end && pos_start == pos_end);

		errorList.push_back(ErrorItem(cit->error_desc,
			cit->error_text, id_start, pos_start, pos_end));
	}
}


string const bufferFormat(Buffer const & buffer)
{
	if (buffer.isDocBook())
		return "docbook";
	else if (buffer.isLiterate())
		return "literate";
	else
		return "latex";
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
	LabelType const labeltype = par.layout()->labeltype;

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
		if (labeltype == prev_par.layout()->labeltype) {
			if (prev_depth < min_depth) {
				return prev_par.itemdepth + 1;
			}
			else if (prev_depth == min_depth) {
				return prev_par.itemdepth;
			}
		}
		min_depth = std::min(min_depth, prev_depth);
		// small optimization: if we are at depth 0, we won't
		// find anything else
		if (prev_depth == 0) {
			return 0;
		}
	}
}


bool needEnumCounterReset(ParIterator const & it)
{
	Paragraph const & par = *it;
	BOOST_ASSERT(par.layout()->labeltype == LABEL_ENUMERATE);
	depth_type const cur_depth = par.getDepth();
	ParIterator prev_it = it;
	while (prev_it.pit()) {
		--prev_it.top().pit();
		Paragraph const & prev_par = *prev_it;
		if (prev_par.getDepth() <= cur_depth)
			return  prev_par.layout()->labeltype != LABEL_ENUMERATE;
	}
	// start of nested inset: reset
	return true;
}


// set the label of a paragraph. This includes the counters.
void setLabel(Buffer const & buf, ParIterator & it)
{
	TextClass const & textclass = buf.params().getTextClass();
	Paragraph & par = it.paragraph();
	LayoutPtr const & layout = par.layout();
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

	if (layout->margintype == MARGIN_MANUAL) {
		if (par.params().labelWidthString().empty())
			par.params().labelWidthString(par.translateIfPossible(layout->labelstring(), buf.params()));
	} else {
		par.params().labelWidthString(docstring());
	}

	switch(layout->labeltype) {
	case LABEL_COUNTER:
		if (layout->toclevel <= buf.params().secnumdepth
		    && (layout->latextype != LATEX_ENVIRONMENT
			|| isFirstInSequence(it.pit(), it.plist()))) {
			counters.step(layout->counter);
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
			par.translateIfPossible(layout->labelstring(), 
						buf.params()));
		break;
	}
}

} // anon namespace

void updateLabels(Buffer const & buf, ParIterator & parit)
{
	BOOST_ASSERT(parit.pit() == 0);

	depth_type maxdepth = 0;
	pit_type const lastpit = parit.lastpit();
	for ( ; parit.pit() <= lastpit ; ++parit.pit()) {
		// reduce depth if necessary
		parit->params().depth(min(parit->params().depth(), maxdepth));
		maxdepth = parit->getMaxDepthAfter();

		// set the counter for this paragraph
		setLabel(buf, parit);

		// Now the insets
		InsetList::const_iterator iit = parit->insetlist.begin();
		InsetList::const_iterator end = parit->insetlist.end();
		for (; iit != end; ++iit) {
			parit.pos() = iit->pos;
			iit->inset->updateLabels(buf, parit);
		}
	}
	
}


// FIXME: buf should should be const because updateLabels() modifies
// the contents of the paragraphs.
void updateLabels(Buffer const & buf, bool childonly)
{
	Buffer const * const master = buf.getMasterBuffer();
	// Use the master text class also for child documents
	TextClass const & textclass = master->params().getTextClass();

	if (!childonly) {
		// If this is a child document start with the master
		if (master != &buf) {
			updateLabels(*master);
			return;
		}

		// start over the counters
		textclass.counters().reset();
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
	// FIXME
	// the embedding signal is emitted with structureChanged signal
	// this is inaccurate so these two will be separated later.
	cbuf.embeddedFiles().update();
	cbuf.embeddingChanged();
}


void checkBufferStructure(Buffer & buffer, ParIterator const & par_it)
{
	if (par_it->layout()->toclevel != Layout::NOT_IN_TOC) {
		Buffer * master = buffer.getMasterBuffer();
		master->tocBackend().updateItem(par_it);
		master->structureChanged();
	}
}

textclass_type defaultTextclass()
{
	// We want to return the article class. if `first' is
	// true in the returned pair, then `second' is the textclass
	// number; if it is false, second is 0. In both cases, second
	// is what we want.
	return textclasslist.numberOfClass("article").second;
}


void loadChildDocuments(Buffer const & buf)
{
	bool parse_error = false;
		
	for (InsetIterator it = inset_iterator_begin(buf.inset()); it; ++it) {
		if (it->lyxCode() != Inset::INCLUDE_CODE)
			continue;
		InsetInclude const & inset = static_cast<InsetInclude const &>(*it);
		InsetCommandParams const & ip = inset.params();
		Buffer * child = loadIfNeeded(buf, ip);
		if (!child)
			continue;
		parse_error |= !child->errorList("Parse").empty();
		loadChildDocuments(*child);
	}

	if (use_gui && buf.getMasterBuffer() == &buf)
		updateLabels(buf);
}
} // namespace lyx
