/*
 *	figinset.C - part of LyX project
 */

/*  Rework of path-handling (Matthias 04.07.1996 )
 * ------------------------------------------------
 *   figinsets keep an absolute path to the eps-file.
 *   For the user alway a relative path appears
 *   (in lyx-file, latex-file and edit-popup).
 *   To calculate this relative path the path to the
 *   document where the figinset is in is needed. 
 *   This is done by a reference to the buffer, called
 *   figinset::cbuffer. To be up to date the cbuffer
 *   is sometimes set to the current buffer 
 *   bufferlist.current(), when it can be assumed that
 *   this operation happens in the current buffer. 
 *   This is true for InsetFig::Edit(...), 
 *   InsetFig::InsetFig(...), InsetFig::Read(...),
 *   InsetFig::Write and InsetFig::Latex(...).
 *   Therefore the bufferlist has to make sure that
 *   during these operations bufferlist.current() 
 *   returns the buffer where the figinsets are in.
 *   This made few changes in buffer.C necessary.
 *
 * The above is not totally valid anymore. (Lgb)
 */


#include <config.h>

#include <fstream>
#include <queue>
#include <list>
#include <algorithm>
#include <vector>
#include <utility>

#include <unistd.h>
#include <csignal>
#include <sys/wait.h>

#include FORMS_H_LOCATION
#include <cstdlib>
#include <cctype>
#include <cmath>

#include "figinset.h"
#include "lyx_main.h"
#include "buffer.h"
#include "frontends/FileDialog.h"
#include "support/filetools.h"
#include "LyXView.h" // just because of form_main
#include "debug.h"
#include "LaTeXFeatures.h"
#include "lyxrc.h"
#include "gettext.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "support/FileInfo.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "Painter.h"
#include "font.h"
#include "bufferview_funcs.h"
#include "ColorHandler.h"
#include "converter.h"
#include "frontends/Dialogs.h" // redrawGUI
#include "BufferView.h"

using std::ostream;
using std::istream;
using std::ofstream;
using std::ifstream;
using std::queue;
using std::list;
using std::vector;
using std::find;
using std::flush;
using std::endl;
using std::copy;
using std::pair;
using std::make_pair;

#ifndef CXX_GLOBAL_CSTD
using std::memcpy;
using std::sin;
using std::cos;
using std::fabs;
#endif



extern BufferView * current_view;
extern FL_OBJECT * figinset_canvas;

extern char ** environ; // is this only redundtant on linux systems? Lgb.

// xforms doesn't define this (but it should be in <forms.h>).
extern "C"
FL_APPEVENT_CB fl_set_preemptive_callback(Window, FL_APPEVENT_CB, void *);

namespace {

float const DEG2PI = 57.295779513;

struct queue_element {
	float rx, ry;          // resolution x and y
	int ofsx, ofsy;	       // x and y translation
	figdata * data;	       // we are doing it for this data
};

int const MAXGS = 3;			/* maximum 3 gs's at a time */

typedef vector<Figref *> figures_type;
typedef vector<figdata *> bitmaps_type;
figures_type figures; // all figures
bitmaps_type bitmaps; // all bitmaps

queue<queue_element> gsqueue; // queue for ghostscripting

int gsrunning = 0;	/* currently so many gs's are running */
bool bitmap_waiting = false; /* bitmaps are waiting finished */

bool gs_color;			// do we allocate colors for gs?
bool color_visual;     		// is the visual color?
bool gs_xcolor = false;		// allocated extended colors
unsigned long gs_pixels[128];	// allocated pixels
int gs_spc;			// shades per color
int gs_allcolors;		// number of all colors

list<int> pidwaitlist; // pid wait list

GC createGC()
{
	XGCValues val;
	val.foreground = BlackPixel(fl_get_display(), 
				    DefaultScreen(fl_get_display()));
	
	val.function=GXcopy;
	val.graphics_exposures = false;
	val.line_style = LineSolid;
	val.line_width = 0;
	return XCreateGC(fl_get_display(), RootWindow(fl_get_display(), 0), 
			 GCForeground | GCFunction | GCGraphicsExposures
			 | GCLineWidth | GCLineStyle , &val);
}

GC local_gc_copy;


void addpidwait(int pid)
{
	// adds pid to pid wait list
	pidwaitlist.push_back(pid);

	if (lyxerr.debugging()) {
		lyxerr << "Pids to wait for: \n";
		copy(pidwaitlist.begin(), pidwaitlist.end(),
		     std::ostream_iterator<int>(lyxerr, "\n"));
		lyxerr << flush;
	}
}


string make_tmp(int pid)
{
	return system_tempdir + "/~lyxgs" + tostr(pid) + ".ps";
}


void kill_gs(int pid, int sig)
{
	if (lyxerr.debugging()) 
		lyxerr << "Killing gs " << pid << endl;
	lyx::kill(pid, sig);
	lyx::unlink(make_tmp(pid));
}


extern "C"
int GhostscriptMsg(XEvent * ev, void *)
{
	// bin all events not of interest
	if (ev->type != ClientMessage)
		return FL_PREEMPT;

	XClientMessageEvent * e = reinterpret_cast<XClientMessageEvent*>(ev);

	if (lyxerr.debugging()) {
		lyxerr << "ClientMessage, win:[xx] gs:[" << e->data.l[0]
		       << "] pm:[" << e->data.l[1] << "]" << endl;
	}

	// just kill gs, that way it will work for sure
	// This loop looks like S**T so it probably is...
	for (bitmaps_type::iterator it = bitmaps.begin();
	     it != bitmaps.end(); ++it)
		if (static_cast<long>((*it)->bitmap) ==
		    static_cast<long>(e->data.l[1])) {
			// found the one
			figdata * p = (*it);
			p->gsdone = true;

			// first update p->bitmap, if necessary
			if (p->bitmap != None
			    && p->flags > (1|8) && gs_color && p->wid) {
				// query current colormap and re-render
				// the pixmap with proper colors
				XWindowAttributes wa;
				register XImage * im;
				int i;
				int y;
				int spc1 = gs_spc - 1;
				int spc2 = gs_spc * gs_spc;
				int wid = p->wid;
				int forkstat;
				Display * tmpdisp;
				GC gc = local_gc_copy;

				XGetWindowAttributes(fl_get_display(),
						     fl_get_canvas_id(
							     figinset_canvas),
						     &wa);
				XFlush(fl_get_display());
				if (lyxerr.debugging()) {
					lyxerr << "Starting image translation "
					       << p->bitmap << " "
					       << p->flags << " "
					       << p->wid << "x" << p->hgh
					       << " " << wa.depth
					       << " " << XYPixmap << endl;
				}
				// now fork rendering process
				forkstat = fork();
				if (forkstat == -1) {
					lyxerr[Debug::INFO]
						<< "Cannot fork, using slow "
						"method for pixmap translation." << endl;
					tmpdisp = fl_get_display();
				} else if (forkstat > 0) { // parent
					// register child
					if (lyxerr.debugging()) {
						lyxerr << "Spawned child "
						       << forkstat << endl;
					}
					addpidwait(forkstat);
					break;
				} else {  // child
					tmpdisp = XOpenDisplay(DisplayString(fl_get_display()));
					XFlush(tmpdisp);
				}
				im = XGetImage(tmpdisp, p->bitmap, 0, 0,
					       p->wid, p->hgh, (1<<wa.depth)-1, XYPixmap);
				XFlush(tmpdisp);
				if (lyxerr.debugging()) {
					lyxerr << "Got the image" << endl;
				}
				if (!im) {
					if (lyxerr.debugging()) {
						lyxerr << "Error getting the image" << endl;
					}
					goto noim;
				}
				{
				// query current colormap
					XColor * cmap = new XColor[gs_allcolors];
					for (i = 0; i < gs_allcolors; ++i) cmap[i].pixel = i;
					XQueryColors(tmpdisp,
						     fl_state[fl_get_vclass()]
						     .colormap, cmap,
						     gs_allcolors);
					XFlush(tmpdisp);
				// now we process all the image
					for (y = 0; y < p->hgh; ++y) {
						for (int x = 0; x < wid; ++x) {
							XColor * pc = cmap +
								XGetPixel(im, x, y);
							XFlush(tmpdisp);
							XPutPixel(im, x, y,
								  gs_pixels[((pc->red+6553)*
									     spc1/65535)*spc2+((pc->green+6553)*
											       spc1/65535)*gs_spc+((pc->blue+6553)*
														   spc1/65535)]);
							XFlush(tmpdisp);
						}
					}
				// This must be correct.
					delete [] cmap;
					if (lyxerr.debugging()) {
						lyxerr << "Putting image back"
						       << endl;
					}
					XPutImage(tmpdisp, p->bitmap,
						  gc, im, 0, 0,
						  0, 0, p->wid, p->hgh);
					XDestroyImage(im);
					if (lyxerr.debugging()) {
						lyxerr << "Done translation"
						       << endl;
					}
				}
			  noim:
				kill_gs(p->gspid, SIGHUP);
				if (forkstat == 0) {
					XCloseDisplay(tmpdisp);
					_exit(0);
				}
			} else {
				kill_gs(p->gspid, SIGHUP);
			}
			break;
		}
	return FL_PREEMPT;
}


void AllocColors(int num)
// allocate color cube numxnumxnum, if possible
{
	if (lyxerr.debugging()) {
		lyxerr << "Allocating color cube " << num
		       << 'x' << num << 'x' << num << endl;
	}

	if (num <= 1) {
		lyxerr << "Error allocating color colormap." << endl;
		gs_color = false;
		return;
	}
	if (num > 5) num = 5;
	XColor xcol;
	for (int i = 0; i < num * num * num; ++i) {
		xcol.red = short(65535 * (i / (num * num)) / (num - 1));
		xcol.green = short(65535 * ((i / num) % num) / (num - 1));
		xcol.blue = short(65535 * (i % num) / (num - 1));
		xcol.flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(fl_get_display(),
				 fl_state[fl_get_vclass()].colormap, &xcol)) {
			if (i) XFreeColors(fl_get_display(),
					   fl_state[fl_get_vclass()].colormap,
					   gs_pixels, i, 0);
			if (lyxerr.debugging()) {
				lyxerr << "Cannot allocate color cube "
				       << num << endl;;
			}
			AllocColors(num - 1);
			return;
		}
		gs_pixels[i] = xcol.pixel;
	}
	gs_color = true;
	gs_spc = num;
}


// allocate grayscale ramp
void AllocGrays(int num)
{
	if (lyxerr.debugging()) {
		lyxerr << "Allocating grayscale colormap "
		       << num << endl;
	}

	if (num < 4) {
		lyxerr << "Error allocating grayscale colormap." << endl;
		gs_color = false;
		return;
	}
	if (num > 128) num = 128;
	XColor xcol;
	for (int i = 0; i < num; ++i) {
		xcol.red = xcol.green = xcol.blue = short(65535 * i / (num - 1));
		xcol.flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(fl_get_display(),
				 fl_state[fl_get_vclass()].colormap, &xcol)) {
			if (i) XFreeColors(fl_get_display(),
					   fl_state[fl_get_vclass()].colormap,
					   gs_pixels, i, 0);
			if (lyxerr.debugging()) {
				lyxerr << "Cannot allocate grayscale " 
				       << num << endl;
			}
			AllocGrays(num / 2);
			return;
		}
		gs_pixels[i] = xcol.pixel;
	}
	gs_color = true;
}

void InitFigures()
{
	// if bitmaps and figures are not empty we will leak mem
	figures.clear();
	bitmaps.clear();

	// allocate color cube on pseudo-color display
	// first get visual
	gs_color = false;
	if (lyxrc.use_gui) {
		/* we want to capture every event, in order to work around an
		 * xforms bug.
		 */
		fl_set_preemptive_callback(fl_get_canvas_id(figinset_canvas), GhostscriptMsg, 0);

		local_gc_copy = createGC();

		Visual * vi = DefaultVisual(fl_get_display(),
					    DefaultScreen(fl_get_display()));
		if (lyxerr.debugging()) {
			printf("Visual ID: %ld, class: %d, bprgb: %d, mapsz: %d\n", 
			       vi->visualid, vi->c_class, 
			       vi->bits_per_rgb, vi->map_entries);
		}
		color_visual = ( (vi->c_class == StaticColor) ||
				 (vi->c_class == PseudoColor) ||
				 (vi->c_class == TrueColor) ||
				 (vi->c_class == DirectColor) );
		if ((vi->c_class & 1) == 0) return;
		// now allocate colors
		if (vi->c_class == GrayScale) {
			// allocate grayscale
			AllocGrays(vi->map_entries/2);
		} else {
			// allocate normal color
			int i = 5;
			while (i * i * i * 2 > vi->map_entries) --i;
			AllocColors(i);
		}
		gs_allcolors = vi->map_entries;
	}
}


void DoneFigures()
{
	// if bitmaps and figures are not empty we will leak mem
	bitmaps.clear();
	figures.clear();
	
	lyxerr[Debug::INFO] << "Unregistering figures..." << endl;
}


void freefigdata(figdata * tmpdata)
{
	tmpdata->ref--;
	if (tmpdata->ref) return;

	if (tmpdata->gspid > 0) {
		int pid = tmpdata->gspid;
		// kill ghostscript and unlink it's files
		tmpdata->gspid = -1;
		kill_gs(pid, SIGKILL);
	}

	if (tmpdata->bitmap) XFreePixmap(fl_get_display(), tmpdata->bitmap);
	bitmaps.erase(find(bitmaps.begin(), bitmaps.end(), tmpdata));
	delete tmpdata;
}


void runqueue()
{
	// This _have_ to be set before the fork!
	unsigned long background_pixel =
		lyxColorHandler->colorPixel(LColor::background);
	
	// run queued requests for ghostscript, if any
	if (!gsrunning && gs_color && !gs_xcolor) {
		// here alloc all colors, so that gs will use only
		// those we allocated for it
		// *****
		gs_xcolor = true;
	}
	
	while (gsrunning < MAXGS) {
		//char tbuf[384]; //, tbuf2[80];
		Atom * prop;
		int nprop, i;

		if (gsqueue.empty()) {
			if (!gsrunning && gs_xcolor) {
				// de-allocate rest of colors
				// *****
				gs_xcolor = false;
			}
			return;
		}
		queue_element * p = &gsqueue.front();
		if (!p->data) {
			gsqueue.pop();
			continue;
		}

		int pid = ::fork();
		
		if (pid == -1) {
			if (lyxerr.debugging()) {
				lyxerr << "GS start error! Cannot fork."
				       << endl;
			}
			p->data->broken = true;
			p->data->reading = false;
			return;
		}
		if (pid == 0) { // child
			char ** env;
			int ne = 0;
			Display * tempdisp = XOpenDisplay(DisplayString(fl_get_display()));

			// create translation file
			ofstream ofs;
			ofs.open(make_tmp(getpid()).c_str());
			ofs << "gsave clippath pathbbox grestore\n"
			    << "4 dict begin\n"
			    << "/ury exch def /urx exch def /lly exch def "
				"/llx exch def\n"
			    << p->data->wid / 2.0 << " "
			    << p->data->hgh / 2.0 << " translate\n"
			    << p->data->angle << " rotate\n"
			    << -(p->data->raw_wid / 2.0) << " "
			    << -(p->data->raw_hgh / 2.0) << " translate\n"
			    << p->rx / 72.0 << " " << p->ry / 72.0
			    << " scale\n"
			    << -p->ofsx << " " << -p->ofsy << " translate\n"
			    << "end" << endl;
			ofs.close(); // Don't remove this.

			// gs process - set ghostview environment first
			ostringstream t2;
			t2 << "GHOSTVIEW=" << fl_get_canvas_id(figinset_canvas)
			   << ' ' << p->data->bitmap;
			// now set up ghostview property on a window
			// #ifdef WITH_WARNINGS
			// #warning BUG seems that the only bug here
			// might be the hardcoded dpi.. Bummer!
			// #endif
			ostringstream t1;
			t1 << "0 0 0 0 " << p->data->wid << ' '
			   << p->data->hgh << " 72 72 0 0 0 0";
			
			if (lyxerr.debugging()) {
				lyxerr << "Will set GHOSTVIEW property to ["
				       << t1.str() << "]" << endl;
			}
			// wait until property is deleted if executing multiple
			// ghostscripts
			XGrabServer(tempdisp);
			for (;;) {
				// grab server to prevent other child
				// interfering with setting GHOSTVIEW property
				// The grabbing goes on for too long, is it
				// really needed? (Lgb)
				// I moved most of the grabs... (Lgb)
				if (lyxerr.debugging()) {
					lyxerr << "Grabbing the server"
					       << endl;
				}
				prop = XListProperties(tempdisp,
						       fl_get_canvas_id(
					figinset_canvas), &nprop);
				if (!prop) break;

				bool err = true;
				for (i = 0; i < nprop; ++i) {
					char * p = XGetAtomName(tempdisp,
								prop[i]);
					if (compare(p, "GHOSTVIEW") == 0) {
						err = false;
						// We free it when we leave so we don't leak.
						XFree(p);
						break;
					}
					XFree(p);
				}
				XFree(reinterpret_cast<char *>(prop)); // jc:
				if (err) break;
				// ok, property found, we must wait until
				// ghostscript deletes it
				if (lyxerr.debugging()) {
					lyxerr << "Releasing the server\n["
					       << getpid()
					       << "] GHOSTVIEW property"
						" found. Waiting." << endl;
				}
				XUngrabServer(tempdisp);
				XFlush(tempdisp);
				::sleep(1);
				XGrabServer(tempdisp);
			}
			XChangeProperty(tempdisp, 
					fl_get_canvas_id(figinset_canvas),
					XInternAtom(tempdisp, "GHOSTVIEW", false),
					XInternAtom(tempdisp, "STRING", false),
					8, PropModeAppend, 
					reinterpret_cast<unsigned char*>(const_cast<char*>(t1.str().c_str())),
					int(t1.str().size()));
			XUngrabServer(tempdisp);
			XFlush(tempdisp);

			ostringstream t3;

			switch (p->data->flags & 3) {
			case 0: t3 << 'H'; break; // Hidden
			case 1: t3 << 'M'; break; // Mono
			case 2: t3 << 'G'; break; // Gray
			case 3:
				if (color_visual) 
					t3 << 'C'; // Color
				else 
					t3 << 'G'; // Gray
				break;
			}
	
			t3 << ' ' << BlackPixelOfScreen(DefaultScreenOfDisplay(tempdisp))
			   << ' ' << background_pixel;

			XGrabServer(tempdisp);
			XChangeProperty(tempdisp, 
					fl_get_canvas_id(figinset_canvas),
					XInternAtom(tempdisp,
						    "GHOSTVIEW_COLORS", false),
					XInternAtom(tempdisp, "STRING", false),
					8, PropModeReplace, 
					reinterpret_cast<unsigned char*>(const_cast<char*>(t3.str().c_str())),
					int(t3.str().size()));
			XUngrabServer(tempdisp);
			XFlush(tempdisp);
			
			if (lyxerr.debugging()) {
				lyxerr << "Releasing the server" << endl;
			}
			XCloseDisplay(tempdisp);

			// set up environment
			while (environ[ne])
				++ne;
			typedef char * char_p;
			env = new char_p[ne + 2];
			string tmp = t2.str().c_str();
			env[0] = new char[tmp.size() + 1];
			std::copy(tmp.begin(), tmp.end(), env[0]);
			env[0][tmp.size()] = '\0';
			memcpy(&env[1], environ,
			       sizeof(char*) * (ne + 1));
			environ = env;

			// now make gs command
			// close(0);
			// close(1); do NOT close. If GS writes out
			// errors it would hang. (Matthias 290596) 

			string rbuf = "-r" + tostr(p->rx) + "x" + tostr(p->ry);
			string gbuf = "-g" + tostr(p->data->wid) + "x" + tostr(p->data->hgh);

			// now chdir into dir with .eps file, to be on the safe
			// side
			lyx::chdir(OnlyPath(p->data->fname));
			// make temp file name
			string tmpf = make_tmp(getpid());
			if (lyxerr.debugging()) {
				lyxerr << "starting gs " << tmpf << " "
				       << p->data->fname
				       << ", pid: " << getpid() << endl;
			}

			int err = ::execlp(lyxrc.ps_command.c_str(), 
					 lyxrc.ps_command.c_str(), 
					 "-sDEVICE=x11",
					 "-dNOPAUSE", "-dQUIET",
					 "-dSAFER", 
					 rbuf.c_str(), gbuf.c_str(), tmpf.c_str(), 
					 p->data->fname.c_str(), 
					 "showpage.ps", "quit.ps", "-", 0);
			// if we are still there, an error occurred.
			lyxerr << "Error executing ghostscript. "
			       << "Code: " << err << endl;
			lyxerr[Debug::INFO] << "Cmd: " 
				       << lyxrc.ps_command
				       << " -sDEVICE=x11 "
				       << tmpf << ' '
				       << p->data->fname << endl;
			_exit(0);	// no gs?
		}
		// normal process (parent)
		if (lyxerr.debugging()) {
			lyxerr << "GS ["  << pid << "] started" << endl;
		}

		p->data->gspid = pid;
		++gsrunning;
		gsqueue.pop();
	}
}


void addwait(int psx, int psy, int pswid, int pshgh, figdata * data)
{
	// recompute the stuff and put in the queue
	queue_element p;
	p.ofsx = psx;
	p.ofsy = psy;
	p.rx = (float(data->raw_wid) * 72.0) / pswid;
	p.ry = (float(data->raw_hgh) * 72.0) / pshgh;

	p.data = data;

	gsqueue.push(p);

	// if possible, run the queue
	runqueue();
}


figdata * getfigdata(int wid, int hgh, string const & fname, 
		     int psx, int psy, int pswid, int pshgh, 
		     int raw_wid, int raw_hgh, float angle, char flags)
{
	/* first search for an exact match with fname and width/height */

	if (fname.empty() || !IsFileReadable(fname)) 
		return 0;

	for (bitmaps_type::iterator it = bitmaps.begin();
	     it != bitmaps.end(); ++it) {
		if ((*it)->wid == wid && (*it)->hgh == hgh &&
		    (*it)->flags == flags && (*it)->fname == fname &&
		    (*it)->angle == angle) {
			(*it)->ref++;
			return (*it);
		}
	}
	figdata * p = new figdata;
	p->wid = wid;
	p->hgh = hgh;
	p->raw_wid = raw_wid;
	p->raw_hgh = raw_hgh;
	p->angle = angle;
	p->fname = fname;
	p->flags = flags;
	bitmaps.push_back(p);
	XWindowAttributes wa;
	XGetWindowAttributes(fl_get_display(), fl_get_canvas_id(
		figinset_canvas), &wa);

	if (lyxerr.debugging()) {
		lyxerr << "Create pixmap disp:" << fl_get_display()
		       << " scr:" << DefaultScreen(fl_get_display())
		       << " w:" << wid
		       << " h:" << hgh
		       << " depth:" << wa.depth << endl;
	}
	
	p->ref = 1;
	p->reading = false;
	p->broken = false;
	p->gspid = -1;
	if (flags) {
		p->bitmap = XCreatePixmap(fl_get_display(), fl_get_canvas_id(
			figinset_canvas), wid, hgh, wa.depth);
		p->gsdone = false;
		// initialize reading of .eps file with correct sizes and stuff
		addwait(psx, psy, pswid, pshgh, p);
		p->reading = true;
	} else {
		p->bitmap = None;
		p->gsdone = true;
	}

	return p;
}


void getbitmap(figdata * p)
{
	p->gspid = -1;
}


void makeupdatelist(figdata * p)
{
	for (figures_type::iterator it = figures.begin();
	    it != figures.end(); ++it)
		if ((*it)->data == p) {
			if (lyxerr.debugging()) {
				lyxerr << "Updating inset "
				       << (*it)->inset
				       << endl;
			}
			// add inset figures[i]->inset into to_update list
			current_view->pushIntoUpdateList((*it)->inset);
		}
}

} // namespace anon


// this func is only "called" in spellchecker.C
void sigchldchecker(pid_t pid, int * status)
{
	lyxerr[Debug::INFO] << "Got pid = " << pid << endl;
	bool pid_handled = false;
	for (bitmaps_type::iterator it = bitmaps.begin();
	     it != bitmaps.end(); ++it) {
		if ((*it)->reading && pid == (*it)->gspid) {
			lyxerr[Debug::INFO] << "Found pid in bitmaps" << endl;
			// now read the file and remove it from disk
			figdata * p = (*it);
			p->reading = false;
			if ((*it)->gsdone) *status = 0;
			if (*status == 0) {
				lyxerr[Debug::INFO] << "GS [" << pid
					       << "] exit OK." << endl;
			} else {
				lyxerr << "GS [" << pid  << "] error "
				       << *status << " E:"
				       << WIFEXITED(*status)
				       << " " << WEXITSTATUS(*status)
				       << " S:" << WIFSIGNALED(*status)
				       << " " << WTERMSIG(*status) << endl;
			}
			if (*status == 0) {
				bitmap_waiting = true;
				p->broken = false;
			} else {
				// remove temporary files
				lyx::unlink(make_tmp(p->gspid));
				p->gspid = -1;
				p->broken = true;
			}
			makeupdatelist((*it));
			--gsrunning;
			runqueue();
			pid_handled = true;
		}
	}
	if (!pid_handled) {
		lyxerr[Debug::INFO] << "Checking pid in pidwait" << endl;
		list<int>::iterator it = find(pidwaitlist.begin(),
					      pidwaitlist.end(), pid);
		if (it != pidwaitlist.end()) {
			lyxerr[Debug::INFO] << "Found pid in pidwait\n"
				       << "Caught child pid of recompute "
				"routine" << pid << endl;
			pidwaitlist.erase(it);
		}
	}
	if (pid == -1) {
		lyxerr[Debug::INFO] << "waitpid error" << endl;
		switch (errno) {
		case ECHILD:
			lyxerr << "The process or process group specified by "
				"pid does  not exist or is not a child of "
				"the calling process or can never be in the "
				"states specified by options." << endl;
			break;
		case EINTR:
			lyxerr << "waitpid() was interrupted due to the "
				"receipt of a signal sent by the calling "
				"process." << endl;
			break;
		case EINVAL:
			lyxerr << "An invalid value was specified for "
				"options." << endl;
			break;
		default:
			lyxerr << "Unknown error from waitpid" << endl;
			break;
		}
	} else if (pid == 0) {
		lyxerr << "waitpid nohang" << endl;;
	} else {
		lyxerr[Debug::INFO] << "normal exit from childhandler" << endl;
	}
}


namespace {

void getbitmaps()
{
	bitmap_waiting = false;
	for (bitmaps_type::iterator it = bitmaps.begin();
	     it != bitmaps.end(); ++it)
		if ((*it)->gspid > 0 && !(*it)->reading)
			getbitmap((*it));
}


void RegisterFigure(InsetFig * fi)
{
	if (figures.empty()) InitFigures();
	fi->form = 0;
	Figref * tmpfig = new Figref;
	tmpfig->data = 0;
	tmpfig->inset = fi;
	figures.push_back(tmpfig);
	fi->figure = tmpfig;

	if (lyxerr.debugging() && current_view) {
		lyxerr << "Register Figure: buffer:["
		       << current_view->buffer() << "]" << endl;
	}
}


void UnregisterFigure(InsetFig * fi)
{
	if (!lyxrc.use_gui)
		return;

	Figref * tmpfig = fi->figure;

	if (tmpfig->data) freefigdata(tmpfig->data);
	if (tmpfig->inset->form) {
		if (tmpfig->inset->form->Figure->visible) {
			fl_set_focus_object(tmpfig->inset->form->Figure,
					    tmpfig->inset->form->OkBtn);
			fl_hide_form(tmpfig->inset->form->Figure);
		}
#if FL_REVISION == 89
		// CHECK Reactivate this free_form calls
#else
		fl_free_form(tmpfig->inset->form->Figure);
		free(tmpfig->inset->form); // Why free?
		tmpfig->inset->form = 0;
#endif
	}
	figures.erase(find(figures.begin(), figures.end(), tmpfig));
	delete tmpfig;

	if (figures.empty()) DoneFigures();
}

} // namespace anon


InsetFig::InsetFig(int tmpx, int tmpy, Buffer const & o)
	: owner(&o)
{
	wid = tmpx;
	hgh = tmpy;
	wtype = DEF;
	htype = DEF;
	twtype = DEF;
	thtype = DEF;
	pflags = flags = 9;
	psubfigure = subfigure = false;
	xwid = xhgh = angle = 0;
	pswid = pshgh = 0;
	raw_wid = raw_hgh = 0;
	changedfname = false;
	RegisterFigure(this);
	r_ = Dialogs::redrawGUI.connect(SigC::slot(this, &InsetFig::redraw));
}


InsetFig::~InsetFig()
{
	if (lyxerr.debugging()) {
		lyxerr << "Figure destructor called" << endl;
	}
	UnregisterFigure(this);
	r_.disconnect();
}


void InsetFig::redraw()
{
	if (form && form->Figure->visible)
		fl_redraw_form(form->Figure);
}


int InsetFig::ascent(BufferView *, LyXFont const &) const
{
	return hgh + 3;
}


int InsetFig::descent(BufferView *, LyXFont const &) const
{
	return 1;
}


int InsetFig::width(BufferView *, LyXFont const &) const
{
	return wid + 2;
}


void InsetFig::draw(BufferView * bv, LyXFont const & f,
		    int baseline, float & x, bool) const
{
	LyXFont font(f);
	Painter & pain = bv->painter();
	
	if (bitmap_waiting) getbitmaps();
	
	// I wish that I didn't have to use this
	// but the figinset code is so complicated so
	// I don't want to fiddle with it now.

	if (figure && figure->data && figure->data->bitmap &&
	    !figure->data->reading && !figure->data->broken) {
		// draw the bitmap
		pain.pixmap(int(x + 1), baseline - hgh,
			    wid, hgh, figure->data->bitmap);

		if (flags & 4)
			pain.rectangle(int(x), baseline - hgh - 1,
				       wid + 1, hgh + 1);
		
	} else {
		//char const * msg = 0;
		string msg;
		string lfname = fname;
		if (!fname.empty() && GetExtension(fname).empty())
		    lfname += ".eps";
		// draw frame
		pain.rectangle(int(x), baseline - hgh - 1, wid + 1, hgh + 1);

		if (figure && figure->data) {
			if (figure->data->broken)  msg = _("[render error]");
			else if (figure->data->reading) msg = _("[rendering ... ]");
		} 
		else if (fname.empty()) 
			msg = _("[no file]");
		else if (!IsFileReadable(lfname))
			msg = _("[bad file name]");
		else if ((flags & 3) == 0) 
			msg = _("[not displayed]");
		else if (lyxrc.ps_command.empty()) 
			msg = _("[no ghostscript]");
		
		if (msg.empty()) msg = _("[unknown error]");
		
		font.setFamily(LyXFont::SANS_FAMILY);
		font.setSize(LyXFont::SIZE_FOOTNOTE);
		string const justname = OnlyFilename (fname);
		pain.text(int(x + 8), baseline - lyxfont::maxAscent(font) - 4,
			  justname, font);
		
		font.setSize(LyXFont::SIZE_TINY);
		pain.text(int(x + 8), baseline - 4, msg, font);
	}
	x += width(bv, font);    // ?
}


void InsetFig::write(Buffer const *, ostream & os) const
{
	regenerate();
	os << "Figure size " << wid << " " << hgh << "\n";
	if (!fname.empty()) {
		string buf1 = OnlyPath(owner->fileName());
		string fname2 = MakeRelPath(fname, buf1);
		os << "file " << fname2 << "\n";
	}
	if (!subcaption.empty())
		os << "subcaption " << subcaption << "\n";
	if (wtype) os << "width " << static_cast<int>(wtype) << " " << xwid << "\n";
	if (htype) os << "height " << static_cast<int>(htype) << " " << xhgh << "\n";
	if (angle != 0) os << "angle " << angle << "\n";
	os << "flags " << flags << "\n";
	if (subfigure) os << "subfigure\n";
}


void InsetFig::read(Buffer const *, LyXLex & lex)
{
	string buf;
	bool finished = false;
	
	while (lex.IsOK() && !finished) {
		lex.next();

		string const token = lex.GetString();
		lyxerr[Debug::INFO] << "Token: " << token << endl;
		
		if (token.empty())
			continue;
		else if (token == "\\end_inset") {
			finished = true;
		} else if (token == "file") {
			if (lex.next()) {
				buf = lex.GetString();
				string buf1 = OnlyPath(owner->fileName());
				fname = MakeAbsPath(buf, buf1);
				changedfname = true;
			}
		} else if (token == "extra") {
			if (lex.next());
			// kept for backwards compability. Delete in 0.13.x
		} else if (token == "subcaption") {
			if (lex.EatLine())
				subcaption = lex.GetString();
		} else if (token == "label") {
			if (lex.next());
			// kept for backwards compability. Delete in 0.13.x
		} else if (token == "angle") {
			if (lex.next())
				angle = lex.GetFloat();
		} else if (token == "size") {
			if (lex.next())
				wid = lex.GetInteger();
			if (lex.next())
				hgh = lex.GetInteger();
		} else if (token == "flags") {
			if (lex.next())
				flags = pflags = lex.GetInteger();
		} else if (token == "subfigure") {
			subfigure = psubfigure = true;
		} else if (token == "width") {
			int typ = 0;
			if (lex.next())
				typ = lex.GetInteger();
			if (lex.next())
				xwid = lex.GetFloat();
			switch (typ) {
			case DEF: wtype = DEF; break;
			case CM: wtype = CM; break;
			case IN: wtype = IN; break;
			case PER_PAGE: wtype = PER_PAGE; break;
			case PER_COL: wtype = PER_COL; break;
			default:
				lyxerr[Debug::INFO] << "Unknown type!" << endl;
				break;
			}
			twtype = wtype;
		} else if (token == "height") {
			int typ = 0;
			if (lex.next())
				typ = lex.GetInteger();
			if (lex.next())
				xhgh = lex.GetFloat();
			switch (typ) {
			case DEF: htype = DEF; break;
			case CM: htype = CM; break;
			case IN: htype = IN; break;
			case PER_PAGE: htype = PER_PAGE; break;
			default:
				lyxerr[Debug::INFO] << "Unknown type!" << endl;
				break;
			}
			thtype = htype;
		}
	}
	regenerate();
	recompute();
}


int InsetFig::latex(Buffer const *, ostream & os,
		    bool /* fragile*/, bool /* fs*/) const
{
	regenerate();
	if (!cmd.empty()) os << cmd << " ";
	return 0;
}


int InsetFig::ascii(Buffer const *, ostream &, int) const
{
	return 0;
}


int InsetFig::linuxdoc(Buffer const *, ostream &) const
{
	return 0;
}


int InsetFig::docBook(Buffer const *, ostream & os) const
{
	string const buf1 = OnlyPath(owner->fileName());
	string figurename = MakeRelPath(fname, buf1);

	if (suffixIs(figurename, ".eps"))
		figurename.erase(figurename.length() - 4);

	os << "@<graphic fileref=\"" << figurename << "\"></graphic>";
	return 0;
} 


void InsetFig::validate(LaTeXFeatures & features) const
{
	features.graphics = true;
	if (subfigure) features.subfigure = true;
}


Inset::EDITABLE InsetFig::editable() const
{
	return IS_EDITABLE;
}


bool InsetFig::deletable() const
{
	return false;
}


string const InsetFig::editMessage() const 
{
	return _("Opened figure");
}


void InsetFig::edit(BufferView * bv, int, int, unsigned int)
{
	lyxerr[Debug::INFO] << "Editing InsetFig." << endl;
	regenerate();

	// We should have RO-versions of the form instead.
	// The actual prevention of altering a readonly doc
	// is done in CallbackFig()
	if (bv->buffer()->isReadonly()) 
		WarnReadonly(bv->buffer()->fileName());

	if (!form) {
		form = create_form_Figure();
		fl_set_form_atclose(form->Figure, CancelCloseBoxCB, 0);
		fl_set_object_return(form->Angle, FL_RETURN_ALWAYS);
		fl_set_object_return(form->Width, FL_RETURN_ALWAYS);
		fl_set_object_return(form->Height, FL_RETURN_ALWAYS);
	}
	restoreForm();
	if (form->Figure->visible) {
		fl_raise_form(form->Figure);
	} else {
		fl_show_form(form->Figure,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_TRANSIENT,
			     _("Figure"));
	}
}


Inset * InsetFig::clone(Buffer const & buffer, bool) const
{
	InsetFig * tmp = new InsetFig(100, 100, buffer);

	if (lyxerr.debugging()) {
		lyxerr << "Clone Figure: buffer:["
		       << &buffer
		       << "], cbuffer:[xx]" << endl;
	}

	tmp->wid = wid;
	tmp->hgh = hgh;
	tmp->raw_wid = raw_wid;
	tmp->raw_hgh = raw_hgh;
	tmp->angle = angle;
	tmp->xwid = xwid;
	tmp->xhgh = xhgh;
	tmp->flags = flags;
	tmp->pflags = pflags;
	tmp->subfigure = subfigure;
	tmp->psubfigure = psubfigure;
	tmp->wtype = wtype;
	tmp->htype = htype;
	tmp->psx = psx;
	tmp->psy = psy;
	tmp->pswid = pswid;
	tmp->pshgh = pshgh;
	tmp->fname = fname;
	string lfname = fname;
	if (!fname.empty() && GetExtension(fname).empty())
		lfname += ".eps";
	if (!fname.empty() && IsFileReadable(lfname) 
	    && (flags & 3) && !lyxrc.ps_command.empty()
	    && lyxrc.use_gui) { 
		// do not display if there is
		// "do not display" chosen (Matthias 260696)
		tmp->figure->data = getfigdata(wid, hgh, lfname, psx, psy,
					       pswid, pshgh, raw_wid, raw_hgh,
					       angle, flags & (3|8));
	} else tmp->figure->data = 0;
	tmp->subcaption = subcaption;
	tmp->changedfname = false;
	tmp->owner = owner;
	tmp->regenerate();
	return tmp;
}


Inset::Code InsetFig::lyxCode() const
{
	return Inset::GRAPHICS_CODE;
}


namespace {

string const stringify(InsetFig::HWTYPE hw, float f, string suffix)
{
	string res;
	switch (hw) {
		case InsetFig::DEF:
			break;
		case InsetFig::CM:// \resizebox*{h-length}{v-length}{text}
			res = tostr(f) + "cm";
			break;
		case InsetFig::IN: 
			res = tostr(f) + "in";
			break;
		case InsetFig::PER_PAGE:
			res = tostr(f/100) + "\\text" + suffix;
			break;
		case InsetFig::PER_COL:
			// Doesn't occur for htype...
			res = tostr(f/100) + "\\column" + suffix;
			break;
	}
	return res;
}

} // namespace anon


void InsetFig::regenerate() const
{
	string cmdbuf;
	string resizeW;
	string resizeH;
	string rotate;
	string recmd;

	if (fname.empty()) {
		cmd = "\\fbox{\\rule[-0.5in]{0pt}{1in}";
	        cmd += _("empty figure path");
		cmd += '}';
		return;
	}

	string buf1 = OnlyPath(owner->fileName());
	string fname2 = MakeRelPath(fname, buf1);

	string gcmd = "\\includegraphics{" + fname2 + '}';
	resizeW = stringify(wtype, xwid, "width");
	resizeH = stringify(htype, xhgh, "height");

	if (!resizeW.empty() || !resizeH.empty()) {
		recmd = "\\resizebox*{";
		if (!resizeW.empty())
			recmd += resizeW;
		else
			recmd += '!';
		recmd += "}{";
		if (!resizeH.empty())
			recmd += resizeH;
		else
			recmd += '!';
		recmd += "}{";
	}
	
	
	if (angle != 0) {
		// \rotatebox{angle}{text}
		rotate = "\\rotatebox{" + tostr(angle) + "}{";
	}

	cmdbuf = recmd;
	cmdbuf += rotate;
	cmdbuf += gcmd;
	if (!rotate.empty()) cmdbuf += '}';
	if (!recmd.empty()) cmdbuf += '}';
	if (subfigure) {
		if (!subcaption.empty())
			cmdbuf = "\\subfigure[" + subcaption +
				"]{" + cmdbuf + "}";
		else
			cmdbuf = "\\subfigure{" + cmdbuf + "}";
	}
	
	cmd = cmdbuf;
}


void InsetFig::tempRegenerate()
{
	string cmdbuf;
	string resizeW;
	string resizeH;
	string rotate;
	string recmd;
	
	char const * tfname = fl_get_input(form->EpsFile);
	string tsubcap = fl_get_input(form->Subcaption);
	float tangle = atof(fl_get_input(form->Angle));
	float txwid = atof(fl_get_input(form->Width));
	float txhgh = atof(fl_get_input(form->Height));

	if (!tfname || !*tfname) {
		cmd = "\\fbox{\\rule[-0.5in]{0pt}{1in}";
		cmd += _("empty figure path");
		cmd += '}';
		return;
	}

	string buf1 = OnlyPath(owner->fileName());
	string fname2 = MakeRelPath(tfname, buf1);
	// \includegraphics*[<llx,lly>][<urx,ury>]{file}
	string gcmd = "\\includegraphics{" + fname2 + '}';

	resizeW = stringify(twtype, txwid, "width"); 	
	resizeH = stringify(thtype, txhgh, "height"); 	

	// \resizebox*{h-length}{v-length}{text}
	if (!resizeW.empty() || !resizeH.empty()) {
		recmd = "\\resizebox*{";
		if (!resizeW.empty())
			recmd += resizeW;
		else
			recmd += '!';
		recmd += "}{";
		if (!resizeH.empty())
			recmd += resizeH;
		else
			recmd += '!';
		recmd += "}{";
	}
	
	if (tangle != 0) {
		// \rotatebox{angle}{text}
		rotate = "\\rotatebox{" + tostr(tangle) + "}{";
	}

	cmdbuf = recmd + rotate + gcmd;
	if (!rotate.empty()) cmdbuf += '}';
	if (!recmd.empty()) cmdbuf += '}';
	if (psubfigure && !tsubcap.empty()) {
		cmdbuf = string("\\subfigure{") + tsubcap
			+ string("}{") + cmdbuf + "}";
	}
}


void InsetFig::recompute()
{
	if (!lyxrc.use_gui)
		return;

	bool changed = changedfname;
	int newx, newy, nraw_x, nraw_y;

	if (changed) getPSSizes();

	float sin_a = sin(angle / DEG2PI);  /* rotation; H. Zeller 021296 */
	float cos_a = cos(angle / DEG2PI);
	int frame_wid = int(ceil(fabs(cos_a * pswid) + fabs(sin_a * pshgh)));
	int frame_hgh= int(ceil(fabs(cos_a * pshgh) + fabs(sin_a * pswid)));

	string lfname = fname;
	if (GetExtension(fname).empty())
	    lfname += ".eps";

	/* now recompute wid and hgh, and if that is changed, set changed */
	/* this depends on chosen size of the picture and its bbox */
	// This will be redone in 0.13 ... (hen)
	if (!lfname.empty() && IsFileReadable(lfname)) {
		// say, total width is 595 pts, as A4 in TeX, thats in 1/72" */

		newx = frame_wid;
		newy = frame_hgh;
		switch (wtype) {
		case DEF:
			break;
		case CM:	/* cm */
			newx = int(28.346 * xwid);
			break;
		case IN: /* in */
			newx = int(72 * xwid);
			break;
		case PER_PAGE:	/* % of page */
			newx = int(5.95 * xwid);
			break;
		case PER_COL:	/* % of col */
			newx = int(2.975 * xwid);
			break;
		}
		
		if (wtype && frame_wid) newy = newx*frame_hgh/frame_wid;
		
		switch (htype) {
		case DEF:
			//lyxerr << "This should not happen!" << endl;
			break;
		case CM:        /* cm */
			newy = int(28.346 * xhgh);
			break;
		case IN: /* in */
			newy = int(72 * xhgh);
			break;
		case PER_PAGE:  /* % of page */
			newy = int(8.42 * xhgh);
			break;
		case PER_COL: 
			// Doesn't occur; case exists to suppress
			// compiler warnings.  
                        break;
		}
		if (htype && !wtype && frame_hgh)
			newx = newy*frame_wid/frame_hgh;
	} else {
		newx = wid;
		newy = hgh;
	}

	if (frame_wid == 0)
		nraw_x = 5;
	else
		nraw_x = int((1.0 * pswid * newx)/frame_wid);

	if (frame_hgh == 0)
		nraw_y = 5;
	else
		nraw_y = int((1.0 * pshgh * newy)/frame_hgh);

	// cannot be zero, actually, set it to some minimum, so its clickable
	if (newx < 5) newx = 5;
	if (newy < 5) newy = 5;

	if (newx   != wid     || newy   != hgh     || 
	    nraw_x != raw_wid || nraw_y != raw_hgh ||
	    flags  != pflags  || subfigure != psubfigure) 
		changed = true;
       
	raw_wid = nraw_x;
	raw_hgh = nraw_y;
	wid = newx;
	hgh = newy;
	flags = pflags;
	subfigure = psubfigure;

	if (changed) {
		figdata * pf = figure->data;

		// get new data
		if (!lfname.empty() && IsFileReadable(lfname) && (flags & 3)
		    && !lyxrc.ps_command.empty()) {
			// do not display if there is "do not display"
			// chosen (Matthias 260696)
			figure->data = getfigdata(wid, hgh, lfname,
						  psx, psy, pswid, pshgh,
						  raw_wid, raw_hgh,
						  angle, flags & (3|8));
		} else figure->data = 0;

		// free the old data
		if (pf) freefigdata(pf);
	}

	changedfname = false;
}


void InsetFig::getPSSizes()
{
	/* get %%BoundingBox: from postscript file */
	
	/* defaults to associated size
	 * ..just in case the PS-file is not readable (Henner, 24-Aug-97) 
	 */
	psx = 0;
	psy = 0;
	pswid = wid;
	pshgh = hgh;

	if (fname.empty()) return;
	string p;
	string lfname = fname;
	if (GetExtension(fname).empty())
		lfname += ".eps";
	ifstream ifs(lfname.c_str());

	if (!ifs) return;	// file not found !!!!

	/* defaults to A4 page */
	psx = 0;
	psy = 0;
	pswid = 595;
	pshgh = 842;

	char lastchar = 0; ifs.get(lastchar);
	for (;;) {
		char c = 0; ifs.get(c);
		if (ifs.eof()) {
			lyxerr[Debug::INFO] << "End of (E)PS file reached and"
				" no BoundingBox!" << endl;
			break;
		}
		if (c == '%' && lastchar == '%') {
			ifs >> p;
			if (p.empty()) break;
			lyxerr[Debug::INFO] << "Token: `" << p << "'" << endl;
			if (p == "BoundingBox:") {
				float fpsx, fpsy, fpswid, fpshgh;
				if (ifs >> fpsx >> fpsy >> fpswid >> fpshgh) {
					psx = int(fpsx);
					psy = int(fpsy);
					pswid = int(fpswid);
					pshgh = int(fpshgh);
				}
				if (lyxerr.debugging()) {
					lyxerr << "%%%%BoundingBox:"
					       << psx << ' '
					       << psy << ' '
					       << pswid << ' '
					       << pshgh << endl;
				}
				break;
			}
			c = 0;
		}
		lastchar = c;
	}
	pswid -= psx;
	pshgh -= psy;

}


void InsetFig::callbackFig(long arg)
{
	bool regen = false;
	char const * p;

	if (lyxerr.debugging()) {
		lyxerr << "Figure callback, arg " << arg << endl;
	}

	switch (arg) {
	case 10:
	case 11:
	case 12:	/* width type */
	case 13:
	case 14:
		switch (arg - 10) {
		case DEF:
			twtype = DEF;
			// put disable here
			fl_deactivate_object(form->Width);
			break;
		case CM:
			twtype = CM;
			// put enable here
			fl_activate_object(form->Width);
			break;
		case IN:
			twtype = IN;
			// put enable here
			fl_activate_object(form->Width);
			break;
		case PER_PAGE:
			twtype = PER_PAGE;
			// put enable here
			fl_activate_object(form->Width);
			break;
		case PER_COL:
			twtype = PER_COL;
			// put enable here
			fl_activate_object(form->Width);
			break;
		default:
			lyxerr[Debug::INFO] << "Unknown type!" << endl;
			break;
		}
		regen = true;
		break;
	case 20:
	case 21:
	case 22:	/* height type */
	case 23:
		switch (arg - 20) {
		case DEF:
			thtype = DEF;
			// put disable here
			fl_deactivate_object(form->Height);
			break;
		case CM:
			thtype = CM;
			// put enable here
			fl_activate_object(form->Height);
			break;
		case IN:
			thtype = IN;
			// put enable here
			fl_activate_object(form->Height);
			break;
		case PER_PAGE:
			thtype = PER_PAGE;
			// put enable here
			fl_activate_object(form->Height);
			break;
		default:
			lyxerr[Debug::INFO] << "Unknown type!" << endl;
			break;
		}
		regen = true;
		break;
	case 3:
		pflags = pflags & ~3;		/* wysiwyg0 */
		break;
	case 33:
		pflags = (pflags & ~3) | 1;	/* wysiwyg1 */
		break;
	case 43:
		pflags = (pflags & ~3) | 2;	/* wysiwyg2 */
		break;
	case 63:
		pflags = (pflags & ~3) | 3;	/* wysiwyg3 */
		break;
	case 53:
		pflags ^= 4;	/* frame */
		break;
	case 54:
		pflags ^= 8;	/* do translations */
		break;
	case 70:
		psubfigure = !psubfigure;	/* This is a subfigure */
		break;
	case 2:
		regen = true;		/* regenerate command */
		break;
	case 0:				/* browse file */
		browseFile();
		regen = true;
		break;
	case 1:				/* preview */
		p = fl_get_input(form->EpsFile);
		preview(p);
		break;
	case 7:				/* apply */
	case 8:				/* ok (apply and close) */
		if (!current_view->buffer()->isReadonly()) {
			wtype = twtype;
			htype = thtype;
			xwid = atof(fl_get_input(form->Width));
			xhgh = atof(fl_get_input(form->Height));
			angle = atof(fl_get_input(form->Angle));
			p = fl_get_input(form->EpsFile);
			if (p && *p) {
				string buf1 = OnlyPath(owner->fileName());
				fname = MakeAbsPath(p, buf1);
				changedfname = true;
			} else {
				if (!fname.empty()) {
					changedfname = true;
					fname.erase();
				}
			}
			subcaption = fl_get_input(form->Subcaption);
	
			regenerate();
			recompute();
			/* now update inset */
			if (lyxerr.debugging()) {
				lyxerr << "Update: ["
				       << wid << 'x' << hgh << ']' << endl;
			}
			current_view->updateInset(this, true);
			if (arg == 8) {
				fl_set_focus_object(form->Figure, form->OkBtn);
				fl_hide_form(form->Figure);
#if FL_REVISION == 89
				// CHECK Reactivate this free_form calls
#else
				fl_free_form(form->Figure);
				free(form); // Why free?
				form = 0;
#endif
			}
			break;
		} //if not readonly
		//  The user has already been informed about RO in ::Edit
		if (arg == 7) // if 'Apply'
			break;
		// fall through
	case 9:				/* cancel = restore and close */
		fl_set_focus_object(form->Figure, form->OkBtn);
		fl_hide_form(form->Figure);
#if FL_REVISION == 89
		// CHECK Reactivate this free_form calls
		// Jug, is this still a problem?
#else
		fl_free_form(form->Figure);
		free(form); // Why free?
		form = 0;
#endif
		break;
	}

	if (regen) tempRegenerate();
}


inline
void DisableFigurePanel(FD_Figure * const form)
{
        fl_deactivate_object(form->EpsFile);
	fl_deactivate_object(form->Browse);
	fl_deactivate_object(form->Width);
	fl_deactivate_object(form->Height);
	fl_deactivate_object(form->Frame);
	fl_deactivate_object(form->Translations);
	fl_deactivate_object(form->Angle);
	fl_deactivate_object(form->HeightGrp);
	fl_deactivate_object(form->page2);
	fl_deactivate_object(form->Default2);
	fl_deactivate_object(form->cm2);
	fl_deactivate_object(form->in2);
	fl_deactivate_object(form->HeightLabel);
	fl_deactivate_object(form->WidthLabel);
	fl_deactivate_object(form->DisplayGrp);
	fl_deactivate_object(form->Wysiwyg3);
	fl_deactivate_object(form->Wysiwyg0);
	fl_deactivate_object(form->Wysiwyg2);
	fl_deactivate_object(form->Wysiwyg1);
	fl_deactivate_object(form->WidthGrp);
	fl_deactivate_object(form->Default1);
	fl_deactivate_object(form->cm1);
	fl_deactivate_object(form->in1);
	fl_deactivate_object(form->page1);
	fl_deactivate_object(form->column1);
	fl_deactivate_object(form->Subcaption);
	fl_deactivate_object(form->Subfigure);
	fl_deactivate_object (form->OkBtn);
	fl_deactivate_object (form->ApplyBtn);
	fl_set_object_lcol (form->OkBtn, FL_INACTIVE);
	fl_set_object_lcol (form->ApplyBtn, FL_INACTIVE);
}


inline
void EnableFigurePanel(FD_Figure * const form)
{
        fl_activate_object(form->EpsFile);
	fl_activate_object(form->Browse);
	fl_activate_object(form->Width);
	fl_activate_object(form->Height);
	fl_activate_object(form->Frame);
	fl_activate_object(form->Translations);
	fl_activate_object(form->Angle);
	fl_activate_object(form->HeightGrp);
	fl_activate_object(form->page2);
	fl_activate_object(form->Default2);
	fl_activate_object(form->cm2);
	fl_activate_object(form->in2);
	fl_activate_object(form->HeightLabel);
	fl_activate_object(form->WidthLabel);
	fl_activate_object(form->DisplayGrp);
	fl_activate_object(form->Wysiwyg3);
	fl_activate_object(form->Wysiwyg0);
	fl_activate_object(form->Wysiwyg2);
	fl_activate_object(form->Wysiwyg1);
	fl_activate_object(form->WidthGrp);
	fl_activate_object(form->Default1);
	fl_activate_object(form->cm1);
	fl_activate_object(form->in1);
	fl_activate_object(form->page1);
	fl_activate_object(form->column1);
	fl_activate_object(form->Subcaption);
	fl_activate_object(form->Subfigure);
	fl_activate_object (form->OkBtn);
	fl_activate_object (form->ApplyBtn);
	fl_set_object_lcol (form->OkBtn, FL_BLACK);
	fl_set_object_lcol (form->ApplyBtn, FL_BLACK);
}


void InsetFig::restoreForm()
{
	EnableFigurePanel(form);

	twtype = wtype;
	fl_set_button(form->Default1, (wtype == 0));
	fl_set_button(form->cm1, (wtype == 1));
	fl_set_button(form->in1, (wtype == 2));
	fl_set_button(form->page1, (wtype == 3));
	fl_set_button(form->column1, (wtype == 4));
	if (wtype == 0) {
		fl_deactivate_object(form->Width);
	} else {
		fl_activate_object(form->Width);
	}
		
	// enable and disable should be put here.
	thtype = htype;
	fl_set_button(form->Default2, (htype == 0));
	fl_set_button(form->cm2, (htype == 1));
	fl_set_button(form->in2, (htype == 2));
	fl_set_button(form->page2, (htype == 3));
	// enable and disable should be put here.
	if (htype == 0) {
		fl_deactivate_object(form->Height);
	} else {
		fl_activate_object(form->Height);
	}

	int piflags = flags & 3;
	fl_set_button(form->Wysiwyg0, (piflags == 0));
	fl_set_button(form->Wysiwyg1, (piflags == 1));
	fl_set_button(form->Wysiwyg2, (piflags == 2));
	fl_set_button(form->Wysiwyg3, (piflags == 3));
	fl_set_button(form->Frame, ((flags & 4) != 0));
	fl_set_button(form->Translations, ((flags & 8) != 0));
	fl_set_button(form->Subfigure, (subfigure != 0));
	pflags = flags;
	psubfigure = subfigure;
	fl_set_input(form->Width, tostr(xwid).c_str());
	fl_set_input(form->Height, tostr(xhgh).c_str());
	fl_set_input(form->Angle, tostr(angle).c_str());
	if (!fname.empty()){
		string buf1 = OnlyPath(owner->fileName());
		string fname2 = MakeRelPath(fname, buf1);
		fl_set_input(form->EpsFile, fname2.c_str());
	}
	else fl_set_input(form->EpsFile, "");
	fl_set_input(form->Subcaption, subcaption.c_str());
	if (current_view->buffer()->isReadonly()) 
	        DisableFigurePanel(form);

	tempRegenerate();
}


void InsetFig::preview(string const & p)
{
	string tfname = p;
	if (GetExtension(tfname).empty())
	    tfname += ".eps";
	string buf1 = OnlyPath(owner->fileName());
	string buf2 = os::external_path(MakeAbsPath(tfname, buf1));
	if (!formats.View(owner, buf2, "eps"))
		lyxerr << "Can't view " << buf2 << endl;
}


void InsetFig::browseFile()
{
	static string current_figure_path;
	static int once;

	if (lyxerr.debugging()) {
		lyxerr << "Filename: "
		       << owner->fileName() << endl;
	}
	string p = fl_get_input(form->EpsFile);

	string buf = MakeAbsPath(owner->fileName());
	string buf2 = OnlyPath(buf);
	if (!p.empty()) {
		buf = MakeAbsPath(p, buf2);
		buf = OnlyPath(buf);
	} else {
		buf = OnlyPath(owner->fileName());
	}
	
	// Does user clipart directory exist?
	string bufclip = AddName (user_lyxdir, "clipart");	
	FileInfo fileInfo(bufclip);
	if (!(fileInfo.isOK() && fileInfo.isDir()))
		// No - bail out to system clipart directory
		bufclip = AddName (system_lyxdir, "clipart");	


	FileDialog fileDlg(current_view->owner(), _("Select an EPS figure"),
		LFUN_SELECT_FILE_SYNC,
		make_pair(string(_("Clip art")), string(bufclip)),
		make_pair(string(_("Documents")), string(buf)));

	bool error = false;
	do {
		string const path = (once) ? current_figure_path : buf;

		FileDialog::Result result = fileDlg.Select(path, _("*ps| PostScript documents"));

		string const p = result.second;

		if (p.empty())
			return;

		buf = MakeRelPath(p, buf2);
		current_figure_path = OnlyPath(p);
		once = 1;
		
		if (contains(p, "#") || contains(p, "~") || contains(p, "$")
		    || contains(p, "%") || contains(p, " ")) {
			WriteAlert(_("Filename can't contain any "
				     "of these characters:"),
				   // xgettext:no-c-format
				   _("space, '#', '~', '$' or '%'.")); 
			error = true;
		}
	} while (error);

	if (form) fl_set_input(form->EpsFile, buf.c_str());
}


void GraphicsCB(FL_OBJECT * obj, long arg)
{
	/* obj->form contains the form */

	if (lyxerr.debugging()) {
		lyxerr << "GraphicsCB callback: " << arg << endl;
	}

	/* find inset we were reacting to */
	for (figures_type::iterator it = figures.begin();
	     it != figures.end(); ++it)
		if ((*it)->inset->form && (*it)->inset->form->Figure
		    == obj->form) {
			if (lyxerr.debugging()) {
				lyxerr << "Calling back figure "
				       << (*it) << endl;
			}
			(*it)->inset->callbackFig(arg);
			return;
		}
}


void HideFiguresPopups()
{
	for (figures_type::iterator it = figures.begin();
	     it != figures.end(); ++it)
		if ((*it)->inset->form 
		    && (*it)->inset->form->Figure->visible) {
			if (lyxerr.debugging()) {
				lyxerr << "Hiding figure " << (*it) << endl;
			}
			// hide and free the form
			(*it)->inset->callbackFig(9);
		}
}
