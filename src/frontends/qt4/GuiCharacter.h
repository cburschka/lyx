// -*- C++ -*-
/**
 * \file GuiCharacter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICHARACTER_H
#define GUICHARACTER_H

#include "GuiDialogView.h"
#include "ControlCharacter.h"
#include "ui_CharacterUi.h"

#include <QDialog>

#include <vector>

namespace lyx {
namespace frontend {

class GuiCharacter;

class GuiCharacterDialog : public QDialog, public Ui::CharacterUi {
	Q_OBJECT
public:
	GuiCharacterDialog(GuiCharacter * form);
protected:
	void closeEvent(QCloseEvent * e);
private:
	GuiCharacter * form_;
protected Q_SLOTS:
	void change_adaptor();
};


class GuiCharacter : public GuiView<GuiCharacterDialog>
{
public:
	friend class GuiCharacterDialog;

	GuiCharacter(Dialog &);
	/// parent controller
	ControlCharacter & controller()
	{ return static_cast<ControlCharacter &>(this->getController()); }
	/// parent controller
	ControlCharacter const & controller() const
	{ return static_cast<ControlCharacter const &>(this->getController()); }
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	std::vector<FamilyPair> family;
	std::vector<SeriesPair> series;
	std::vector<ShapePair>  shape;
	std::vector<SizePair>   size;
	std::vector<BarPair>    bar;
	std::vector<ColorPair>  color;
	std::vector<LanguagePair> language;
};

} // namespace frontend
} // namespace lyx

#endif // GUICHARACTER_H
