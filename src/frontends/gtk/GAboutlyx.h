// -*- C++ -*-
/**
 * \file GAboutlyx.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GABOUTLYX_H
#define GABOUTLYX_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlAboutlyx;

class GAboutlyx : public GViewCB<ControlAboutlyx, GViewGladeB>
{
public:
	GAboutlyx(Dialog &);
private:
	virtual void apply() {}
	virtual void update() {}
	virtual void doBuild();
};

} // namespace frontend
} // namespace lyx

#endif
