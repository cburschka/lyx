/**
 * \file buffer_funcs.C
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
#include "buffer.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "dociterator.h"
#include "counters.h"
#include "errorlist.h"
#include "Floating.h"
#include "FloatList.h"
#include "gettext.h"
#include "language.h"
#include "LaTeX.h"
#include "lyxtextclass.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "pariterator.h"
#include "lyxvc.h"
#include "texrow.h"
#include "TocBackend.h"
#include "vc-backend.h"

#include "frontends/Alert.h"

#include "insets/insetbibitem.h"
#include "insets/insetinclude.h"

#include "support/filetools.h"
#include "support/fs_extras.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>


namespace lyx {

using namespace std;

using support::bformat;
using support::FileName;
using support::libFileSearch;
using support::makeDisplayPath;
using support::onlyFilename;
using support::onlyPath;
using support::unlink;

using std::min;
using std::string;

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
			return b->readFile(e.absFilename());
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
			return b->readFile(a.absFilename());
		case 1:
			// Here we delete the autosave
			unlink(a);
			break;
		default:
			return false;
		}
	}
	return b->readFile(s.absFilename());
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

// FIXME newFile() should probably be a member method of Application...
Buffer * newFile(string const & filename, string const & templatename,
		 bool const isNamed)
{
	// get a free buffer
	Buffer * b = theBufferList().newBuffer(filename);
	BOOST_ASSERT(b);

	string tname;
	// use defaults.lyx as a default template if it exists.
	if (templatename.empty())
		tname = libFileSearch("templates", "defaults.lyx").absFilename();
	else
		tname = templatename;

	if (!tname.empty()) {
		if (!b->readFile(tname)) {
			docstring const file = makeDisplayPath(tname, 50);
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
	b->updateDocLang(b->params().language);

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


// set the label of a paragraph. This includes the counters.
void setLabel(Buffer const & buf, ParIterator & it, LyXTextClass const & textclass)
{
	Paragraph & par = *it;
	LyXLayout_ptr const & layout = par.layout();
	Counters & counters = textclass.counters();

	if (it.pit() == 0) {
		par.params().appendix(par.params().startOfAppendix());
	} else {
		par.params().appendix(it.plist()[it.pit() - 1].params().appendix());
		if (!par.params().appendix() &&
		    par.params().startOfAppendix()) {
			par.params().appendix(true);
			textclass.counters().reset();
		}
	}

	// Compute the item depth of the paragraph
	par.itemdepth = getItemDepth(it);

	// erase what was there before
	par.params().labelString(docstring());

	if (layout->margintype == MARGIN_MANUAL) {
		if (par.params().labelWidthString().empty())
			par.setLabelWidthString(buf.translateLabel(layout->labelstring()));
	} else {
		par.setLabelWidthString(docstring());
	}

	// is it a layout that has an automatic label?
	if (layout->labeltype == LABEL_COUNTER) {
		if (layout->toclevel <= buf.params().secnumdepth
		    && (layout->latextype != LATEX_ENVIRONMENT
			|| isFirstInSequence(it.pit(), it.plist()))) {
			counters.step(layout->counter);
			docstring label = expandLabel(buf, layout,
						      par.params().appendix());
			par.params().labelString(label);
		}
	} else if (layout->labeltype == LABEL_ITEMIZE) {
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
			itemlabel += char_type(0x2219); // or 0x00b7
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

		par.params().labelString(counters.counterLabel(buf.B_(format)));
	} else if (layout->labeltype == LABEL_BIBLIO) {// ale970302
		counters.step(from_ascii("bibitem"));
		int number = counters.value(from_ascii("bibitem"));
		if (par.bibitem())
			par.bibitem()->setCounter(number);
		par.params().labelString(buf.translateLabel(layout->labelstring()));
		// In biblio should't be following counters but...
	} else if (layout->labeltype == LABEL_SENSITIVE) {
		// Search for the first float or wrap inset in the iterator
		docstring type;
		size_t i = it.depth();
		while (i > 0) {
			--i;
			InsetBase * const in = &it[i].inset();
			if (in->lyxCode() == InsetBase::FLOAT_CODE
			    || in->lyxCode() == InsetBase::WRAP_CODE) {
				type = in->getInsetName();
				break;
			}
		}

		docstring s;
		if (!type.empty()) {
			Floating const & fl = textclass.floats().getType(to_ascii(type));
			// FIXME UNICODE
			counters.step(from_ascii(fl.type()));

			// Doesn't work... yet.
			s = bformat(_("%1$s #:"), buf.B_(fl.name()));
		} else {
			// par->SetLayout(0);
			s = buf.translateLabel(layout->labelstring());
		}

		par.params().labelString(s);
	} else if (layout->labeltype == LABEL_NO_LABEL)
		par.params().labelString(docstring());
	else
		par.params().labelString(buf.translateLabel(layout->labelstring()));
}

} // anon namespace


bool updateCurrentLabel(Buffer const & buf,
	ParIterator & it)
{
    if (it == par_iterator_end(buf.inset()))
	return false;

//	if (it.lastpit == 0 && LyXText::isMainText(buf))
//		return false;

	switch (it->layout()->labeltype) {

	case LABEL_NO_LABEL:
	case LABEL_MANUAL:
	case LABEL_BIBLIO:
	case LABEL_TOP_ENVIRONMENT:
	case LABEL_CENTERED_TOP_ENVIRONMENT:
	case LABEL_STATIC:
	case LABEL_ITEMIZE:
		setLabel(buf, it, buf.params().getLyXTextClass());
		return true;

	case LABEL_SENSITIVE:
	case LABEL_COUNTER:
	// do more things with enumerate later
	case LABEL_ENUMERATE:
		return false;
	}

	// This is dead code which get rid of a warning:
	return true;
}


void updateLabels(Buffer const & buf,
	ParIterator & from, ParIterator & to, bool childonly)
{
	for (ParIterator it = from; it != to; ++it) {
		if (it.pit() > it.lastpit())
			return;
		if (!updateCurrentLabel (buf, it)) {
			updateLabels(buf, childonly);
			return;
		}
	}
}


void updateLabels(Buffer const & buf, ParIterator & iter, bool childonly)
{
	if (updateCurrentLabel(buf, iter))
		return;

	updateLabels(buf, childonly);
}


void updateLabels(Buffer const & buf, bool childonly)
{
	// Use the master text class also for child documents
	LyXTextClass const & textclass = buf.params().getLyXTextClass();

	if (!childonly) {
		// If this is a child document start with the master
		Buffer const * const master = buf.getMasterBuffer();
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

		// Now included docs
		InsetList::const_iterator iit = it->insetlist.begin();
		InsetList::const_iterator end = it->insetlist.end();
		for (; iit != end; ++iit) {
			if (iit->inset->lyxCode() == InsetBase::INCLUDE_CODE)
				static_cast<InsetInclude const *>(iit->inset)
					->updateLabels(buf);
		}
	}

	const_cast<Buffer &>(buf).tocBackend().update();
}


docstring expandLabel(Buffer const & buf,
		      LyXLayout_ptr const & layout, bool appendix)
{
	LyXTextClass const & tclass = buf.params().getLyXTextClass();

	docstring fmt = buf.translateLabel(appendix ?
			layout->labelstring_appendix() :
			layout->labelstring());

	// handle 'inherited level parts' in 'fmt',
	// i.e. the stuff between '@' in   '@Section@.\arabic{subsection}'
	size_t const i = fmt.find('@', 0);
	if (i != docstring::npos) {
		size_t const j = fmt.find('@', i + 1);
		if (j != docstring::npos) {
			docstring parent(fmt, i + 1, j - i - 1);
			// FIXME UNICODE
			docstring label = expandLabel(buf, tclass[to_utf8(parent)], appendix);
			fmt = docstring(fmt, 0, i) + label + docstring(fmt, j + 1, docstring::npos);
		}
	}

	return tclass.counters().counterLabel(fmt);
}


} // namespace lyx
