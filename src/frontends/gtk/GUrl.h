// -*- C++ -*-
/**
 * \file GUrl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GURL_H
#define GURL_H

#include "GViewBase.h"

namespace lyx {
namespace frontend {

class ControlCommand;

class GUrl : public GViewCB<ControlCommand, GViewGladeB>
{
public:
	GUrl(Dialog & parent);
private:
	virtual void apply();
	virtual void doBuild();
	virtual void update();
	void onEntryChanged();
	Gtk::Entry * url_;
	Gtk::Entry * name_;
	Gtk::CheckButton * htmlType_;
};

} // namespace frontend
} // namespace lyx

#endif
