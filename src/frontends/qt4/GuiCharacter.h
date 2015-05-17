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
#include "Font.h"

#include <utility>

#ifdef IGNORE
#undef IGNORE
#endif

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
	STRIKEOUT_TOGGLE,
	///
	UULINE_TOGGLE,
	///
	UWAVE_TOGGLE,
	///
	INHERIT
};

typedef std::pair<QString, FontFamily> FamilyPair;
typedef std::pair<QString, FontFamily> FamilyPair;
typedef std::pair<QString, FontSeries> SeriesPair;
typedef std::pair<QString, FontShape>  ShapePair;
typedef std::pair<QString, FontSize>   SizePair;
typedef std::pair<QString, FontState>  BarPair;
typedef std::pair<QString, QString>    LanguagePair;

class GuiCharacter : public GuiDialog, public Ui::CharacterUi
{
	Q_OBJECT

public:
	GuiCharacter(GuiView & lv);

protected Q_SLOTS:
	void change_adaptor();

private:
	/// \name Dialog inherited methods
	//@{
	void applyView();
	void updateContents();
	bool initialiseParams(std::string const & data);
	void clearParams() {}
	void dispatchParams();
	bool isBufferDependent() const { return true; }
	FuncCode getLfun() const { return LFUN_TEXTSTYLE_UPDATE; }
	void saveSession() const;
	void restoreSession();
	//@}

	///
	void paramsToDialog(Font const & font);

	QList<FamilyPair> family;
	QList<SeriesPair> series;
	QList<ShapePair>  shape;
	QList<SizePair>   size;
	QList<BarPair>    bar;
	QList<ColorCode> color;
	QList<LanguagePair> language;

	///
	///
	Font font_;
	///
	bool toggleall_;
};

} // namespace frontend
} // namespace lyx

#endif // GUICHARACTER_H
