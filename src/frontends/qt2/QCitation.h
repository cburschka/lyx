// -*- C++ -*-
/**
 * \file QCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QCITATION_H
#define QCITATION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class QListBox;
class ControlCitation;
class QCitationDialog;


class QCitation : public Qt2CB<ControlCitation, Qt2DB<QCitationDialog> >
{
	friend class QCitationDialog;

public:
	///
	QCitation();

private:
	///
	enum State {
		///
		ON,
		///
		OFF
	};

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

	void updateBrowser(QListBox *, std::vector<string> const &) const;
	///
	void setBibButtons(State) const;
	///
	void setCiteButtons(State) const;

	///
	std::vector<string> citekeys;
	///
	std::vector<string> bibkeys;
};

#endif // QCITATION_H
