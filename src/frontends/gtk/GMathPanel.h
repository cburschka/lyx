// -*- C++ -*-
/**
 * \file GMathPanel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GMATH_PANEL_H
#define GMATH_PANEL_H

#include "GViewBase.h"
#include "support/lstrings.h"
#include "GXpmBtnTbl.h"

class ControlMath;

class GMathPanel : public GViewCB<ControlMath, GViewGladeB>
{
public:
	GMathPanel(Dialog & parent);
private:
	virtual void apply() {}
	virtual void update() {}
	virtual void doBuild();
	void onShowDialog(char const * dialogName);
	void onTableUpClicked(int row, int col);
	void onTableDownClicked(int row, int col);
	void onSuperClicked();
	void onSubClicked();
	void onEquationClicked();
	void onInsert(char const * what);
	void onFunctionSelected();
	Gtk::TreeView * functions_;
	Gtk::TreeModelColumn<Glib::ustring> listCol_;
	Gtk::TreeModel::ColumnRecord listCols_;
	Glib::RefPtr<Gtk::ListStore> listStore_;
	Glib::RefPtr<Gtk::TreeSelection> listSel_;
	GXpmBtnTbl tableUp_;
	GXpmBtnTbl tableDown_;
};

#endif
