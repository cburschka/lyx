#include <config.h>

#include "lyxdraw.h"
#include "debug.h"

extern int reverse_video;
extern int mono_video;
extern char label_color[];
extern char math_color[];
extern char math_frame_color[];
extern char latex_color[];
extern char foot_color[];
extern char new_line_color[];
extern char fill_color[];
extern int fast_selection;
extern char on_off_line_color[];
extern string background_color;
extern char lighted_color[];
extern char selection_color[];
extern char note_color[];
extern char note_frame_color[];

Colormap color_map = 0;
long int background_pixels;

// X11 color names
char const * const X11Color[11] = 
{ "black", "black", "white", "red", "green", "blue", "cyan", "magenta", 
  "yellow", "black", "black" };
 
// Different graphic contexts
static GC clear_gc = 0;
static GC latex_gc = 0;
static GC foot_gc = 0;
static GC new_line_gc = 0;
static GC math_gc = 0;
static GC math_frame_gc = 0;
static GC fill_gc = 0;
static GC note_gc = 0;
static GC note_frame_gc = 0;
static GC copy_gc = 0;
static GC select_gc = 0;
static GC on_off_line_gc = 0;
static GC thin_on_off_line_gc = 0;
static GC thick_line_gc = 0;
static GC lighted_gc = 0;
static GC selection_gc = 0;
static GC accent_gc[10] = { 0, 0, 0, 0, 0,
			    0, 0, 0, 0, 0 };
static GC color_gc[11] = { 0, 0, 0, 0, 0, 0,
			   0, 0, 0, 0, 0 };
static GC minipage_gc = 0;


// Allocates color and sets it as foreground
// Returns "false" if unsuccesful
bool setForegroundColor(char const * const color, XGCValues & val)
{
	XColor xcol;
	if (!color_map) {
		color_map = fl_state[fl_get_vclass()].colormap;
	}
	if (!mono_video) {
		if (XParseColor(fl_display, color_map, color, &xcol)
		    && XAllocColor(fl_display, color_map, &xcol))
			{
				val.foreground = xcol.pixel;
			} else {
				lyxerr << "LyX: Couldn't get color "
				       << color << endl;
				return false;
			}
	}
	return true;
}


static
void do_reverse_video(XGCValues &val)
{
	if (reverse_video) {
		val.foreground= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
		val.background= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
	} else {
		val.foreground= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
		val.background= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
	}
}


static
GC make_normal_gc(const char* color)
{
	XGCValues val;
	do_reverse_video(val);
	val.function = GXcopy;
	val.graphics_exposures = false;
	setForegroundColor(color, val);
	return XCreateGC(fl_display, fl_root,
			 GCBackground | GCForeground | GCFunction |
			 GCGraphicsExposures, &val);
}


static
GC GetNoteGC()
{
	if (note_gc) return note_gc;
	
 	note_gc = make_normal_gc(note_color);

	return note_gc;
}


static
GC GetNoteFrameGC()
{
	if (note_frame_gc) return note_frame_gc;
	
 	note_frame_gc = make_normal_gc(note_frame_color);

	return note_frame_gc;
}


static
GC GetMathGC()
{
	if (math_gc) return math_gc;
	
 	math_gc = make_normal_gc(math_color);

	return math_gc;
}


static
GC GetLatexGC()
{
	if (latex_gc) return latex_gc;

	XGCValues val;
	if (reverse_video ^ mono_video) {
		val.foreground= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
		val.background= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
	} else {
		val.foreground= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
		val.background= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
	}
	val.function= GXcopy;
	val.graphics_exposures = false;
	setForegroundColor(latex_color, val);
	latex_gc = XCreateGC(fl_display, fl_root, GCBackground 
			     | GCForeground | GCFunction
			     | GCGraphicsExposures, 
			     &val);
	XFlush(fl_display);
	
	return latex_gc;
}


static
GC GetFootGC()
{
	if (foot_gc) return foot_gc;
	
	foot_gc = make_normal_gc(foot_color);
	
	return foot_gc;
}


static
GC GetNewLineGC()
{
        if (new_line_gc) return new_line_gc;
 
        new_line_gc = make_normal_gc(new_line_color);
 
        return new_line_gc;
}
 
 
static
GC GetMathFrameGC()
{
	if (math_frame_gc) return math_frame_gc;
	
	math_frame_gc = make_normal_gc(math_frame_color);
	
	return math_frame_gc;
}


static
GC GetFillGC()
{
	if (fill_gc) return fill_gc;
	
	fill_gc = make_normal_gc(fill_color);
	
	return fill_gc;
}


static
GC GetClearGC()
{
	if (clear_gc) return clear_gc;
	
	XGCValues val;

	if (reverse_video) {
		val.foreground= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
		val.background= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
	} else {
		val.background= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
		val.foreground= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
	}
	val.function = GXcopy;
	val.graphics_exposures = false;
	if (!fast_selection && background_color != "white") {
		setForegroundColor(background_color.c_str(), val);
	}
	background_pixels = val.foreground;
	
	clear_gc = XCreateGC(fl_display, fl_root, GCBackground 
			     | GCForeground | GCFunction
			     | GCGraphicsExposures, 
			     &val);
	XFlush(fl_display);
	
	return clear_gc;
}


static
GC GetOnOffLineGC()
{
	if (on_off_line_gc) return on_off_line_gc;
	
	XGCValues val;
	do_reverse_video(val);

	val.function= GXcopy;
	val.graphics_exposures = false;
	setForegroundColor(on_off_line_color, val);
	val.line_width = 0;
	val.line_style = LineOnOffDash;
	on_off_line_gc = XCreateGC(fl_display, fl_root,
				   GCBackground | GCForeground | GCFunction |
				   GCGraphicsExposures | GCLineWidth |
				   GCLineStyle , &val);
	XFlush(fl_display);

	return on_off_line_gc;
}


static
GC GetThickLineGC()
{
	if (thick_line_gc) return thick_line_gc;
	XGCValues val;
	do_reverse_video(val);

	val.function= GXcopy;
	val.graphics_exposures = false;
	val.line_width = 2;
	val.line_style = LineSolid;
	thick_line_gc = XCreateGC(fl_display, fl_root, GCBackground 
				  | GCForeground | GCFunction
				  | GCGraphicsExposures
				  | GCLineWidth | GCLineStyle , &val);
	XFlush(fl_display);
	
	return thick_line_gc;
}


static
GC GetThinOnOffLineGC()
{
	if (thin_on_off_line_gc) return thin_on_off_line_gc;
	XGCValues val;
	do_reverse_video(val);

	val.function= GXcopy;
	val.graphics_exposures = false;
	val.line_style = LineOnOffDash;
	val.line_width = 0;
	thin_on_off_line_gc = 
		XCreateGC(fl_display, fl_root, GCBackground
			  | GCForeground | GCFunction | GCGraphicsExposures
			  | GCLineWidth | GCLineStyle , &val);
	XFlush(fl_display);
	
	return thin_on_off_line_gc;
}


static
GC GetCopyGC()
{
	if (copy_gc) return copy_gc;
	XGCValues val;
	do_reverse_video(val);

	val.function= GXcopy;
	val.graphics_exposures = false;
	val.line_style = LineSolid;
	val.line_width = 0;
	copy_gc = XCreateGC(fl_display, fl_root, GCBackground
			    | GCForeground | GCFunction | GCGraphicsExposures
			    | GCLineWidth | GCLineStyle , &val);
	XFlush(fl_display);

	return copy_gc;
}


static
GC GetSelectGC()
{
	if (select_gc) return select_gc;
	XGCValues val;
	unsigned int a = BlackPixel(fl_display, DefaultScreen(fl_display));
	unsigned int b = WhitePixel(fl_display, DefaultScreen(fl_display)); 
	val.plane_mask = a^b;
	val.line_style = LineSolid;
	val.line_width = 2;
	val.graphics_exposures = false;
	val.function= GXinvert;
	select_gc = XCreateGC(fl_display, fl_root,
			      GCFunction | GCGraphicsExposures | GCPlaneMask
			      | GCLineWidth | GCLineStyle , &val);
	XFlush(fl_display);

	return select_gc; 
}


static
GC GetSelectionGC()
{
	if (selection_gc) return selection_gc;
	
	XGCValues val;

	if (!reverse_video) {
		val.foreground= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
		val.background= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
	} else {
		val.background= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
		val.foreground= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
	}
	val.function= GXcopy;
	val.graphics_exposures = false;
	if (!fast_selection && selection_color[0] != 0) {
		if (!setForegroundColor(selection_color, val)) {
			fast_selection = True;
			if (clear_gc) clear_gc = 0;
			lyxerr << "     Will use FastSelection-method."
			       << endl;
		}
	}
	selection_gc = XCreateGC(fl_display, fl_root,
				 GCBackground | GCForeground | GCFunction
				 | GCGraphicsExposures, &val);

	return selection_gc;
}


static
GC GetLightedGC()
{
	if (lighted_gc) return lighted_gc;
	XGCValues val;
	if (reverse_video) {
		val.background= BlackPixel(fl_display,
					   DefaultScreen(fl_display));
	} else {
		val.background= WhitePixel(fl_display,
					   DefaultScreen(fl_display));
	}
	val.foreground= val.background;
	val.function= GXcopy;
	val.graphics_exposures = false;
	val.line_style = LineSolid;
	val.line_width = 0;
	setForegroundColor(lighted_color, val);
	lighted_gc = XCreateGC(fl_display, fl_root, GCBackground
			       | GCForeground | GCFunction
			       | GCGraphicsExposures
			       | GCLineWidth | GCLineStyle , &val);
	XFlush(fl_display);

	return lighted_gc;
}


GC GetColorGC(LyXFont::FONT_COLOR color)
{
	if (color_gc[color]) return color_gc[color];
		       
	XGCValues val;
	do_reverse_video(val);

	val.function= GXcopy;
	val.graphics_exposures = false;
	setForegroundColor(X11Color[color], val);
	val.line_width = 0;
	val.line_style = LineSolid;
	color_gc[color] = XCreateGC(fl_display, fl_root,
				    GCBackground | GCForeground | GCFunction |
				    GCGraphicsExposures | GCLineWidth |
				    GCLineStyle , &val);
	XFlush(fl_display);

	return color_gc[color];
}


GC GetAccentGC(LyXFont const &f, int line_width)
{
	if (line_width >= 10) line_width = 9;
	
	if (accent_gc[line_width]) return accent_gc[line_width];
	
	// Build this one from normal text GC, but change linewidth.
	XGCValues val;
	GC gc = f.getGC();
	XGetGCValues(fl_display, gc, GCBackground | GCForeground | 
		     GCFunction | GCGraphicsExposures | GCLineWidth | 
		     GCLineStyle | GCPlaneMask, &val);

	val.line_width = line_width;
	val.cap_style = CapRound;
	val.join_style = JoinRound;
	
	GC ac = XCreateGC(fl_display, fl_root, 
			  GCBackground | GCForeground | GCFunction | 
			  GCGraphicsExposures | GCLineWidth | 
			  GCLineStyle | GCPlaneMask, &val);
	
	XFlush(fl_display);
	accent_gc[line_width] = ac;

	return accent_gc[line_width];
}


static
GC GetMinipageGC()
{
	if (minipage_gc) return minipage_gc;
        XGCValues val;
	do_reverse_video(val);

        val.function= GXcopy;
        val.graphics_exposures = false;
        val.line_style = LineOnOffDash;
        val.line_width = 0;
        setForegroundColor(fill_color, val);
        minipage_gc = XCreateGC(fl_display, fl_root,
                                GCBackground | GCForeground | GCFunction |
                                GCGraphicsExposures | GCLineWidth |
                                GCLineStyle , &val);
        XFlush(fl_display);

	return minipage_gc;
}


GC getGC(gc_type typ)
{
	GC gc = 0;
	switch(typ) {
	case gc_clear:
		gc = GetClearGC();
		break;
	case gc_latex:
		gc = GetLatexGC();
		break;
	case gc_foot:
		gc = GetFootGC();
		break;
	case gc_new_line:
	        gc = GetNewLineGC();
                break;
	case gc_math:
		gc = GetMathGC();
		break;
	case gc_math_frame:
		gc = GetMathFrameGC();
		break;
	case gc_fill:
		gc = GetFillGC();
		break;
	case gc_copy:
		gc = GetCopyGC();
		break;
	case gc_select:
		gc = GetSelectGC();
		break;
	case gc_on_off_line:
		gc = GetOnOffLineGC();
		break;
	case gc_thin_on_off_line:
		gc = GetThinOnOffLineGC();
		break;
	case gc_thick_line:
		gc = GetThickLineGC();
		break;
	case gc_lighted:
		gc = GetLightedGC();
		break;
	case gc_selection:
		gc = GetSelectionGC();
		break;
	case gc_minipage:
		gc = GetMinipageGC();
		break;
	case gc_note:
		gc = GetNoteGC();
		break;
	case gc_note_frame:
		gc = GetNoteFrameGC();
		break;
	}
	return gc;
}
