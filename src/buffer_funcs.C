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


extern BufferList bufferlist;

using namespace lyx::support;


namespace {

bool readFile(Buffer * b, string const & s)
{
	string ts(s);
	string e = OnlyPath(s);
	string a = e;
	// File information about normal file
	FileInfo fileInfo(s);

	if (!fileInfo.exist()) {
		string const file = MakeDisplayPath(s, 50);
		string text = bformat(_("The specified document\n%1$s"
					"\ncould not be read."), file);
		Alert::error(_("Could not read document"), text);
		return false;
	}

	// Check if emergency save file exists and is newer.
	e += OnlyFilename(s) + ".emergency";
	FileInfo fileInfoE(e);

	bool use_emergency = false;

	if (fileInfoE.exist() && fileInfo.exist()) {
		if (fileInfoE.getModificationTime()
		    > fileInfo.getModificationTime()) {
			string const file = MakeDisplayPath(s, 20);
			string text = bformat(_("An emergency save of the document %1$s exists.\n"
				"\nRecover emergency save?"), file);
			int const ret = Alert::prompt(_("Load emergency save?"),
				text, 0, 1, _("&Recover"), _("&Load Original"));

			if (ret == 0) {
				ts = e;
				// the file is not saved if we load the
				// emergency file.
				b->markDirty();
				use_emergency = true;
			}
		}
	}

	if (!use_emergency) {
		// Now check if autosave file is newer.
		a += '#';
		a += OnlyFilename(s);
		a += '#';
		FileInfo fileInfoA(a);
		if (fileInfoA.exist() && fileInfo.exist()) {
			if (fileInfoA.getModificationTime()
			    > fileInfo.getModificationTime()) {
				string const file = MakeDisplayPath(s, 20);
				string text = bformat(_("The backup of the document %1$s is newer.\n\n"
					"Load the backup instead?"), file);
				int const ret = Alert::prompt(_("Load backup?"),
					text, 0, 1, _("&Load backup"), _("Load &original"));

				if (ret == 0) {
					ts = a;
					// the file is not saved if we load the
					// autosave file.
					b->markDirty();
				} else {
					// Here, we should delete the autosave
					unlink(a);
				}
			}
		}
	}
	return b->readFile(ts);
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
			b->paragraphs().push_back(Paragraph());
			b->paragraphs().begin()->layout(b->params().getLyXTextClass().defaultLayout());
		}
	} else {  // start with empty buffer
		b->paragraphs().push_back(Paragraph());
		b->paragraphs().begin()->layout(b->params().getLyXTextClass().defaultLayout());
	}

	if (!isNamed) {
		b->setUnnamed();
		b->setFileName(filename);
	}

	b->setReadonly(false);
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
	ErrorList::const_iterator it = el.begin();
	ErrorList::const_iterator end = el.end();

	for (; it != end; ++it)
		buf.error(*it);
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
