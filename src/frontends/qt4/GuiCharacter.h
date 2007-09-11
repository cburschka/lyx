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

#include "GuiDialog.h"
#include "ControlCharacter.h"
#include "ui_CharacterUi.h"

#include <vector>

namespace lyx {
namespace frontend {

class GuiCharacterDialog : public GuiDialog, public Ui::CharacterUi
{
	Q_OBJECT

public:
	GuiCharacterDialog(LyXView & lv);

protected Q_SLOTS:
	void change_adaptor();

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlCharacter & controller();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();

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
