// -*- C++ -*-
/**
 * \file GuiCommand.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICOMMAND_H
#define GUICOMMAND_H

#include "GuiDialog.h"

#include "insets/InsetCommandParams.h"


namespace lyx {
namespace frontend {

class GuiCommand : public GuiDialog
{
public:
	/// We need to know with what sort of inset we're associated.
	// FIXME This should probably be an InsetCode
	GuiCommand(GuiView &, QString const & name, QString const & title);
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams() { params_.clear(); }
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

protected:
	///
	InsetCommandParams params_;
	//FIXME It should be possible to eliminate lfun_name_
	//now and recover that information from params().insetType().
	//But let's not do that quite yet.
	/// Flags what action is taken by Kernel::dispatch()
	std::string const lfun_name_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIDIALOG_H
