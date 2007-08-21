/**
 * \file Kernel.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Kernel.h"

#include "Buffer.h"
#include "BufferView.h"
#include "FuncRequest.h"
#include "LyXFunc.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"


using std::string;

namespace lyx {
namespace frontend {

Kernel::Kernel(LyXView & lyxview)
	: lyxview_(lyxview)
{}


void Kernel::dispatch(FuncRequest const & fr) const
{
	lyxview_.dispatch(fr);
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
	return lyxview_.buffer() != 0;
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


Kernel::DocType Kernel::docType() const
{
	if (buffer().isLatex())
		return LATEX;
	if (buffer().isLiterate())
		return LITERATE;

	return DOCBOOK;
}


BufferView * Kernel::bufferview()
{
	return lyxview_.view();
}


BufferView const * Kernel::bufferview() const
{
	return lyxview_.view();
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
