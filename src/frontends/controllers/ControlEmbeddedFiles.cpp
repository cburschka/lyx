/**
 * \file ControlEmbeddedFiles.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlEmbeddedFiles.h"

#include "Buffer.h"

#include "FuncRequest.h"
#include "gettext.h"
#include "debug.h"
#include "Format.h"
#include "LyXRC.h"

#include "frontend_helpers.h"
#include "frontends/LyXView.h"

#include "support/FileFilterList.h"
#include "support/convert.h"

using std::string;

namespace lyx {

using support::FileFilterList;

namespace frontend {

ControlEmbeddedFiles::ControlEmbeddedFiles(Dialog & parent)
	: Dialog::Controller(parent), embedded_files(NULL)
{}


bool ControlEmbeddedFiles::initialiseParams(string const &)
{
	return true;
}


void ControlEmbeddedFiles::updateEmbeddedFiles()
{
	// copy buffer embeddedFiles to a local copy
	kernel().buffer().embeddedFiles().update();
	embedded_files = &kernel().buffer().embeddedFiles();
}


void ControlEmbeddedFiles::dispatchParams()
{
	// lyx view will only be updated if we do something to the main window. :-)
	kernel().dispatch(FuncRequest(LFUN_MESSAGE, message_));
}


void ControlEmbeddedFiles::goTo(EmbeddedFile const & item)
{
	int id = item.parID();
	if (id != 0) {
		string const tmp = convert<string>(item.parID());
		kernel().lyxview().dispatch(FuncRequest(LFUN_PARAGRAPH_GOTO, tmp));
	}
}


void ControlEmbeddedFiles::view(EmbeddedFile const & item)
{
	formats.view(kernel().buffer(), item, formats.getFormatFromFile(item));
}


docstring const ControlEmbeddedFiles::browseFile()
{
	std::pair<docstring, docstring> dir1(_("Documents|#o#O"),
				  lyx::from_utf8(lyxrc.document_path));
	FileFilterList const filter(_("All file (*.*)"));
	return browseRelFile(docstring(), lyx::from_utf8(kernel().bufferFilepath()),
			     _("Select a file to embed"),
			     filter, false, dir1);
}


bool ControlEmbeddedFiles::extract(EmbeddedFile const & item)
{
	return item.extract(&kernel().buffer());
}


} // namespace frontend
} // namespace lyx
