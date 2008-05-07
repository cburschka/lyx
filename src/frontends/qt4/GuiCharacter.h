// -*- C++ -*-
/**
 * \file GuiCharacter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICHARACTER_H
#define GUICHARACTER_H

#include "GuiDialog.h"
#include "ui_CharacterUi.h"
#include "qt_helpers.h" // for LanguagePair
#include "Font.h"

#include <utility>


namespace lyx {
namespace frontend {

enum FontState {
	///
	IGNORE,
	///
	EMPH_TOGGLE,
	///
	UNDERBAR_TOGGLE,
	///
	NOUN_TOGGLE,
	///
	INHERIT
};

typedef std::pair<QString, FontFamily> FamilyPair;
typedef std::pair<QString, FontFamily> FamilyPair;
typedef std::pair<QString, FontSeries> SeriesPair;
typedef std::pair<QString, FontShape>  ShapePair;
typedef std::pair<QString, FontSize>   SizePair;
typedef std::pair<QString, FontState>  BarPair;
typedef std::pair<QString, ColorCode>  ColorPair;

class GuiCharacter : public GuiDialog, public Ui::CharacterUi
{
	Q_OBJECT

public:
	GuiCharacter(GuiView & lv);

protected Q_SLOTS:
	void change_adaptor();

private:
	/// Dialog inherited methods
	//@{
	void applyView();
	void updateContents();
	bool initialiseParams(std::string const & data);
	void clearParams() {}
	void dispatchParams();
	bool isBufferDependent() const { return true; }
	FuncCode getLfun() const { return LFUN_FONT_FREE_UPDATE; }
	//@}

	///
	void paramsToDialog(Font const & font);

	QList<FamilyPair> family;
	QList<SeriesPair> series;
	QList<ShapePair>  shape;
	QList<SizePair>   size;
	QList<BarPair>    bar;
	QList<ColorPair>  color;
	QList<LanguagePair> language;

	///
	///
	Font font_;
	///
	bool toggleall_;
	/// If true the language should be reset.
	/// If false the language of font_ is used.
	bool reset_lang_;
};

} // namespace frontend
} // namespace lyx

#endif // GUICHARACTER_H
