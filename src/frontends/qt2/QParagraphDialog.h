// -*- C++ -*-
/**
 * \file QParagraphDialog.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef QPARAGRAPHDIALOG_H
#define QPARAGRAPHDIALOG_H

#include <config.h>

#include "ui/QParagraphDialogBase.h"
#include "vspace.h"
#include "paragraph.h"
#include "layout.h"
#include "lyxgluelength.h"


class QParagraph;

class QParagraphDialog : public QParagraphDialogBase
{ Q_OBJECT

public:
	QParagraphDialog(QParagraph * form, QWidget * parent = 0,
			 char const * name = 0, bool modal = FALSE,
			 WFlags fl = 0);
	~QParagraphDialog();

	void setReadOnly(bool);
	void setLabelWidth(char const *);
	void setAlign(int);
	void setChecks(bool, bool, bool, bool, bool);
	void setSpace(VSpace::vspace_kind, VSpace::vspace_kind, bool, bool);
	void setAboveLength(float, float, float,
			    LyXLength::UNIT, LyXLength::UNIT, LyXLength::UNIT);
	void setBelowLength(float, float, float,
			    LyXLength::UNIT, LyXLength::UNIT, LyXLength::UNIT);
	void setExtra(float, LyXLength::UNIT,
		      string const &, int, bool, bool, Paragraph::PEXTRA_TYPE);
	LyXGlueLength getAboveLength() const;
	LyXGlueLength getBelowLength() const;
	LyXLength getExtraWidth() const;
	string getExtraWidthPercent() const;
	// FIXME: return a std::string !
	char const * getLabelWidth() const;
	LyXAlignment getAlign() const;
	bool getAboveKeep() const;
	bool getBelowKeep() const;
	bool getLineAbove() const;
	bool getLineBelow() const;
	bool getPagebreakAbove() const;
	bool getPagebreakBelow() const;
	bool getNoIndent() const;
	VSpace::vspace_kind getSpaceAboveKind() const;
	VSpace::vspace_kind getSpaceBelowKind() const;
	Paragraph::PEXTRA_TYPE getExtraType() const;
	Paragraph::MINIPAGE_ALIGNMENT getExtraAlign() const;
	bool getHfillBetween() const;
	bool getStartNewMinipage() const;
protected:
	void closeEvent (QCloseEvent * e);
private:
	QParagraph * form_;
	VSpace::vspace_kind getSpaceKind(int val) const;
	LyXLength::UNIT getLyXLength(int val) const;
	int getItem(LyXLength::UNIT unit) const;
protected slots:
	void apply_adaptor();
	void cancel_adaptor();
	void enable_extraOptions(int);
	void enable_minipageOptions(int);
	void enable_spacingAbove(int);
	void enable_spacingBelow(int);
	void ok_adaptor();
	void restore_adaptor();
};

#endif // QPARAGRAPHDIALOG_H
