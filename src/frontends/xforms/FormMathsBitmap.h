// -*- C++ -*-
/**
 * \file FormMathsBitmap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSBITMAP_H
#define FORM_MATHSBITMAP_H


#include "FormBase.h"
#include <boost/shared_ptr.hpp>
#include <vector>

struct BitmapStore
{
	BitmapStore(int nt_in, int nx_in, int ny_in, int bw_in, int bh_in,
		    unsigned char const * data_in, bool vert_in)
		: nt(nt_in), nx(nx_in), ny(ny_in), bw(bw_in), bh(bh_in),
		  data(data_in), vert(vert_in)
	{}

	int nt;
	int nx;
	int ny;
	int bw;
	int bh;
	unsigned char const * data;
	bool vert;
};


struct FD_maths_bitmap
{
	~FD_maths_bitmap();
	FL_FORM * form;
	FL_OBJECT * button_close;
};


/**
 * This class provides an XForms implementation of a maths bitmap form.
 */

class ControlMathSub;

class FormMathsBitmap : public FormCB<ControlMathSub, FormDB<FD_maths_bitmap> >
{
public:
	///
	FormMathsBitmap(string const &, std::vector<string> const &);
	///
	void addBitmap(int, int, int, int, int, unsigned char const *,
		       bool = true);

private:
	///
	int GetIndex(FL_OBJECT *);

	///
	virtual void apply();
	///
	virtual void build();
	///
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	/// Not needed.
	virtual void update() {}

	///
	FL_OBJECT * buildBitmap(BitmapStore const & bmstore);

	/// The latex names associated with each symbol
	std::vector<string> latex_;
	/// The latex name chosen
	string latex_chosen_;

	/** Temporary store for bitmap data passed to addBitmap()
	 *  but before the FL_OBJECT is created in build().
	 */
	std::vector<BitmapStore> bitmaps_;

	/// Border width
	int ww_;
	///
	int x_;
	///
	int y_;
	///
	int w_;
	///
	int h_;
};


#endif // FORM_MATHSBITMAP_H
