/**
 * \file paragraphdlgimpl.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef PARAGRAPHDLGIMPL_H
#define PARAGRAPHDLGIMPL_H

#include <config.h>
 
#include "paragraphdlg.h"
#include "vspace.h"
#include "lyxparagraph.h"


class FormParagraph;

class ParagraphDlgImpl : public ParagraphDlg
{
	Q_OBJECT
	  
public:
	
	ParagraphDlgImpl(FormParagraph *form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~ParagraphDlgImpl();
	
	void setReadOnly(bool);
	void setLabelWidth(const char *);
	void setAlign(int);
	void setChecks(bool, bool, bool, bool, bool);
	void setSpace(VSpace::vspace_kind, VSpace::vspace_kind, bool, bool);
	void setAboveLength(float, float, float, LyXGlueLength::UNIT, LyXGlueLength::UNIT, LyXGlueLength::UNIT);
	void setBelowLength(float, float, float, LyXGlueLength::UNIT, LyXGlueLength::UNIT, LyXGlueLength::UNIT);
	void setExtra(float, LyXGlueLength::UNIT, const string, int, bool, bool, LyXParagraph::PEXTRA_TYPE);
	LyXGlueLength getAboveLength() const;
	LyXGlueLength getBelowLength() const;
	LyXLength getExtraWidth() const;
	string getExtraWidthPercent() const;
	// FIXME: return a std::string ! 
	const char * getLabelWidth() const;
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
	LyXParagraph::PEXTRA_TYPE getExtraType() const;
	LyXParagraph::MINIPAGE_ALIGNMENT getExtraAlign() const;
	bool getHfillBetween() const;
	bool getStartNewMinipage() const;
	
	
protected:
	
	void closeEvent (QCloseEvent * e);
	
private:
	
	FormParagraph * form_;
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

#endif // PARAGRAPHDLGIMPL_H
