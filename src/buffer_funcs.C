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
#include "errorlist.h"
#include "gettext.h"
#include "LaTeX.h"
#include "paragraph.h"
#include "lyxvc.h"
#include "texrow.h"
#include "vc-backend.h"

#include "frontends/Alert.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>

using lyx::support::bformat;
using lyx::support::FileInfo;
using lyx::support::IsFileWriteable;
using lyx::support::LibFileSearch;
using lyx::support::MakeDisplayPath;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::unlink;

using std::string;


extern BufferList bufferlist;

namespace {

bool readFile(Buffer * b, string const & s)
{
	// File information about normal file
	FileInfo fileN(s);
	if (!fileN.exist()) {
		string const file = MakeDisplayPath(s, 50);
		string text = bformat(_("The specified document\n%1$s"
					"\ncould not be read."), file);
		Alert::error(_("Could not read document"), text);
		return false;
	}

	// Check if emergency save file exists and is newer.
	string const e = OnlyPath(s) + OnlyFilename(s) + ".emergency";
	FileInfo fileE(e);

	if (fileE.exist() && fileN.exist()
	    && fileE.getModificationTime() > fileN.getModificationTime())
	{
		string const file = MakeDisplayPath(s, 20);
		string text = bformat(_("An emergency save of the document "
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
	FileInfo fileA(a);

	if (fileA.exist() && fileN.exist()
	    && fileA.getModificationTime() > fileN.getModificationTime())
	{
		string const file = MakeDisplayPath(s, 20);
		string text = bformat(_("The backup of the document "
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
	switch (IsFileWriteable(s)) {
	case 0:
		b->setReadonly(true);
		// Fall through
	case 1:
		if (readFile(b, s)) {
			b->lyxvc().file_found_hook(s);
			return true;
		}
		break;
	case -1:
		string const file = MakeDisplayPath(s, 20);
		// Here we probably should run
		if (LyXVC::file_not_found_hook(s)) {
			string text = bformat(_("Do you want to retrieve the document"
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
		break;
	}
	return false;
}


Buffer * newFile(string const & filename, string const & templatename,
		 bool isNamed)
{
	// get a free buffer
	Buffer * b = bufferlist.newBuffer(filename);

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

