/**
 * \file Kernel.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Kernel.h"

#include "buffer.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "lyxfunc.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"


using std::string;

namespace lyx {
namespace frontend {

Kernel::Kernel(LyXView & lyxview)
	: lyxview_(lyxview)
{}


void Kernel::dispatch(FuncRequest const & fr, bool verbose) const
{
	lyxview_.getLyXFunc().dispatch(fr, verbose);
}


void Kernel::updateDialog(string const & name) const
{
	dispatch(FuncRequest(LFUN_DIALOG_UPDATE, name));
}


void Kernel::disconnect(string const & name) const
{
	lyxview_.getDialogs().disconnect(name);
}

bool Kernel::isBufferAvailable() const
{
	if (!lyxview_.view().get())
                return false;
        return lyxview_.view()->available();
}


bool Kernel::isBufferReadonly() const
{
	if (!lyxview_.buffer())
                return true;
        return lyxview_.buffer()->isReadonly();
}


string const Kernel::bufferFilepath() const
{
	return buffer().filePath();
}


Kernel::DocTypes Kernel::docType() const
{
	if (buffer().isLatex())
		return LATEX;
	if (buffer().isLiterate())
		return LITERATE;
	if (buffer().isLinuxDoc())
		return LINUXDOC;

	return DOCBOOK;
}


void Kernel::redrawGUI() const
{
	lyxview_.getDialogs().redrawGUI();
}


BufferView * Kernel::bufferview()
{
	return lyxview_.view().get();
}


BufferView const * Kernel::bufferview() const
{
	return lyxview_.view().get();
}


Buffer & Kernel::buffer()
{
	BOOST_ASSERT(lyxview_.buffer());
	return *lyxview_.buffer();
}


Buffer const & Kernel::buffer() const
{
	BOOST_ASSERT(lyxview_.buffer());
	return *lyxview_.buffer();
}

} // namespace frontend
} // namespace lyx
