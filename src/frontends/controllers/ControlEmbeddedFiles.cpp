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

#include "frontends/LyXView.h"
#include "support/convert.h"

using std::string;

namespace lyx {
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
	string const tmp = convert<string>(item.parID());
	kernel().lyxview().dispatch(FuncRequest(LFUN_PARAGRAPH_GOTO, tmp));
}


void ControlEmbeddedFiles::view(EmbeddedFile const & item)
{
	formats.view(kernel().buffer(), item, formats.getFormatFromFile(item));
}

} // namespace frontend
} // namespace lyx
