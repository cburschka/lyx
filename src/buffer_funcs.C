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
#include "LaTeX.h"
#include "lyxtextclass.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphList_fwd.h"
#include "ParagraphParameters.h"
#include "pariterator.h"
#include "lyxvc.h"
#include "texrow.h"
#include "vc-backend.h"

#include "frontends/Alert.h"

#include "insets/insetbibitem.h"

#include "support/filetools.h"
#include "support/fs_extras.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

using lyx::pit_type;
using lyx::support::bformat;
using lyx::support::LibFileSearch;
using lyx::support::MakeDisplayPath;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::unlink;

using std::min;
using std::string;

namespace fs = boost::filesystem;

extern BufferList bufferlist;

namespace {

bool readFile(Buffer * const b, string const & s)
{
	BOOST_ASSERT(b);

	// File information about normal file
	if (!fs::exists(s)) {
		string const file = MakeDisplayPath(s, 50);
		string text = bformat(_("The specified document\n%1$s"
					"\ncould not be read."), file);
		Alert::error(_("Could not read document"), text);
		return false;
	}

	// Check if emergency save file exists and is newer.
	string const e = OnlyPath(s) + OnlyFilename(s) + ".emergency";

	if (fs::exists(e) && fs::exists(s)
	    && fs::last_write_time(e) > fs::last_write_time(s))
	{
		string const file = MakeDisplayPath(s, 20);
		string const text =
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
	string const a = OnlyPath(s) + '#' + OnlyFilename(s) + '#';

	if (fs::exists(a) && fs::exists(s)
	    && fs::last_write_time(a) > fs::last_write_time(s))
	{
		string const file = MakeDisplayPath(s, 20);
		string const text =
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



bool loadLyXFile(Buffer * b, string const & s)
{
	BOOST_ASSERT(b);

	if (fs::is_readable(s)) {
		if (readFile(b, s)) {
			b->lyxvc().file_found_hook(s);
			if (!fs::is_writable(s))
				b->setReadonly(true);
			return true;
		}
	} else {
		string const file = MakeDisplayPath(s, 20);
		// Here we probably should run
		if (LyXVC::file_not_found_hook(s)) {
			string const text =
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


Buffer * newFile(string const & filename, string const & templatename,
		 bool const isNamed)
{
	// get a free buffer
	Buffer * b = bufferlist.newBuffer(filename);
	BOOST_ASSERT(b);

	string tname;
	// use defaults.lyx as a default template if it exists.
	if (templatename.empty())
		tname = LibFileSearch("templates", "defaults.lyx");
	else
		tname = templatename;

	if (!tname.empty()) {
		if (!b->readFile(tname)) {
			string const file = MakeDisplayPath(tname, 50);
			string const text  = bformat(_("The specified document template\n%1$s\ncould not be read."), file);
			Alert::error(_("Could not read template"), text);
			// no template, start with empty buffer
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


void bufferErrors(Buffer const & buf, TeXErrors const & terr)
{
	TeXErrors::Errors::const_iterator cit = terr.begin();
	TeXErrors::Errors::const_iterator end = terr.end();

	for (; cit != end; ++cit) {
		int par_id = -1;
		int posstart = -1;
		int const errorrow = cit->error_in_line;
		buf.texrow().getIdFromRow(errorrow, par_id, posstart);
		int posend = -1;
		buf.texrow().getIdFromRow(errorrow + 1, par_id, posend);
		buf.error(ErrorItem(cit->error_desc,
					 cit->error_text,
					 par_id, posstart, posend));
	}
}


void bufferErrors(Buffer const & buf, ErrorList const & el)
{
	for_each(el.begin(), el.end(), bind(ref(buf.error), _1));
}


string const BufferFormat(Buffer const & buffer)
{
	if (buffer.isLinuxDoc())
		return "linuxdoc";
	else if (buffer.isDocBook())
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
		    && !isDeletedText(dit.paragraph(), dit.pos())) {
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

void incrementItemDepth(ParagraphList & pars, pit_type pit, pit_type first_pit)
{
	int const cur_labeltype = pars[pit].layout()->labeltype;

	if (cur_labeltype != LABEL_ENUMERATE && cur_labeltype != LABEL_ITEMIZE)
		return;

	int const cur_depth = pars[pit].getDepth();

	pit_type prev_pit = pit - 1;
	while (true) {
		int const prev_depth = pars[prev_pit].getDepth();
		int const prev_labeltype = pars[prev_pit].layout()->labeltype;
		if (prev_depth == 0 && cur_depth > 0) {
			if (prev_labeltype == cur_labeltype) {
				pars[pit].itemdepth = pars[prev_pit].itemdepth + 1;
			}
			break;
		} else if (prev_depth < cur_depth) {
			if (prev_labeltype == cur_labeltype) {
				pars[pit].itemdepth = pars[prev_pit].itemdepth + 1;
				break;
			}
		} else if (prev_depth == cur_depth) {
			if (prev_labeltype == cur_labeltype) {
				pars[pit].itemdepth = pars[prev_pit].itemdepth;
				break;
			}
		}
		if (prev_pit == first_pit)
			break;

		--prev_pit;
	}
}


void resetEnumCounterIfNeeded(ParagraphList & pars, pit_type pit,
	pit_type firstpit, Counters & counters)
{
	if (pit == firstpit)
		return;

	int const cur_depth = pars[pit].getDepth();
	pit_type prev_pit = pit - 1;
	while (true) {
		int const prev_depth = pars[prev_pit].getDepth();
		int const prev_labeltype = pars[prev_pit].layout()->labeltype;
		if (prev_depth <= cur_depth) {
			if (prev_labeltype != LABEL_ENUMERATE) {
				switch (pars[pit].itemdepth) {
				case 0:
					counters.reset("enumi");
				case 1:
					counters.reset("enumii");
				case 2:
					counters.reset("enumiii");
				case 3:
					counters.reset("enumiv");
				}
			}
			break;
		}

		if (prev_pit == firstpit)
			break;

		--prev_pit;
	}
}


// set the counter of a paragraph. This includes the labels
void setCounter(Buffer const & buf, ParIterator & it)
{
	Paragraph & par = *it;
	BufferParams const & bufparams = buf.params();
	LyXTextClass const & textclass = bufparams.getLyXTextClass();
	LyXLayout_ptr const & layout = par.layout();
	Counters & counters = textclass.counters();

	// Always reset
	par.itemdepth = 0;

	if (it.pit() == 0) {
		par.params().appendix(par.params().startOfAppendix());
	} else {
		par.params().appendix(it.plist()[it.pit() - 1].params().appendix());
		if (!par.params().appendix() &&
		    par.params().startOfAppendix()) {
			par.params().appendix(true);
			textclass.counters().reset();
		}

		// Maybe we have to increment the item depth.
		incrementItemDepth(it.plist(), it.pit(), 0);
	}

	// erase what was there before
	par.params().labelString(string());

	if (layout->margintype == MARGIN_MANUAL) {
		if (par.params().labelWidthString().empty())
			par.setLabelWidthString(layout->labelstring());
	} else {
		par.setLabelWidthString(string());
	}

	// is it a layout that has an automatic label?
	if (layout->labeltype == LABEL_COUNTER) {
		if (layout->toclevel <= buf.params().secnumdepth
		    && (!layout->isEnvironment()
			|| isFirstInSequence(it.pit(), it.plist()))) {
			counters.step(layout->counter);
			string label = expandLabel(textclass, layout,
						   par.params().appendix());
			par.params().labelString(label);
		}
	} else if (layout->labeltype == LABEL_ITEMIZE) {
		// At some point of time we should do something more
		// clever here, like:
		//   par.params().labelString(
		//    bufparams.user_defined_bullet(par.itemdepth).getText());
		// for now, use a simple hardcoded label
		string itemlabel;
		switch (par.itemdepth) {
		case 0:
			itemlabel = "*";
			break;
		case 1:
			itemlabel = "-";
			break;
		case 2:
			itemlabel = "@";
			break;
		case 3:
			itemlabel = "·";
			break;
		}

		par.params().labelString(itemlabel);
	} else if (layout->labeltype == LABEL_ENUMERATE) {
		// Maybe we have to reset the enumeration counter.
		resetEnumCounterIfNeeded(it.plist(), it.pit(), 0, counters);

		// FIXME
		// Yes I know this is a really, really! bad solution
		// (Lgb)
		string enumcounter = "enum";

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

		counters.step(enumcounter);

		par.params().labelString(counters.enumLabel(enumcounter));
	} else if (layout->labeltype == LABEL_BIBLIO) {// ale970302
		counters.step("bibitem");
		int number = counters.value("bibitem");
		if (par.bibitem()) 
			par.bibitem()->setCounter(number);
		par.params().labelString(layout->labelstring());
		// In biblio should't be following counters but...
	} else if (layout->labeltype == LABEL_SENSITIVE) {
		// Search for the first float or wrap inset in the iterator
		string type;
		size_t i = it.depth();
		while (i > 0) {
			--i;
			InsetBase * const in = &it[i].inset();
			if (in->lyxCode() == InsetBase::FLOAT_CODE
			    || in->lyxCode() == InsetBase::WRAP_CODE)
				type = in->getInsetName();
				break;
		}

		string s;
		if (!type.empty()) {
			Floating const & fl = textclass.floats().getType(type);

			counters.step(fl.type());

			// Doesn't work... yet.
			s = bformat(_("%1$s #:"), buf.B_(fl.name()));
		} else {
			// par->SetLayout(0);
			s = buf.B_(layout->labelstring());
		}

		par.params().labelString(s);
	} else
		par.params().labelString(buf.B_(layout->labelstring()));
}

} // anon namespace


void updateCounters(Buffer const & buf)
{
	// start over
	buf.params().getLyXTextClass().counters().reset();

	for (ParIterator it = par_iterator_begin(buf.inset()); it; ++it) {
		// reduce depth if necessary
		if (it.pit()) {
			Paragraph const & prevpar = it.plist()[it.pit() - 1];
			it->params().depth(min(it->params().depth(),
					       prevpar.getMaxDepthAfter()));
		} else
			it->params().depth(0);

		// set the counter for this paragraph
		setCounter(buf, it);
	}
}


string expandLabel(LyXTextClass const & textclass,
	LyXLayout_ptr const & layout, bool appendix)
{
	string fmt = appendix ?
		layout->labelstring_appendix() : layout->labelstring();

	// handle 'inherited level parts' in 'fmt',
	// i.e. the stuff between '@' in   '@Section@.\arabic{subsection}'
	size_t const i = fmt.find('@', 0);
	if (i != string::npos) {
		size_t const j = fmt.find('@', i + 1);
		if (j != string::npos) {
			string parent(fmt, i + 1, j - i - 1);
			string label = expandLabel(textclass, textclass[parent], appendix);
			fmt = string(fmt, 0, i) + label + string(fmt, j + 1, string::npos);
		}
	}

	return textclass.counters().counterLabel(fmt);
}


