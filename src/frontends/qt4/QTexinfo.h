// -*- C++ -*-
/**
 * \file QTexinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTEXINFO_H
#define QTEXINFO_H

#include "QDialogView.h"
#include "ControlTexinfo.h"

#include <map>
#include <vector>

namespace lyx {
namespace frontend {

class QTexinfoDialog;

///
class QTexinfo
	 : public QController<ControlTexinfo, QView<QTexinfoDialog> > {
public:
	///
	friend class QTexinfoDialog;
	///
	QTexinfo(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update (do we need this?)
	virtual void update_contents() {}
	/// build the dialog
	virtual void build_dialog();
	///
	void updateStyles(ControlTexinfo::texFileSuffix);
	///
	void updateStyles();
	///
	bool warningPosted;
	///
	ControlTexinfo::texFileSuffix activeStyle;
	///
	typedef std::vector<std::string> ContentsType;
	std::map<ControlTexinfo::texFileSuffix, ContentsType> texdata_;
};

} // namespace frontend
} // namespace lyx

#endif // QTEXINFO_H
