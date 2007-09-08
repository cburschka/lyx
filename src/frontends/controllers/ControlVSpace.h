// -*- C++ -*-
/**
 * \file ControlVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLVSPACE_H
#define CONTROLVSPACE_H

#include "Dialog.h"
#include "VSpace.h"

namespace lyx {
namespace frontend {

class ControlVSpace : public Controller {
public:
	///
	ControlVSpace(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	VSpace & params();
	///
	VSpace const & params() const;
	///
	bool inInset() const;

private:
	///
	VSpace params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLPARAGRAPH_H
