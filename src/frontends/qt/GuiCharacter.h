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

#include <QAction>

#include <utility>

#ifdef IGNORE
#undef IGNORE
#endif

namespace lyx {
namespace frontend {

enum FontDeco {
	///
	IGNORE,
	///
	UNDERBAR,
	///
	STRIKEOUT,
	///
	XOUT,
	///
	UULINE,
	///
	UWAVE,
	///
	INHERIT,
	///
	NONE
};

typedef std::pair<QString, FontFamily> FamilyPair;
typedef std::pair<QString, FontFamily> FamilyPair;
typedef std::pair<QString, FontSeries> SeriesPair;
typedef std::pair<QString, FontShape>  ShapePair;
typedef std::pair<QString, FontSize>   SizePair;
typedef std::pair<QString, FontDeco>  BarPair;
typedef std::pair<QString, QString>    LanguagePair;

class GuiCharacter : public GuiDialog, public Ui::CharacterUi
{
	Q_OBJECT

public:
	GuiCharacter(GuiView & lv);

protected Q_SLOTS:
	void change_adaptor();
	void on_emphCB_clicked();
	void on_nounCB_clicked();
	void on_nospellcheckCB_clicked();
	void resetToDefault();
	void resetToNoChange();
	void checkRestoreDefaults();

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
	void saveSession(QSettings & settings) const;
	void restoreSession();
	//@}

	///
	void paramsToDialog(Font const & font);
	///
	void setBar(FontInfo & fi, FontDeco val);
	///
	void setStrike(FontInfo & fi, FontDeco val);

	QList<FamilyPair> family;
	QList<SeriesPair> series;
	QList<ShapePair>  shape;
	QList<SizePair>   size;
	QList<BarPair>    bar;
	QList<BarPair>    strike;
	QList<ColorCode> color;
	QList<LanguagePair> language;

	///
	Font font_;
	///
	bool emph_;
	///
	bool noun_;
	///
	bool nospellcheck_;

	///
	QAction * resetdefault_ = new QAction(this);
	///
	QAction * resetnochange_ = new QAction(this);
};

} // namespace frontend
} // namespace lyx

#endif // GUICHARACTER_H
