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
#include "LyXVC.h"
#include "TextClass.h"
#include "Paragraph.h"
#include "ParagraphList.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "TexRow.h"
#include "Text.h"
#include "TocBackend.h"

#include "frontends/alert.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetInclude.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/mutex.h"
#include "support/textutils.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;


Buffer * checkAndLoadLyXFile(FileName const & filename, bool const acceptDirty)
{
	// File already open?
	Buffer * checkBuffer = theBufferList().getBuffer(filename);
	if (checkBuffer) {
		// Sometimes (when setting the master buffer from a child)
		// we accept a dirty buffer right away (otherwise we'd get
		// an infinite loop (bug 5514).
		// We also accept a dirty buffer when the document has not
		// yet been saved to disk.
		if (checkBuffer->isClean() || acceptDirty || !filename.exists())
			return checkBuffer;
		docstring const file = makeDisplayPath(filename.absFileName(), 20);
		docstring const text = bformat(_(
				"The document %1$s is already loaded and has unsaved changes.\n"
				"Do you want to abandon your changes and reload the version on disk?"), file);
		int res = Alert::prompt(_("Reload saved document?"),
					text, 2, 2,  _("Yes, &Reload"), _("No, &Keep Changes"), _("&Cancel"));
		switch (res) {
			case 0: {
				// reload the document
				if (checkBuffer->reload() != Buffer::ReadSuccess)
					return 0;
				return checkBuffer;
			}
			case 1:
				// keep changes
				return checkBuffer;
			case 2:
				// cancel
				return 0;
		}
	}

	bool const exists = filename.exists();
	bool const tryVC = exists ? false : LyXVC::fileInVC(filename);
	if (exists || tryVC) {
		if (exists && !filename.isReadableFile()) {
			docstring text = bformat(_("The file %1$s exists but is not "
				"readable by the current user."),
				from_utf8(filename.absFileName()));
			Alert::error(_("File not readable!"), text);
			return 0;
		}
		Buffer * b = theBufferList().newBuffer(filename.absFileName());
		if (!b) {
			// Buffer creation is not possible.
			return 0;
		}
		if (b->loadLyXFile() != Buffer::ReadSuccess) {
			// do not save an emergency file when releasing the buffer
			b->markClean();
			theBufferList().release(b);
			return 0;
		}
		return b;
	}

	docstring text = bformat(_("The document %1$s does not yet "
		"exist.\n\nDo you want to create a new document?"),
		from_utf8(filename.absFileName()));
	if (!Alert::prompt(_("Create new document?"),
			text, 0, 1, _("&Create"), _("Cancel")))
		return newFile(filename.absFileName(), string(), true);

	return 0;
}


// FIXME newFile() should probably be a member method of Application...
Buffer * newFile(string const & filename, string const & templatename,
		 bool is_named)
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
		if (b->loadThisLyXFile(tname) != Buffer::ReadSuccess) {
			docstring const file = makeDisplayPath(tname.absFileName(), 50);
			docstring const text  = bformat(
				_("The specified document template\n%1$s\ncould not be read."),
				file);
			Alert::error(_("Could not read template"), text);
			theBufferList().release(b);
			return 0;
		}
	}

	if (is_named)
		// in this case, the user chose the filename, so we
		// assume that she really does want this file.
		b->markDirty();
	else
		b->setUnnamed();

	b->setReadonly(false);
	b->setFullyLoaded(true);

	return b;
}


Buffer * newUnnamedFile(FileName const & path, string const & prefix,
						string const & templatename)
{
	static map<string, int> file_number;
	static Mutex mutex;

	Mutex::Locker locker(&mutex);
	FileName filename;

	do {
		filename.set(path, 
			prefix + convert<string>(++file_number[prefix]) + ".lyx");
	}
	while (theBufferList().exists(filename) || filename.isReadableFile());
		
	return newFile(filename.absFileName(), templatename, false);
}


Buffer * loadIfNeeded(FileName const & fname)
{
	Buffer * buffer = theBufferList().getBuffer(fname);
	if (!buffer) {
		if (!fname.exists() && !LyXVC::fileInVC(fname))
			return 0;

		buffer = theBufferList().newBuffer(fname.absFileName());
		if (!buffer)
			// Buffer creation is not possible.
			return 0;

		if (buffer->loadLyXFile() != Buffer::ReadSuccess) {
			//close the buffer we just opened
			theBufferList().release(buffer);
			return 0;
		}
	}
	return buffer;
}


} // namespace lyx
