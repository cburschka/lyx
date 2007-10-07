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

#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyXFunc.h"

#include "frontends/LyXView.h"
#include "frontends/Dialogs.h" // FIXME

#include "Buffer.h"


namespace lyx {
namespace frontend {


Dialog::~Dialog()
{}


Controller::Controller(Dialog & parent)
	: parent_(parent), lyxview_(0)
{}


Controller::Controller(Dialog * parent)
	: parent_(*parent), lyxview_(0)
{}


Controller::~Controller()
{}


bool Controller::canApply() const
{
	FuncRequest const fr(getLfun(), dialog().name());
	FuncStatus const fs(getStatus(fr));
	return fs.enabled();
}


void Controller::dispatch(FuncRequest const & fr) const
{
	lyxview_->dispatch(fr);
}


void Controller::updateDialog(std::string const & name) const
{
	dispatch(FuncRequest(LFUN_DIALOG_UPDATE, name));
}


void Controller::disconnect(std::string const & name) const
{
	lyxview_->getDialogs().disconnect(name);
}


bool Controller::isBufferAvailable() const
{
	return lyxview_->buffer() != 0;
}


bool Controller::isBufferReadonly() const
{
	if (!lyxview_->buffer())
		return true;
	return lyxview_->buffer()->isReadonly();
}


std::string const Controller::bufferFilepath() const
{
	return buffer().filePath();
}


KernelDocType Controller::docType() const
{
	if (buffer().isLatex())
		return LATEX;
	if (buffer().isLiterate())
		return LITERATE;

	return DOCBOOK;
}


BufferView * Controller::bufferview()
{
	return lyxview_->view();
}


BufferView const * Controller::bufferview() const
{
	return lyxview_->view();
}


Buffer & Controller::buffer()
{
	BOOST_ASSERT(lyxview_->buffer());
	return *lyxview_->buffer();
}


Buffer const & Controller::buffer() const
{
	BOOST_ASSERT(lyxview_->buffer());
	return *lyxview_->buffer();
}

} // namespace frontend
} // namespace lyx
