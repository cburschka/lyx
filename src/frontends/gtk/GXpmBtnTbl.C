/**
 * \file GXpmBtnTbl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>
#include <cassert>
#include "GXpmBtnTbl.h"


void GXpmBtnTbl::GXpmBtn::setXpm(XpmData xpm)
{
	Glib::RefPtr<Gdk::Colormap> clrmap = get_colormap();
	Gtk::Image * image;
	pixmap_ = Gdk::Pixmap::create_from_xpm(clrmap,
					       mask_,
					       xpm);
	image = SigC::manage(new Gtk::Image(pixmap_, mask_));
	image->show();
	add(*image);
}


void GXpmBtnTbl::GXpmBtn::setXpm(Glib::RefPtr<Gdk::Pixmap> pixmap,
				 Glib::RefPtr<Gdk::Bitmap> mask)
{
	pixmap_ = pixmap;
	mask_ = mask;
	Gtk::Image * image;
	image = SigC::manage(new Gtk::Image(pixmap_, mask_));
	image->show();
	add(*image);
}


GXpmBtnTbl::GXpmBtnTbl(int rows, int cols, XpmData xpms[]) :
	Gtk::Table(rows, cols, true), rows_(rows), cols_(cols),
	xbm_(0)
{
	construct();
	setBtnXpm(xpms);
}


GXpmBtnTbl::GXpmBtnTbl(int rows, int cols, const XbmData& xbm) :
	Gtk::Table(rows, cols, true), rows_(rows), cols_(cols),
	xbm_(&xbm)
{
	construct();
}


GXpmBtnTbl::~GXpmBtnTbl()
{
}


void GXpmBtnTbl::construct()
{
	assert(rows_);
	assert(cols_);
	btns_.reset(new GXpmBtn[rows_ * cols_]);
	assert(btns_.get());

	GXpmBtn * btn;
	int row, col;
	for (row = 0; row < rows_; ++row) 
		for (col = 0; col < cols_; ++col) {
			btn = &btns_[index(row, col)];
			btn->setRow(row);
			btn->setCol(col);
			btn->signalClicked().connect(signalClicked_.slot());
			btn->show();
			attach(*btn, col, col + 1,  row, row + 1);
		}
}


void GXpmBtnTbl::setBtnXpm(XpmData xpms[])
{
	int row, col;
	for (row = 0; row < rows_; ++row) 
		for (col = 0; col < cols_; ++col)
			btns_[index(row, col)].setXpm(xpms[index(row, col)]);
}


void GXpmBtnTbl::setBtnXpm(const XbmData& xbm)
{
	Glib::RefPtr<Gdk::Bitmap> mask;
	Glib::RefPtr<Gdk::Pixmap> pixmap;
	Glib::RefPtr<Gdk::Colormap> clrmap = get_colormap();
	Gdk::Color fg(const_cast<GdkColor *>(&xbm.fg_));
	clrmap->alloc_color(fg);
	Glib::RefPtr<Gdk::Window> window = get_window();
	pixmap = Gdk::Pixmap::create_from_data(
		window,
		reinterpret_cast<char const * const>(xbm.data_),
		xbm.width_,
		xbm.height_,
		window->get_depth(),
		fg,
		get_style()->get_bg(Gtk::STATE_NORMAL));
	mask = Gdk::Bitmap::create(
		window,
		reinterpret_cast<char const * const>(xbm.data_),
		xbm.width_,
		xbm.height_);
	Glib::RefPtr<Gdk::Bitmap> maskBtn;
	Glib::RefPtr<Gdk::Pixmap> pixmapBtn;
	Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(mask);
	int row, col;
	int btnWidth = xbm.width_ / cols_;
	int btnHeight = xbm.height_ / rows_;
	for (row = 0; row < rows_; ++row)
		for (col = 0; col < cols_; ++col) {
			pixmapBtn = Gdk::Pixmap::create(
				window,
				btnWidth,
				btnHeight,
				window->get_depth());
			pixmapBtn->draw_drawable(get_style()->get_black_gc(),
						 pixmap,
						 col * btnWidth,
						 row * btnHeight,
						 0,
						 0,
						 btnWidth,
						 btnHeight);
			maskBtn = Gdk::Bitmap::create(
				window,
				reinterpret_cast<char const * const>(xbm.data_),
				btnWidth,
				btnHeight);
			maskBtn->draw_drawable(gc,
					       mask,
					       col * btnWidth,
					       row * btnHeight,
					       0,
					       0,
					       btnWidth,
					       btnHeight);
			btns_[index(row, col)].setXpm(pixmapBtn, maskBtn);
		}
}


void GXpmBtnTbl::on_realize()
{
	Gtk::Table::on_realize();
	if (!xbm_)
		return;
	setBtnXpm(*xbm_);
}


void buttonSetXpm(Gtk::Button * btn, char const **xpm)
{
	Glib::RefPtr<Gdk::Bitmap> mask;
	Glib::RefPtr<Gdk::Pixmap> pixmap;
	Glib::RefPtr<Gdk::Colormap> clrmap = btn->get_colormap();
	Gtk::Image * image;
	pixmap = Gdk::Pixmap::create_from_xpm(clrmap,
					      mask,
					      xpm);
	image = SigC::manage(new Gtk::Image(pixmap, mask));
	image->show();
	btn->add(*image);
}
