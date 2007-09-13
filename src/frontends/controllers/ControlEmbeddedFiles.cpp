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
	: Controller(parent)
{}


EmbeddedFiles & ControlEmbeddedFiles::embeddedFiles()
{
	return buffer().embeddedFiles();
}


bool ControlEmbeddedFiles::initialiseParams(string const &)
{
	return true;
}


void ControlEmbeddedFiles::updateEmbeddedFiles()
{
	// copy buffer embeddedFiles to a local copy
	buffer().embeddedFiles().update();
	buffer().embeddingChanged();
}


void ControlEmbeddedFiles::dispatchMessage(string const & msg)
{
	// FIXME: the right thing to do? QT guys?
	// lyx view will only be updated if we do something to the main window. :-)
	dispatch(FuncRequest(LFUN_MESSAGE, msg));
}


void ControlEmbeddedFiles::goTo(EmbeddedFile const & item, int idx)
{
	BOOST_ASSERT(idx < item.refCount());
	item.saveBookmark(&buffer(), idx);
	lyxview().dispatch(FuncRequest(LFUN_BOOKMARK_GOTO, "0"));
}


void ControlEmbeddedFiles::view(EmbeddedFile const & item)
{
	formats.view(buffer(), item, formats.getFormatFromFile(item));
}


void ControlEmbeddedFiles::setEmbed(EmbeddedFile & item, bool embed, bool update)
{
	if (item.embedded() == embed)
		return;
	item.setEmbed(embed);
	if (update) {
		if (embed)
			item.updateFromExternalFile(&buffer());
		else
			item.extract(&buffer());
		item.updateInsets(&buffer());
	}
	if (embed)
		dispatchMessage("Embed file " + item.outputFilename(buffer().filePath()));
	else
		dispatchMessage("Stop embedding file " + item.outputFilename(buffer().filePath()));
	buffer().markDirty();
}


docstring const ControlEmbeddedFiles::browseFile()
{
	std::pair<docstring, docstring> dir1(_("Documents|#o#O"),
				  from_utf8(lyxrc.document_path));
	FileFilterList const filter(_("All file (*.*)"));
	return browseRelFile(docstring(), from_utf8(bufferFilepath()),
			     _("Select a file to embed"),
			     filter, false, dir1);
}


bool ControlEmbeddedFiles::extract(EmbeddedFile const & item)
{
	if (item.embedded())
		return item.extract(&buffer());
	else
		return false;
}


bool ControlEmbeddedFiles::update(EmbeddedFile const & item)
{
	if (item.embedded())
		return item.updateFromExternalFile(&buffer());
	else
		return false;
}

} // namespace frontend
} // namespace lyx
