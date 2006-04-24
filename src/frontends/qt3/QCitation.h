// -*- C++ -*-
/**
 * \file QCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCITATION_H
#define QCITATION_H

#include "QDialogView.h"
#include <vector>

class QListBox;

namespace lyx {
namespace frontend {

class ControlCitation;
class QCitationDialog;


class QCitation : public QController<ControlCitation, QView<QCitationDialog> >
{
public:
	friend class QCitationDialog;
	///
	QCitation(Dialog &);
protected:
	virtual bool isValid();
private:

	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build_dialog();
	/// Hide the dialog.
	virtual void hide();
	/// Update dialog before/whilst showing it.
	virtual void update_contents();

	/// fill the styles combo
	void fillStyles();

	/// set the styles combo
	void updateStyle();

	void updateBrowser(QListBox *, std::vector<std::string> const &) const;
	/// check if apply has been pressed
	bool open_find_;

	/// selected keys
	std::vector<std::string> citekeys;
	/// available bib keys
	std::vector<std::string> bibkeys;
	/// selected natbib style
	int style_;
};

} // namespace frontend
} // namespace lyx

#endif // QCITATION_H
