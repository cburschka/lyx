/*
 * paradlg.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PARADLG_H
#define PARADLG_H

#include <config.h>
#include <gettext.h>
#include <string>

#include  "parageneraldlg.h"
#include  "paraextradlg.h"
 
#include <qtabdialog.h>
 
#include "vspace.h" 
#include "lyxparagraph.h"
#include "debug.h" 

// to connect apply() and close()
#include "FormParagraph.h"

class ParaDialog : public QTabDialog  {
   Q_OBJECT
public:
	ParaDialog(FormParagraph *form, QWidget *parent=0, const char *name=0,
			    bool modal=false, WFlags f=0);
	~ParaDialog();

	void setReadOnly(bool);
	void setLabelWidth(const char *);
	void setAlign(int);
	void setChecks(bool, bool, bool, bool, bool);
	void setSpace(VSpace::vspace_kind, VSpace::vspace_kind, bool, bool);
	void setAboveLength(float, float, float, LyXGlueLength::UNIT, LyXGlueLength::UNIT, LyXGlueLength::UNIT);
	void setBelowLength(float, float, float, LyXGlueLength::UNIT, LyXGlueLength::UNIT, LyXGlueLength::UNIT);
	void setExtra(float, LyXGlueLength::UNIT, const string, int, bool, bool, LyXParagraph::PEXTRA_TYPE);

	const char *getLabelWidth() const {
		return generalpage->labelwidth->text();
	}

	LyXAlignment getAlign() const {
		if (generalpage->right->isChecked())
			return LYX_ALIGN_RIGHT;
		else if (generalpage->left->isChecked())
			return LYX_ALIGN_LEFT;
		else if (generalpage->center->isChecked())
			return LYX_ALIGN_CENTER;
		else
			return LYX_ALIGN_BLOCK;
	}

	bool getLineAbove() const {
		return generalpage->lineabove->isChecked();
	}
 
	bool getLineBelow() const {
		return generalpage->linebelow->isChecked();
	}

	bool getPagebreakAbove() const {
		return generalpage->pagebreakabove->isChecked();
	}

	bool getPagebreakBelow() const {
		return generalpage->pagebreakbelow->isChecked();
	}

	bool getNoIndent() const {
		return generalpage->noindent->isChecked();
	}

	VSpace::vspace_kind getSpaceAboveKind() const {
		return getSpaceKind(generalpage->spaceabove->currentItem());
	}
	
	VSpace::vspace_kind getSpaceBelowKind() const {
		return getSpaceKind(generalpage->spacebelow->currentItem());
	}
	
	bool getSpaceAboveKeep() const {
		// FIXME
		return true;
	}

	bool getSpaceBelowKeep() const {
		// FIXME
		return true;
	}

	LyXGlueLength getAboveLength() const;

	LyXGlueLength getBelowLength() const;

	LyXLength getExtraWidth() const;

	string getExtraWidthPercent() const;

	LyXParagraph::PEXTRA_TYPE getExtraType() const {
		switch (extrapage->type->currentItem()) {
			case 0: return LyXParagraph::PEXTRA_NONE;
			case 1: return LyXParagraph::PEXTRA_INDENT;
			case 2: return LyXParagraph::PEXTRA_MINIPAGE;
			case 3: return LyXParagraph::PEXTRA_FLOATFLT;
		}
		return LyXParagraph::PEXTRA_NONE;
	}

	LyXParagraph::MINIPAGE_ALIGNMENT getExtraAlign() const {
		if (extrapage->top->isChecked())
			return LyXParagraph::MINIPAGE_ALIGN_TOP;
		if (extrapage->middle->isChecked())
			return LyXParagraph::MINIPAGE_ALIGN_MIDDLE;
		return LyXParagraph::MINIPAGE_ALIGN_BOTTOM;
	}

	bool getHfillBetween() const {
		return extrapage->hfillbetween->isChecked();
	}

	bool getStartNewMinipage() const {
		return extrapage->startnewminipage->isChecked();
	}

protected:
	void closeEvent(QCloseEvent *e);
 
private:
	FormParagraph *form_;

	/// the general tab page
	ParaGeneralDialog *generalpage;
	/// the extra options tab page
	ParaExtraDialog *extrapage;

	VSpace::vspace_kind getSpaceKind(int val) const {
		switch (val) {
			case 0: return VSpace::NONE;
			case 1: return VSpace::DEFSKIP;
			case 2: return VSpace::SMALLSKIP;
			case 3: return VSpace::MEDSKIP;
			case 4: return VSpace::BIGSKIP;
			case 5: return VSpace::VFILL;
			case 6: return VSpace::LENGTH;
			default:
				lyxerr[Debug::GUI] << "Unknown kind combo entry " << val << endl;
		}
		return VSpace::NONE;
	}

	void setUnits(QComboBox *box, LyXGlueLength::UNIT unit);

	LyXLength::UNIT getUnits(QComboBox *box) const;

private slots:
	void apply_adaptor(void) {
		form_->apply();
		form_->close();
		hide();
	}

	void close_adaptor(void) {
		form_->close();
		hide();
	}

	void restore(void);
};

#endif
