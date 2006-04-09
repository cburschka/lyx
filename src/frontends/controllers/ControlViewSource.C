/**
 * \file ControlViewSource.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlViewSource.h"
#include "gettext.h"

using std::string;

namespace lyx {
namespace frontend {

ControlViewSource::ControlViewSource(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlViewSource::initialiseParams(string const & source)
{
	string sourcetype = source.substr(1, 5);
	if (sourcetype == "LaTeX") {
		type_ = LatexSource;
		source_ = source.substr(7);
	} else if (sourcetype == "Linux") {
		type_ = LinuxDocSource;
		source_ = source.substr(10);
	} else if (sourcetype == "DocBo") {
		type_ = DocBookSource;
		source_ = source.substr(9);
	} else
		return false;
	
	return true;
}


void ControlViewSource::clearParams()
{
	source_.erase();
}


string const ControlViewSource::title() const
{
	switch (type_) {
	case LatexSource:
		return _("LaTeX Source");
	case LinuxDocSource:
		return _("LinuxDoc Source");
	case DocBookSource:
		return _("DocBook Source");
	}
}

} // namespace frontend
} // namespace lyx
