/**
 * \file paradlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef PARADLG_H
#define PARADLG_H

#include <config.h>
#include <gettext.h>

#include  "parageneraldlg.h"
#include  "paraextradlg.h"

#include "vspace.h"
#include "lyxparagraph.h"
#include "debug.h"

#include "dlg/paradlgdata.h"

// to connect apply() and close()
#include "FormParagraph.h"

class ParaDialog : public ParaDialogData  {
	Q_OBJECT
public:
	ParaDialog(FormParagraph * form, QWidget * parent=0, char const * name=0,
			    bool modal=false, WFlags f=0);
	~ParaDialog();

	void setReadOnly(bool);
	void setLabelWidth(char const *);
	void setAlign(int);
	void setChecks(bool, bool, bool, bool, bool);
	void setSpace(VSpace::vspace_kind, VSpace::vspace_kind, bool, bool);
	void setAboveLength(float, float, float, LyXGlueLength::UNIT, LyXGlueLength::UNIT, LyXGlueLength::UNIT);
	void setBelowLength(float, float, float, LyXGlueLength::UNIT, LyXGlueLength::UNIT, LyXGlueLength::UNIT);
	void setExtra(float, LyXGlueLength::UNIT, const string, int, bool, bool, LyXParagraph::PEXTRA_TYPE);

	char const * getLabelWidth() const {
		return generalpage->labelwidth->text();
	}

	LyXAlignment getAlign() const {
		switch (generalpage->justification->currentItem()) { 
			case 0: return LYX_ALIGN_BLOCK;
			case 1: return LYX_ALIGN_CENTER;
			case 2: return LYX_ALIGN_LEFT;
			case 3: return LYX_ALIGN_RIGHT;
		}
		return LYX_ALIGN_BLOCK;
	}

	bool getAboveKeep() const {
		return generalpage->abovepage->keepabove->isChecked();
	}

	bool getBelowKeep() const {
		return generalpage->belowpage->keepbelow->isChecked();
	}

	bool getLineAbove() const {
		return generalpage->lineabove->isChecked();
	}

	bool getLineBelow() const {
		return generalpage->linebelow->isChecked();
	}

	bool getPagebreakAbove() const {
		return generalpage->abovepage->pagebreakabove->isChecked();
	}

	bool getPagebreakBelow() const {
		return generalpage->belowpage->pagebreakbelow->isChecked();
	}

	bool getNoIndent() const {
		return generalpage->noindent->isChecked();
	}

	VSpace::vspace_kind getSpaceAboveKind() const {
		return getSpaceKind(generalpage->abovepage->spaceabove->currentItem());
	}

	VSpace::vspace_kind getSpaceBelowKind() const {
		return getSpaceKind(generalpage->belowpage->spacebelow->currentItem());
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
	void closeEvent(QCloseEvent * e);

private:
	FormParagraph * form_;

	/// the general tab page
	ParaGeneralDialog * generalpage;
	/// the extra options tab page
	ParaExtraDialog * extrapage;

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
				lyxerr[Debug::GUI] << "Unknown kind combo entry " << val << std::endl;
		}
		return VSpace::NONE;
	}

private slots:

	void ok_adaptor(void) {
		apply_adaptor();
		form_->close();
		hide();
	}

	void apply_adaptor(void) {
		form_->apply();
	}

	void restore_adaptor(void) {
		// this will reset to known values
		form_->update();
	}
	
	void cancel_adaptor(void) {
		form_->close();
		hide();
	}

};

#endif // PARADLG_H
