// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1998-2000 The LyX Team
 *
 *======================================================*/

#ifndef PAINTERBASE_H
#define PAINTERBASE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "LColor.h"

class WorkArea;
class LyXFont;
class LyXImage;

/** A painter class to encapsulate all graphics parameters and operations
   
    Every graphics operation in LyX should be made by this class. It will 
    be initialized and managed by the Screen class, and will be passed
    as a parameter to inset.
 
    It hides low level windows system parameters so insets and other
    clients don't have to worry about them and we can control graphics and
    GUI toolkit dependent drawing functions inside this single class.
 
 */
class PainterBase {
protected:
        ///
	static int dummy1;
	///
	static int dummy2;
	///
	static int dummy3;
public:
	///
	enum line_width {
		///
		line_thin,
		///
		line_thick
	};

	///
	enum line_style {
		///
		line_solid,
		///
		line_doubledash,
		///
		line_onoffdash
	};

	///
	explicit PainterBase(WorkArea & wa) : owner(wa) {}
	
	///
	virtual ~PainterBase() {}

	/* Screen geometry */
	///
	int paperMargin() const;
	///
	int paperWidth() const;
	///
	int paperHeight() const;
	
	/// Draw a line from point to point
	virtual PainterBase & line(
		int x1, int y1, int x2, int y2, 
		LColor::color = LColor::foreground,
		enum line_style = line_solid,
		enum line_width = line_thin) = 0;

	/** Draw the lines between the lines in xp and yp.
	    xp and yp are arrays of points, and np is the
	    number of them. */
	virtual PainterBase & lines(
		int const * xp, int const * yp, int np,
		LColor::color = LColor::foreground,
		enum line_style = line_solid,
		enum line_width = line_thin) = 0;

	/// Here xp and yp are arrays of points
	virtual PainterBase & fillPolygon(
		int const * xp, int const * yp,
		int np,
		LColor::color = LColor::foreground) = 0;

	/// Draw lines from x1,y1 to x2,y2. They are arrays
        virtual PainterBase & segments(
		int const * x1, int const * y1, 
		int const * x2, int const * y2, int ns,
		LColor::color = LColor::foreground,
		enum line_style = line_solid,
		enum line_width = line_thin) = 0;
	
	/// Draw a rectangle 
	virtual PainterBase & rectangle(
		int x, int y, int w, int h,
		LColor::color = LColor::foreground,
		enum line_style = line_solid,
		enum line_width = line_thin) = 0;
	
	/// Draw a circle, d is the diameter, not the radious
	virtual PainterBase & circle(
		int x, int y, unsigned int d,
		LColor::color = LColor::foreground);

	/// Draw an ellipse
	virtual PainterBase & ellipse(
		int x, int y,
		unsigned int w, unsigned int h,
		LColor::color = LColor::foreground);
	
	/// Draw an arc
	virtual PainterBase & arc(
		int x, int y,
		unsigned int w, unsigned int h, 
		int a1, int a2,
		LColor::color = LColor::foreground) = 0;
	
	/// Draw a pixel
	virtual PainterBase & point(
		int x, int y,
		LColor::color = LColor::foreground) = 0;
	
	/// Fill a rectangle
	virtual PainterBase & fillRectangle(
		int x, int y, int w, int h,
		LColor::color = LColor::background) = 0;
	
	/// A filled rectangle with the shape of a 3D button
	virtual PainterBase & button(int x, int y, int w, int h);

	/// 
        virtual PainterBase & buttonFrame(int x, int y, int w, int h);
	
	
	// For the figure inset
	virtual PainterBase & image(int x, int y, int w, int h, LyXImage const * image) = 0;

	
	/// Draw a string at position x, y (y is the baseline)
	virtual PainterBase & text(int x, int y,
				   string const & str, LyXFont const & f) = 0;

	/** Draw a string at position x, y (y is the baseline)
	    This is just for fast drawing */
	virtual PainterBase & text(int x, int y, char const * str, size_t l,
		       LyXFont const & f) = 0;

	/// Draw a char at position x, y (y is the baseline)
	virtual PainterBase & text(int x, int y, char c, LyXFont const & f)=0;
	
	/** Draws a string and encloses it inside a rectangle. Returns
	    the size of the rectangle. If draw is false, we only calculate
	    the size. */
	virtual PainterBase & rectText(int x, int baseline, 
			   string const & string, 
			   LyXFont const & font,
			   LColor::color back,
			   LColor::color frame, bool draw = true,
			   int & width = PainterBase::dummy1,
			   int & ascent = PainterBase::dummy2, 
			   int & descent = PainterBase::dummy3);

	/** Draw a string and encloses it inside a button frame. Returns
	    the size of the frame. If draw is false, we only calculate
	    the size. */
	virtual PainterBase & buttonText(int x, int baseline, string const & s,
			     LyXFont const & font, bool draw = true,
			     int & width = PainterBase::dummy1,
			     int & ascent = PainterBase::dummy2, 
			     int & descent = PainterBase::dummy3);
protected:
	///
	WorkArea & owner;
};

#endif
