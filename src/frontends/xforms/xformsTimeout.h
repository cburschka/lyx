// -*- C++ -*-
/**
 * \file xformsTimeout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XFORMSTIMEOUT_H
#define XFORMSTIMEOUT_H

#include "frontends/Timeout.h"

namespace lyx {
namespace frontend {

/**
 * This class executes the callback when the timeout expires
 * using xforms mechanisms
 */
class xformsTimeout : public Timeout::Impl {
public:
	///
	xformsTimeout(Timeout &);
	///
	virtual bool running() const;
	///
	virtual void start();
	///
	virtual void stop();
	///
	virtual void reset();
	/// xforms callback function
	void emitCB();

private:
	///
	int timeout_id;
};

} // namespace frontend
} // namespace lyx

#endif // XFORMSTIMEOUT_H
