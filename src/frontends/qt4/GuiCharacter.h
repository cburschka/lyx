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
#include "frontend_helpers.h" // for LanguagePair
#include "Font.h"

#include <vector>


namespace lyx {

class Color_color;

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

typedef std::pair<QString, Font::FONT_FAMILY> FamilyPair;
typedef std::pair<QString, Font::FONT_SERIES> SeriesPair;
typedef std::pair<QString, Font::FONT_SHAPE>  ShapePair;
typedef std::pair<QString, Font::FONT_SIZE>   SizePair;
typedef std::pair<QString, FontState> BarPair;
typedef std::pair<QString, Color_color> ColorPair;

class GuiCharacter : public GuiDialog, public Ui::CharacterUi
{
	Q_OBJECT

public:
	GuiCharacter(LyXView & lv);

protected Q_SLOTS:
	void change_adaptor();

private:
	void closeEvent(QCloseEvent * e);
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

	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams() {}
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	///
	kb_action getLfun() const { return LFUN_FONT_FREE_UPDATE; }

	///
	void setFamily(Font::FONT_FAMILY);
	///
	void setSeries(Font::FONT_SERIES);
	///
	void setShape(Font::FONT_SHAPE);
	///
	void setSize(Font::FONT_SIZE);
	///
	void setBar(FontState);
	///
	void setColor(Color_color);
	///
	void setLanguage(std::string const &);

	///
	Font::FONT_FAMILY getFamily() const;
	///
	Font::FONT_SERIES getSeries() const;
	///
	Font::FONT_SHAPE getShape() const;
	///
	Font::FONT_SIZE getSize() const;
	///
	FontState getBar() const;
	///
	Color_color getColor() const;
	///
	std::string getLanguage() const;

private:
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
