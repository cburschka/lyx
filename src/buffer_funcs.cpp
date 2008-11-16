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


Buffer * checkAndLoadLyXFile(FileName const & filename)
{
	// File already open?
	Buffer * checkBuffer = theBufferList().getBuffer(filename);
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
		if (!b) {
			// Buffer creation is not possible.
			return 0;
		}
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

	FileName filename(path, 
		"newfile" + convert<string>(++newfile_number) + ".lyx");
	while (theBufferList().exists(filename)
		|| filename.isReadableFile()) {
		++newfile_number;
		filename.set(path,
			"newfile" +	convert<string>(newfile_number) + ".lyx");
	}
	return newFile(filename.absFilename(), templatename, false);
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

} // namespace lyx
