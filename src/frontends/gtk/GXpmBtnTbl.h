// -*- C++ -*-
/**
 * \file GXpmBtnTbl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XPM_BTN_TBL_H
#define XPM_BTN_TBL_H

#include <gtkmm.h>

#include <boost/scoped_array.hpp>

class GXpmBtnTbl : public Gtk::Table {
public:
	typedef char const ** XpmData;
	typedef sigc::signal<void, int, int> SigType;
	struct XbmData {
		unsigned char const* data_;
		int width_;
		int height_;
		GdkColor fg_;
	};

	class GXpmBtn : public Gtk::Button {
	public:
		GXpmBtn() : row_(-1), col_(-1)
		{
			signal_clicked().connect(sigc::mem_fun(*this,
					   &GXpmBtn::onButtonClicked));
		}
		void setRow(int row) { row_ = row; }
		void setCol(int col) { col_ = col; }
		int getRow() { return row_; }
		int getCol() { return col_; }
		void setXpm(XpmData xpm);
		void setXpm(Glib::RefPtr<Gdk::Pixmap> pixmap,
			    Glib::RefPtr<Gdk::Bitmap> mask);
		Glib::RefPtr<Gdk::Pixmap> getPixmap() { return pixmap_; }
		Glib::RefPtr<Gdk::Bitmap> getMask() { return mask_; }
		SigType signalClicked() { return signalClicked_; }

		void onButtonClicked()
		{
			signalClicked_.emit(row_, col_);
		}
	private:
		int row_;
		int col_;
		Glib::RefPtr<Gdk::Pixmap> pixmap_;
		Glib::RefPtr<Gdk::Bitmap> mask_;
		SigType signalClicked_;
	};

	GXpmBtnTbl(int rows, int cols, XpmData xpms[]);
	//GXpmBtnTbl(int rows, int cols, XpmData xpm);
	GXpmBtnTbl(int rows, int cols, XbmData const & xbm);
	~GXpmBtnTbl();

	GXpmBtn * getBtn(int row, int col)
	{
		return &btns_[index(row, col)];
	}

	SigType signalClicked() { return signalClicked_; }
private:
	int index(int row, int col) { return row * cols_ + col; }
	void on_realize();
	void construct();
	void setBtnXpm(XpmData xpms[]);
	void setBtnXpm(XbmData const & xbm);

	int rows_;
	int cols_;
	boost::scoped_array<GXpmBtn> btns_;
	XbmData const * xbm_;
	SigType signalClicked_;
};


void buttonSetXpm(Gtk::Button * btn, char const ** xpm);


#endif
