/**
 * \file Dialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Dialog.h"

#include "GuiView.h"

#include "Buffer.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyXFunc.h"

#include <string>

namespace lyx {
namespace frontend {


Dialog::Dialog(GuiView & lv, std::string const & name)
	: lyxview_(&lv), name_(name)
{}


Dialog::~Dialog()
{}


std::string const & Dialog::name() const
{
	return name_;
}

bool Dialog::canApply() const
{
	FuncRequest const fr(getLfun(), name_);
	FuncStatus const fs(getStatus(fr));
	return fs.enabled();
}


void Dialog::dispatch(FuncRequest const & fr) const
{
	theLyXFunc().setLyXView(lyxview_);
	lyx::dispatch(fr);
}


void Dialog::updateDialog() const
{
	dispatch(FuncRequest(LFUN_DIALOG_UPDATE, name_));
}


void Dialog::disconnect() const
{
	lyxview_->disconnectDialog(name_);
}


bool Dialog::isBufferAvailable() const
{
	return lyxview_->buffer() != 0;
}


bool Dialog::isBufferReadonly() const
{
	if (!lyxview_->buffer())
		return true;
	return lyxview_->buffer()->isReadonly();
}


std::string const Dialog::bufferFilepath() const
{
	return buffer().filePath();
}


KernelDocType Dialog::docType() const
{
	if (buffer().isLatex())
		return LATEX;
	if (buffer().isLiterate())
		return LITERATE;

	return DOCBOOK;
}


BufferView * Dialog::bufferview()
{
	return lyxview_->view();
}


BufferView const * Dialog::bufferview() const
{
	return lyxview_->view();
}


Buffer & Dialog::buffer()
{
	BOOST_ASSERT(lyxview_->buffer());
	return *lyxview_->buffer();
}


Buffer const & Dialog::buffer() const
{
	BOOST_ASSERT(lyxview_->buffer());
	return *lyxview_->buffer();
}

} // namespace frontend
} // namespace lyx
