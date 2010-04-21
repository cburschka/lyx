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
		if (!Alert::prompt(_("Reload saved document?"),
			  text, 0, 1,  _("&Reload"), _("&Keep Changes"))) {
			// reload the document
			if (!checkBuffer->reload())
				return 0;
		}
		return checkBuffer;
	}

	if (filename.exists()) {
		if (!filename.isReadableFile()) {
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
		if (!b->loadLyXFile(filename)) {
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
			docstring const file = makeDisplayPath(tname.absFileName(), 50);
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
	} else
		// in this case, the user chose the filename, so we assume that she
		// really does want this file.
		b->markDirty();

	b->setReadonly(false);
	b->setFullyLoaded(true);

	return b;
}


Buffer * newUnnamedFile(FileName const & path, string const & prefix,
						string const & templatename)
{
	static map<string, int> file_number;

	FileName filename;

	do {
		filename.set(path, 
			prefix + convert<string>(++file_number[prefix]) + ".lyx");
	}
	while (theBufferList().exists(filename) || filename.isReadableFile());
		
	return newFile(filename.absFileName(), templatename, false);
}


/* 
 * FIXME : merge with countChars. The structures of the two functions
 * are similar but, unfortunately, they seem to have a different
 * notion of what to count. Since nobody ever complained about that,
 * this proves (again) that any number beats no number ! (JMarc)
 */
int countWords(DocIterator const & from, DocIterator const & to)
{
	int count = 0;
	bool inword = false;
	for (DocIterator dit = from ; dit != to ; ) {
		if (!dit.inTexted()) {
			dit.forwardPos();
			continue;
		}
		
		Paragraph const & par = dit.paragraph();
		pos_type const pos = dit.pos();
		
		// Copied and adapted from isWordSeparator() in Paragraph
		if (pos != dit.lastpos() && !par.isDeleted(pos)) {
			Inset const * ins = par.getInset(pos);
			if (ins && !ins->producesOutput()) {
				//skip this inset
				++dit.top().pos();
				continue;
			}
			if (par.isWordSeparator(pos)) 
				inword = false;
			else if (!inword) {
				++count;
				inword = true;
			}
		}
		dit.forwardPos();
	}

	return count;
}


int countChars(DocIterator const & from, DocIterator const & to, 
	       bool with_blanks)
{
	int chars = 0;
	int blanks = 0;
	for (DocIterator dit = from ; dit != to ; ) {
		if (!dit.inTexted()) {
			dit.forwardPos();
			continue;
		}
		
		Paragraph const & par = dit.paragraph();
		pos_type const pos = dit.pos();
		
		if (pos != dit.lastpos() && !par.isDeleted(pos)) {
			if (Inset const * ins = par.getInset(pos)) {
				if (!ins->producesOutput()) {
					//skip this inset
					++dit.top().pos();
					continue;
				}
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
		dit.forwardPos();
	}

	return chars + blanks;
}


Buffer * loadIfNeeded(FileName const & fname)
{
	Buffer * buffer = theBufferList().getBuffer(fname);
	if (!buffer) {
		if (!fname.exists())
			return 0;

		buffer = theBufferList().newBuffer(fname.absFileName());
		if (!buffer)
			// Buffer creation is not possible.
			return 0;

		if (!buffer->loadLyXFile(fname)) {
			//close the buffer we just opened
			theBufferList().release(buffer);
			return 0;
		}
	}
	return buffer;
}


} // namespace lyx
