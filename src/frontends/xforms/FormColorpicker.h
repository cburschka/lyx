// -*- C++ -*-
/**
 * \file FormColorpicker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMCOLORPICKER_H
#define FORMCOLORPICKER_H

#include "Color.h"
#include "forms_fwd.h"

#include <boost/scoped_ptr.hpp>
#include <X11/Xlib.h> // for Pixmap

namespace lyx {
namespace frontend {

class FD_colorpicker;
class FD_colorpicker_rgb;
class FD_colorpicker_hsv;
class Tooltips;

class FormColorpicker {
public:
	FormColorpicker();
	~FormColorpicker();

	/** Launch the color picker. All other dialogs are frozen till
	    a color is chosen (or the dialog is closed).
	    \param color the color used to initialise the dialog.
	 */
	RGBColor const & requestColor(RGBColor const & color);

	/** Input callback function.
	 *  Invoked only by the xforms callback interface
	 */
	void input(FL_OBJECT *, long);

private:
	/// Build the dialog
	void build();
	/// Hide the dialog.
	void hide() const;
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Update dialog before/whilst showing it.
	void update() const;

	Tooltips & tooltips() const;
	/// Pointer to the actual instantiation of xform's form
	FL_FORM * form() const;

	/** Called on the first show() request, initialising various bits and
	 *  pieces.
	 */
	void prepare_to_show();

	void InputRGB();
	void InputHSV();

	/// The dialog's minimum allowable dimensions.
	int minw_;
	int minh_;

	/// The title displayed by the Window Manager.
	std::string title_;

	RGBColor input_color_;
	RGBColor color_;
	bool finished_;

	/// Passed to the window manager to give a pretty little symbol ;-)
	Pixmap icon_pixmap_;
	Pixmap icon_mask_;

	boost::scoped_ptr<Tooltips> tooltips_;
	boost::scoped_ptr<FD_colorpicker> dialog_;
	boost::scoped_ptr<FD_colorpicker_rgb> rgb_;
	boost::scoped_ptr<FD_colorpicker_hsv> hsv_;
};

} // namespace frontend
} // namespace lyx

#endif // FORMCOLORPICKER_H
