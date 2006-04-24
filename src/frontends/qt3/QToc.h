// -*- C++ -*-
/**
 * \file QToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTOC_H
#define QTOC_H

#include "QDialogView.h"
#include "toc.h"

namespace lyx {
namespace frontend {

class ControlToc;
class QTocDialog;

class QToc :
	public QController<ControlToc, QView<QTocDialog> >
{
public:
	friend class QTocDialog;

	QToc(Dialog &);
private:
	/// update the listview
	void updateToc(int newdepth);

	///
	bool canOutline();

	/// update the float types
	void updateType();

	/// select an entry
	void select(std::string const & text);

	/// set the depth
	void set_depth(int depth);

	/// Move header up/down/in/out in list (outlining)
	void moveup();
	///
	void movedn();
	///
	void movein();
	///
	void moveout();

	virtual void apply() {}

	/// update dialog
	virtual void update_contents();

	/// build dialog
	virtual void build_dialog();

	/// the toc list
	lyx::toc::Toc toclist;

	/// depth of list shown
	int depth_;

	/// Store selected item's string
	std::string text_;

	/// Store ToC list type
	std::string type_;
};

} // namespace frontend
} // namespace lyx

#endif // QTOC_H
