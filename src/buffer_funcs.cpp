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
#include "gettext.h"
#include "Language.h"
#include "LaTeX.h"
#include "TextClass.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "LyXVC.h"
#include "TexRow.h"
#include "TocBackend.h"
#include "VCBackend.h"

#include "frontends/alert.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetCaption.h"
#include "insets/InsetInclude.h"
#include "insets/InsetTabular.h"

#include "support/filetools.h"
#include "support/fs_extras.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#include "support/textutils.h"
#include "support/filetools.h"
using std::min;
using std::string;


namespace lyx {

using namespace std;

using support::bformat;
using support::FileName;
using support::doesFileExist;
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
	if (!doesFileExist(s)) {
		docstring const file = makeDisplayPath(s.absFilename(), 50);
		docstring text = bformat(_("The specified document\n%1$s"
						     "\ncould not be read."), file);
		Alert::error(_("Could not read document"), text);
		return false;
	}

	// Check if emergency save file exists and is newer.
	FileName const e(s.absFilename() + ".emergency");

	if (doesFileExist(e) && doesFileExist(s) &&
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

	if (doesFileExist(a) && doesFileExist(s) &&
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


Buffer * checkAndLoadLyXFile(FileName const & filename)
{
	// File already open?
	if (theBufferList().exists(filename.absFilename())) {
		docstring const file = makeDisplayPath(filename.absFilename(), 20);
		docstring text = bformat(_("The document %1$s is already "
						     "loaded.\n\nDo you want to revert "
						     "to the saved version?"), file);
		if (Alert::prompt(_("Revert to saved document?"),
				text, 0, 1,  _("&Revert"), _("&Switch to document")))
			return theBufferList().getBuffer(filename.absFilename());

		// FIXME: should be LFUN_REVERT
		if (theBufferList().close(theBufferList().getBuffer(filename.absFilename()), false))
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


int countChars(DocIterator const & from, DocIterator const & to, bool with_blanks)
{
	int chars = 0;
	int blanks = 0;
	for (DocIterator dit = from ; dit != to ; dit.forwardPos()) {
		if (dit.inTexted()
		    && dit.pos() != dit.lastpos()
		    && !dit.paragraph().isDeleted(dit.pos())) {
			if (dit.paragraph().isInset(dit.pos())) {
				if (dit.paragraph().getInset(dit.pos())->isLetter())
					++chars;
				else if (dit.paragraph().getInset(dit.pos())->isSpace() && with_blanks)
					++blanks;
			} else {
				char_type const c = dit.paragraph().getChar(dit.pos());
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
	LYX_LABEL_TYPES const labeltype = par.layout()->labeltype;

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


void setCaptionLabels(Inset & inset, string const & type,
		docstring const label, Counters & counters)
{
	Text * text = inset.getText(0);
	if (!text)
		return;

	ParagraphList & pars = text->paragraphs();
	if (pars.empty())
		return;

	docstring const counter = from_ascii(type);

	ParagraphList::iterator p = pars.begin();
	for (; p != pars.end(); ++p) {
		InsetList::iterator it2 = p->insetlist.begin();
		InsetList::iterator end2 = p->insetlist.end();
		// Any caption within this float should have the same
		// label prefix but different numbers.
		for (; it2 != end2; ++it2) {
			Inset & icap = *it2->inset;
			// Look deeper just in case.
			setCaptionLabels(icap, type, label, counters);
			if (icap.lyxCode() == Inset::CAPTION_CODE) {
				// We found a caption!
				counters.step(counter);
				int number = counters.value(counter);
				InsetCaption & ic = static_cast<InsetCaption &>(icap);
				ic.setType(type);
				ic.setCount(number);
				ic.setCustomLabel(label);
			}
		}
	}
}


void setCaptions(Paragraph & par, TextClass const & textclass)
{
	if (par.insetlist.empty())
		return;

	Counters & counters = textclass.counters();

	InsetList::iterator it = par.insetlist.begin();
	InsetList::iterator end = par.insetlist.end();
	for (; it != end; ++it) {
		Inset & inset = *it->inset;
		if (inset.lyxCode() == Inset::FLOAT_CODE
			|| inset.lyxCode() == Inset::WRAP_CODE) {
			docstring const name = inset.name();
			if (name.empty())
				continue;

			Floating const & fl = textclass.floats().getType(to_ascii(name));
			// FIXME UNICODE
			string const & type = fl.type();
			docstring const label = from_utf8(fl.name());
			setCaptionLabels(inset, type, label, counters);
		}
		else if (inset.lyxCode() == Inset::TABULAR_CODE
			&&  static_cast<InsetTabular &>(inset).tabular.isLongTabular()) {
			// FIXME: are "table" and "Table" the correct type and label?
			setCaptionLabels(inset, "table", from_ascii("Table"), counters);
		}
		else if (inset.lyxCode() == Inset::LISTINGS_CODE)
			setCaptionLabels(inset, "listing", from_ascii("Listing"), counters);
		else if (inset.lyxCode() == Inset::INCLUDE_CODE)
			// if this include inset contains lstinputlisting, and has a caption
			// it will increase the 'listing' counter by one
			static_cast<InsetInclude &>(inset).updateCounter(counters);
	}
}

// set the label of a paragraph. This includes the counters.
void setLabel(Buffer const & buf, ParIterator & it, TextClass const & textclass)
{
	Paragraph & par = *it;
	Layout_ptr const & layout = par.layout();
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

	// Optimisation: setLabel() can be called for each for each
	// paragraph of the document. So we make the string static to
	// avoid the repeated instanciation.
	static docstring itemlabel;

	// is it a layout that has an automatic label?
	if (layout->labeltype == LABEL_COUNTER) {
		if (layout->toclevel <= buf.params().secnumdepth
		    && (layout->latextype != LATEX_ENVIRONMENT
			|| isFirstInSequence(it.pit(), it.plist()))) {
			counters.step(layout->counter);
			par.params().labelString(
				par.expandLabel(layout, buf.params()));
		} else
			par.params().labelString(docstring());

	} else if (layout->labeltype == LABEL_ITEMIZE) {
		// At some point of time we should do something more
		// clever here, like:
		//   par.params().labelString(
		//    buf.params().user_defined_bullet(par.itemdepth).getText());
		// for now, use a simple hardcoded label
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

	} else if (layout->labeltype == LABEL_ENUMERATE) {
		// FIXME
		// Yes I know this is a really, really! bad solution
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

	} else if (layout->labeltype == LABEL_BIBLIO) {// ale970302
		counters.step(from_ascii("bibitem"));
		int number = counters.value(from_ascii("bibitem"));
		if (par.bibitem())
			par.bibitem()->setCounter(number);

		par.params().labelString(
			par.translateIfPossible(layout->labelstring(), buf.params()));
		// In biblio shouldn't be following counters but...
	} else if (layout->labeltype == LABEL_SENSITIVE) {
		// Search for the first float or wrap inset in the iterator
		size_t i = it.depth();
		Inset * in = 0;
		while (i > 0) {
			--i;
			Inset::Code const code = it[i].inset().lyxCode();
			if (code == Inset::FLOAT_CODE ||
			    code == Inset::WRAP_CODE) {
				in = &it[i].inset();
				break;
			}
		}
		// FIXME Can Inset::name() return an empty name for wide or
		// float insets? If not we can put the definition of type
		// inside the if (in) clause and use that instead of
		// if (!type.empty()).
		docstring type;
		if (in)
			type = in->name();

		if (!type.empty()) {
			Floating const & fl = textclass.floats().getType(to_ascii(type));
			// FIXME UNICODE
			counters.step(from_ascii(fl.type()));

			// Doesn't work... yet.
			par.params().labelString(par.translateIfPossible(
				bformat(from_ascii("%1$s #:"), from_utf8(fl.name())),
				buf.params()));
		} else {
			// par->SetLayout(0);
			par.params().labelString(par.translateIfPossible(
				layout->labelstring(), buf.params()));
		}

	} else if (layout->labeltype == LABEL_NO_LABEL)
		par.params().labelString(docstring());
	else
		par.params().labelString(
			par.translateIfPossible(layout->labelstring(), buf.params()));
}

} // anon namespace


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

	ParIterator const end = par_iterator_end(buf.inset());

	for (ParIterator it = par_iterator_begin(buf.inset()); it != end; ++it) {
		// reduce depth if necessary
		if (it.pit()) {
			Paragraph const & prevpar = it.plist()[it.pit() - 1];
			it->params().depth(min(it->params().depth(),
					       prevpar.getMaxDepthAfter()));
		} else
			it->params().depth(0);

		// set the counter for this paragraph
		setLabel(buf, it, textclass);

		// It is better to set the captions after setLabel because
		// the caption number might need the section number in the
		// future.
		setCaptions(*it, textclass);

		// Now included docs
		InsetList::const_iterator iit = it->insetlist.begin();
		InsetList::const_iterator end = it->insetlist.end();
		for (; iit != end; ++iit) {
			if (iit->inset->lyxCode() == Inset::INCLUDE_CODE)
				static_cast<InsetInclude const *>(iit->inset)
					->updateLabels(buf);
		}
	}

	// The Toc backend of child documents will be done later in InsetInclude::addToToc()
	if (master == &buf) {
		Buffer & cbuf = const_cast<Buffer &>(buf);
		cbuf.tocBackend().update();
		if (!childonly)
			cbuf.structureChanged();
	}
}


void checkBufferStructure(Buffer & buffer, ParIterator const & par_it)
{
	if (par_it->layout()->toclevel != Layout::NOT_IN_TOC) {
		Buffer * master = buffer.getMasterBuffer();
		master->tocBackend().updateItem(par_it);
		master->structureChanged();
	}
}

} // namespace lyx
