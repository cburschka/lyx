/*
 *	figinset.C - part of LyX project
 */

extern int	reverse_video;
extern long int background_pixels;

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

#include <unistd.h>
#include <csignal>
#include <sys/wait.h>

#include FORMS_H_LOCATION
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <fstream>
using std::ofstream;

#include "figinset.h"
#include "lyx.h"
#include "lyx_main.h"
#include "buffer.h"
#include "filedlg.h"
#include "support/filetools.h"
#include "LyXView.h" // just because of form_main
#include "debug.h"
#include "lyxdraw.h"
#include "LaTeXFeatures.h"
#include "lyxrc.h"
#include "gettext.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "support/FileInfo.h"

extern BufferView * current_view;
#if 0
static volatile bool alarmed;
#endif
extern FL_OBJECT * figinset_canvas;
#if 0
//inline
extern "C" void waitalarm(int)
{
	alarmed = true;
}
#endif

extern char ** environ; // is this only redundtant on linux systems? Lgb.
extern void UpdateInset(Inset * inset, bool mark_dirty = true);
// better for asyncron updating:
void PutInsetIntoInsetUpdateList(Inset * inset);
extern void ProhibitInput();
extern void AllowInput();

static float const DEG2PI = 57.295779513;
static int const figallocchunk = 32;

static int figinsref = 0;	/* number of figures */
static int figarrsize = 0;	/* current max number of figures */
static int bmpinsref = 0;	/* number of bitmaps */
static int bmparrsize = 0;	/* current max number of bitmaps */

struct queue {
	float rx, ry;		/* resolution x and y */
	int ofsx, ofsy;		/* x and y translation */
	figdata * data;		/* we are doing it for this data */
	queue * next;	        /* next item in queue */
};

struct pidwait {
	int pid;		/* pid to wait for */
	pidwait * next;	/* next */
};

static int const MAXGS = 3;			/* maximum 3 gs's at a time */

static Figref ** figures;	/* all the figures */
static figdata ** bitmaps;	/* all the bitmaps */
static queue * gsqueue = 0;	/* queue for ghostscripting */
static int gsrunning = 0;	/* currently so many gs's are running */
static bool bitmap_waiting = false; /* bitmaps are waiting finished */
static char bittable[256];	/* bit reversion table */

static bool gs_color;			// do we allocate colors for gs?
static bool color_visual;     		// is the visual color?
static bool gs_xcolor = false;		// allocated extended colors
static unsigned long gs_pixels[128];	// allocated pixels
static int gs_num_pixels;		// number of pixels allocated
static int gs_spc;			// shades per color
static bool gs_gray;			// is grayscale?
static int gs_allcolors;		// number of all colors

static pidwait * pw = 0;		// pid wait list


extern Colormap color_map;

void addpidwait(int pid)
{
	// adds pid to pid wait list
	register pidwait * p = new pidwait;

	p->pid = pid;
	p->next = pw;
	pw = p;

	if (lyxerr.debugging()) {
		lyxerr << "Pids to wait for: " << p->pid << endl;
		while (p->next) {
			p = p->next;
			lyxerr << p->pid << endl;
		}
	}
}


extern "C" int GhostscriptMsg(FL_OBJECT *, Window, int, int,
			      XEvent * ev, void *)
{
	char tmp[128];

	XClientMessageEvent * e = reinterpret_cast<XClientMessageEvent*>(ev);

	if(lyxerr.debugging()) {
		lyxerr << "ClientMessage, win:[xx] gs:[" << e->data.l[0]
		       << "] pm:[" << e->data.l[1] << "]" << endl;
	}

	// just kill gs, that way it will work for sure
	// This loop looks like S**T so it probably is...
	for (int i = 0; i < bmpinsref; ++i)
		if ((long)bitmaps[i]->bitmap == (long)e->data.l[1]) {
			// found the one
			figdata * p = bitmaps[i];
			p->gsdone = true;

			// first update p->bitmap, if necessary
			if (p->bitmap != None
			    && p->flags > (1|8) && gs_color && p->wid) {
				// query current colormap and re-render
				// the pixmap with proper colors
				//XColor * cmap;
				XWindowAttributes wa;
				register XImage * im;
				int i, y, wid1, spc1 = gs_spc-1,
					spc2 = gs_spc * gs_spc, wid = p->wid,
					forkstat;
				Display * tmpdisp;
				GC gc = getGC(gc_copy);

				XGetWindowAttributes(fl_display,
						     fl_get_canvas_id(
							     figinset_canvas),
						     &wa);
				XFlush(fl_display);
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
					lyxerr.debug()
						<< "Cannot fork, using slow "
						"method for pixmap translation." << endl;
					tmpdisp = fl_display;
				} else if (forkstat > 0) {
					// register child
					if (lyxerr.debugging()) {
						lyxerr << "Spawned child "
						       << forkstat << endl;
					}
					addpidwait(forkstat);
					break; // in parent process
				} else {
					tmpdisp = XOpenDisplay(XDisplayName(0));
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
				//cmap = (XColor *) malloc(gs_allcolors*sizeof(XColor));
					XColor * cmap = new XColor[gs_allcolors];
					for (i = 0; i < gs_allcolors; ++i) cmap[i].pixel = i;
					XQueryColors(tmpdisp, color_map, cmap, gs_allcolors);
					XFlush(tmpdisp);
					wid1 = p->wid - 1;
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
				if (lyxerr.debugging()) {
					lyxerr << "Killing gs " 
					       << p->gspid << endl;
				}
				kill(p->gspid, SIGHUP);

				sprintf(tmp, "%s/~lyxgs%d.ps",
					system_tempdir.c_str(), 
					p->gspid);
				unlink(tmp);
				if (forkstat == 0) {
					XCloseDisplay(tmpdisp);
					_exit(0);
				}
			} else {
				if (lyxerr.debugging()) {
					lyxerr << "Killing gs " 
					       << p->gspid << endl;
				}
				kill(p->gspid, SIGHUP);

				sprintf(tmp, "%s/~lyxgs%d.ps", 
					system_tempdir.c_str(),
					p->gspid);
				unlink(tmp);
			}
			break;
		}
	return 0;
}


static void AllocColors(int num)
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
		xcol.red = 65535 * (i / (num * num)) / (num - 1);
		xcol.green = 65535 * ((i / num) % num) / (num - 1);
		xcol.blue = 65535 * (i % num) / (num - 1);
		xcol.flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(fl_display, color_map, &xcol)) {
			if (i) XFreeColors(fl_display, color_map,
					   gs_pixels, i, 0);
			if(lyxerr.debugging()) {
				lyxerr << "Cannot allocate color cube "
				       << num << endl;;
			}
			AllocColors(num - 1);
			return;
		}
		gs_pixels[i] = xcol.pixel;
	}
	gs_color = true;
	gs_gray = false;
	gs_spc = num;
	gs_num_pixels = num * num * num;
}


// allocate grayscale ramp
static
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
		xcol.red = xcol.green = xcol.blue = 65535 * i / (num - 1);
		xcol.flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(fl_display, color_map, &xcol)) {
			if (i) XFreeColors(fl_display, color_map,
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
	gs_gray = false;
	gs_num_pixels = num;
}


void InitFigures()
{
	bmparrsize = figarrsize = figallocchunk;
	figures = static_cast<Figref**>
		(malloc(sizeof(Figref*) * figallocchunk));
	bitmaps = static_cast<figdata**>
		(malloc(sizeof(figdata*) * figallocchunk));

	unsigned int k;
	for (unsigned int i = 0; i < 256; ++i) {
		k = 0;
		for (unsigned int j = 0; j < 8; ++j)
			if (i & (1 << (7-j))) k |= 1 << j;
		bittable[i] = char(~k);
	}

	fl_add_canvas_handler(figinset_canvas, ClientMessage,
			      GhostscriptMsg, current_view->owner()->getMainForm());

	// now we have to init color_map
	if (!color_map) color_map = DefaultColormap(fl_display,
						    DefaultScreen(fl_display));
	// allocate color cube on pseudo-color display
	// first get visual
	gs_color = false;

	Visual * vi = DefaultVisual(fl_display, DefaultScreen(fl_display));
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


void DoneFigures()
{
	free(figures);
	free(bitmaps);
	figarrsize = 0;
	bmparrsize = 0;

	lyxerr.debug() << "Unregistering figures..." << endl;

	fl_remove_canvas_handler(figinset_canvas, ClientMessage,
				 GhostscriptMsg);

	if (gs_color) {
		lyxerr.debug() << "Freeing up the colors..." << endl;
		XFreeColors(fl_display, color_map, gs_pixels,
			    gs_num_pixels, 0);
		/******????????????????? what's planes in this case ??????***/
	}
}


int FindBmpIndex(figdata * tmpdata)
{
	int i = 0;
	while (i < bmpinsref) {
		if (bitmaps[i] == tmpdata) return i;
		++i;
	}
	return i;
}


static void chpixmap(Pixmap, int, int)
{
#if 0
	Display * tempdisp = XOpenDisplay(XDisplayName(0));

	// here read the pixmap and change all colors to those we
	// have allocated

	XCloseDisplay(tempdisp);
#endif
}


static void freefigdata(figdata * tmpdata)
{
	tmpdata->ref--;
	if (tmpdata->ref) return;

	if (tmpdata->gspid > 0) {
		int pid = tmpdata->gspid;
		char buf[128];
		// change Pixmap according to our allocated colormap
		chpixmap(tmpdata->bitmap, tmpdata->wid, tmpdata->hgh);
		// kill ghostscript and unlink it's files
		tmpdata->gspid = -1;
		kill(pid, SIGKILL);
		sprintf(buf, "%s/~lyxgs%d.ps", system_tempdir.c_str(), pid);
		unlink(buf);
	}

	if (tmpdata->bitmap) XFreePixmap(fl_display, tmpdata->bitmap);
	delete tmpdata;
	int i = FindBmpIndex(tmpdata);
	--bmpinsref;
	while (i < bmpinsref) {
		bitmaps[i] = bitmaps[i + 1];
		++i;
	}
}


static void runqueue()
{
	// run queued requests for ghostscript, if any
	if (!gsrunning && gs_color && !gs_xcolor) {
		// here alloc all colors, so that gs will use only
		// those we allocated for it
		// *****
		gs_xcolor = true;
	}
	
	while (gsrunning < MAXGS) {
		char tbuf[384], tbuf2[80];
		Atom * prop;
		int nprop, i;

		if (!gsqueue) {
			if (!gsrunning && gs_xcolor) {
				// de-allocate rest of colors
				// *****
				gs_xcolor = false;
			}
			return;
		}
		queue * p = gsqueue;

		if (!p->data) {
			delete p;
			continue;
		}

		int pid = fork();
		
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
			char ** env, rbuf[80], gbuf[40];
			int ne = 0;
			Display * tempdisp = XOpenDisplay(XDisplayName(0));

			// create translation file
			sprintf(tbuf, "%s/~lyxgs%d.ps", system_tempdir.c_str(),
				int(getpid()));
			
			ofstream ofs(tbuf);
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
			sprintf(tbuf2, "GHOSTVIEW=%ld %ld", fl_get_canvas_id(
				figinset_canvas), p->data->bitmap);

			// now set up ghostview property on a window
			sprintf(tbuf, "0 0 0 0 %d %d 72 72 0 0 0 0",
				p->data->wid, p->data->hgh);
			// #warning BUG seems that the only bug here
			// might be the hardcoded dpi.. Bummer!
			
			if (lyxerr.debugging()) {
				lyxerr << "Will set GHOSTVIEW property to ["
				       << tbuf << "]" << endl;
			}
			// wait until property is deleted if executing multiple
			// ghostscripts
			for (;;) {
				// grab server to prevent other child
				// interfering with setting GHOSTVIEW property
				if (lyxerr.debugging()) {
					lyxerr << "Grabbing the server"
					       << endl;
				}
				XGrabServer(tempdisp);
				prop = XListProperties(tempdisp,
						       fl_get_canvas_id(
					figinset_canvas), &nprop);
				if (!prop) break;

				bool err = true;
				for (i = 0; i < nprop; ++i) {
					char * p = XGetAtomName(tempdisp,
								prop[i]);
					if (strcmp(p, "GHOSTVIEW") == 0) {
						err = false;
						break;
					}
					XFree(p);
				}
				XFree(reinterpret_cast<char *>(prop)); // jc:
				if (err) break;
				// release the server
				XUngrabServer(tempdisp);
				XFlush(tempdisp);
				// ok, property found, we must wait until
				// ghostscript deletes it
				if (lyxerr.debugging()) {
					lyxerr << "Releasing the server\n["
					       << getpid()
					       << "] GHOSTVIEW property"
						" found. Waiting." << endl;
				}
#if 0
#ifdef WITH_WARNINGS
#warning What is this doing? (wouldn't a sleep(1); work too?')
#endif
				alarm(1);
				alarmed = false;
				signal(SIGALRM, waitalarm);
				while (!alarmed) pause();
#else
				sleep(1);
#endif
			}

			XChangeProperty(tempdisp, 
					fl_get_canvas_id(figinset_canvas),
					XInternAtom(tempdisp, "GHOSTVIEW", false),
					XInternAtom(tempdisp, "STRING", false),
					8, PropModeAppend, 
					reinterpret_cast<unsigned char*>(tbuf),
					strlen(tbuf));
			
			switch (p->data->flags & 3) {
			case 0: tbuf[0] = 'H'; break; // Hidden
			case 1: tbuf[0] = 'M'; break; // Mono
			case 2: tbuf[0] = 'G'; break; // Gray
			case 3:
				if (color_visual) 
					tbuf[0] = 'C'; // Color
				else 
					tbuf[0] = 'G'; // Gray
				break;
			}

			if (reverse_video) {
				sprintf(tbuf+1, " %ld %ld", WhitePixelOfScreen(
					DefaultScreenOfDisplay(fl_display)),
					background_pixels);
			} else {
				sprintf(tbuf+1, " %ld %ld", BlackPixelOfScreen(
					DefaultScreenOfDisplay(fl_display)),
					background_pixels);
			}

			XChangeProperty(tempdisp, 
					fl_get_canvas_id(figinset_canvas),
					XInternAtom(tempdisp,
						    "GHOSTVIEW_COLORS", false),
					XInternAtom(tempdisp, "STRING", false),
					8, PropModeReplace, 
					reinterpret_cast<unsigned char*>(tbuf),
					strlen(tbuf));
			XUngrabServer(tempdisp);
			XFlush(tempdisp);
			if (lyxerr.debugging()) {
				lyxerr << "Releasing the server" << endl;
			}
			XCloseDisplay(tempdisp);

			// set up environment
			while (environ[ne]) ++ne;
			env = static_cast<char **>
				(malloc(sizeof(char*) * (ne + 2)));
			env[0] = tbuf2;
			memcpy(&env[1], environ, sizeof(char*) * (ne + 1));
			environ = env;

			// now make gs command
			// close(0);
			// close(1); do NOT close. If GS writes out
			// errors it would hang. (Matthias 290596) 
			sprintf(rbuf, "-r%gx%g", p->rx, p->ry);
			sprintf(gbuf, "-g%dx%d", p->data->wid, p->data->hgh);
			// now chdir into dir with .eps file, to be on the safe
			// side
			chdir(OnlyPath(p->data->fname).c_str());
			// make temp file name
			sprintf(tbuf, "%s/~lyxgs%d.ps", system_tempdir.c_str(),
				int(getpid()));
			if (lyxerr.debugging()) {
				lyxerr << "starting gs " << tbuf << " "
				       << p->data->fname
				       << ", pid: " << getpid() << endl;
			}

			int err = execlp(lyxrc->ps_command.c_str(), 
					 lyxrc->ps_command.c_str(), 
					 "-sDEVICE=x11",
					 "-dNOPAUSE", "-dQUIET",
					 "-dSAFER", 
					 rbuf, gbuf, tbuf, 
					 p->data->fname.c_str(), 
					 "showpage.ps", "quit.ps", "-", 0);
			// if we are still there, an error occurred.
			lyxerr << "Error executing ghostscript. "
			       << "Code: " << err << endl;
			lyxerr.debug() << "Cmd: " 
				       << lyxrc->ps_command
				       << " -sDEVICE=x11 "
				       << tbuf << ' '
				       << p->data->fname << endl;
			_exit(0);	// no gs?
		}
		// normal process (parent)
		if (lyxerr.debugging()) {
			lyxerr << "GS ["  << pid << "] started" << endl;
		}
		gsqueue = gsqueue->next;
		gsrunning++;
		p->data->gspid = pid;
		delete p;
	}
}


static void addwait(int psx, int psy, int pswid, int pshgh, figdata * data)
{
	// recompute the stuff and put in the queue
	queue * p = new queue;
	p->ofsx = psx;
	p->ofsy = psy;
	p->rx = (float(data->raw_wid) * 72.0) / pswid;
	p->ry = (float(data->raw_hgh) * 72.0) / pshgh;

	p->data = data;
	p->next = 0;

	// now put into queue
	queue * p2 = gsqueue;
	if (!gsqueue) gsqueue = p;
	else {
		while (p2->next) p2 = p2->next;
		p2->next = p;
	}

	// if possible, run the queue
	runqueue();
}


static figdata * getfigdata(int wid, int hgh, string const & fname, 
			    int psx, int psy, int pswid, int pshgh, 
			    int raw_wid, int raw_hgh, float angle, char flags)
{
	/* first search for an exact match with fname and width/height */

	if (fname.empty()) return 0;

	int i = 0;
	while (i < bmpinsref) {
		if (bitmaps[i]->wid == wid && bitmaps[i]->hgh == hgh &&
		    bitmaps[i]->flags == flags && bitmaps[i]->fname == fname &&
		    bitmaps[i]->angle == angle) {
			bitmaps[i]->ref++;
			return bitmaps[i];
		}
		++i;
	}
	/* not found -> create new record or return 0 if no record */
	++bmpinsref;
	if (bmpinsref > bmparrsize) {
		// allocate more space
		bmparrsize += figallocchunk;
		figdata ** tmp = static_cast<figdata**>
			(malloc(sizeof(figdata*) * bmparrsize));
		memcpy(tmp, bitmaps,
		       sizeof(figdata*) * (bmparrsize - figallocchunk));
		free(bitmaps);
		bitmaps = tmp;
	}
	figdata * p = new figdata;
	bitmaps[bmpinsref-1] = p;
	p->wid = wid;
	p->hgh = hgh;
	p->raw_wid = raw_wid;
	p->raw_hgh = raw_hgh;
	p->angle = angle;
	p->fname = fname;
	p->flags = flags;
	XWindowAttributes wa;
	XGetWindowAttributes(fl_display, fl_get_canvas_id(
		figinset_canvas), &wa);

	if (lyxerr.debugging()) {
		lyxerr << "Create pixmap disp:" << fl_display
		       << " scr:" << DefaultScreen(fl_display)
		       << " w:" << wid
		       << " h:" << hgh
		       << " depth:" << wa.depth << endl;
	}
	
	p->ref = 1;
	p->reading = false;
	p->broken = false;
	p->gspid = -1;
	if (flags) {
		p->bitmap = XCreatePixmap(fl_display, fl_get_canvas_id(
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


static void getbitmap(figdata * p)
{
	p->gspid = -1;
}


static void makeupdatelist(figdata * p)
{
	for (int i = 0; i < figinsref; ++i)
		if (figures[i]->data == p) {
			if (lyxerr.debugging()) {
				lyxerr << "Updating inset "
				       << figures[i]->inset
				       << endl;
			}
			//UpdateInset(figures[i]->inset);
			// add inset figures[i]->inset into to_update list
			PutInsetIntoInsetUpdateList(figures[i]->inset);
		}
}


void sigchldchecker(pid_t pid, int * status)
{
	lyxerr.debug() << "Got pid = " << pid << endl;
	bool pid_handled = false;
	for (int i = bmpinsref - 1; i >= 0; --i) {
		if (bitmaps[i]->reading && pid == bitmaps[i]->gspid) {
			lyxerr.debug() << "Found pid in bitmaps" << endl;
			// now read the file and remove it from disk
			figdata * p = bitmaps[i];
			p->reading = false;
			if (bitmaps[i]->gsdone) *status = 0;
			if (*status == 0) {
				lyxerr.debug() << "GS [" << pid
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
				char tmp[128];
				sprintf(tmp, "%s/~lyxgs%d.ps", 
					system_tempdir.c_str(),
					p->gspid);
				unlink(tmp);
				p->gspid = -1;
				p->broken = true;
			}
			makeupdatelist(bitmaps[i]);
			gsrunning--;
			runqueue();
			pid_handled = true;
		}
	}
	if (!pid_handled) {
		lyxerr.debug() << "Checking pid in pidwait" << endl;
		pidwait * p = pw, * prev = 0;
		while (p) {
			if (pid == p->pid) {
				lyxerr.debug() << "Found pid in pidwait" << endl;
				lyxerr.debug() << "Caught child pid of recompute routine " << pid << endl;
				if (prev)
					prev->next = p->next;
				else
					pw = p->next;
				free(p);
				break;
			}
			prev = p;
			p = p->next;
		}
	}

	if (pid == -1) {
		lyxerr.debug() << "waitpid error" << endl;
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
		lyxerr.debug() << "normal exit from childhandler" << endl;
	}
}


static void getbitmaps()
{
	bitmap_waiting = false;
	for (int i = 0; i < bmpinsref; ++i)
		if (bitmaps[i]->gspid > 0 && !bitmaps[i]->reading)
			getbitmap(bitmaps[i]);
}


static void RegisterFigure(InsetFig *fi)
{
	if (figinsref == 0) InitFigures();
	fi->form = 0;
	++figinsref;
	if (figinsref > figarrsize) {
		// allocate more space
		figarrsize += figallocchunk;
		Figref ** tmp = static_cast<Figref**>
			(malloc(sizeof(Figref*)*figarrsize));
		memcpy(tmp, figures,
		       sizeof(Figref*)*(figarrsize-figallocchunk));
		free(figures);
		figures = tmp;
	}
	Figref * tmpfig = new Figref;
	tmpfig->data = 0;
	tmpfig->inset = fi;
	figures[figinsref-1] = tmpfig;
	fi->figure = tmpfig;

	if (lyxerr.debugging()) {
		lyxerr << "Register Figure: buffer:["
		       << current_view->buffer() << "]" << endl;
	}
}


int FindFigIndex(Figref * tmpfig)
{
	int i = 0;
	while (i < figinsref) {
		if (figures[i] == tmpfig) return i;
		++i;
	}
	return i;
}


static void UnregisterFigure(InsetFig * fi)
{
	Figref * tmpfig = fi->figure;

	if (tmpfig->data) freefigdata(tmpfig->data);
	if (tmpfig->inset->form) {
		if (tmpfig->inset->form->Figure->visible) {
			fl_set_focus_object(tmpfig->inset->form->Figure,
					    tmpfig->inset->form->OkBtn);
			fl_hide_form(tmpfig->inset->form->Figure);
		}
#if FL_REVISION == 89
#warning Reactivate this free_form calls
#else
		fl_free_form(tmpfig->inset->form->Figure);
		free(tmpfig->inset->form);
		tmpfig->inset->form = 0;
#endif
	}
	int i = FindFigIndex(tmpfig);
	--figinsref;
	while (i < figinsref) {
		figures[i] = figures[i+1];
		++i;
	}
	delete tmpfig;

	if (figinsref == 0) DoneFigures();
}


static char * NextToken(FILE * myfile)
{
	char * token = 0;
	char c;
	int i = 0;
   
	if (!feof(myfile)) {
		token = new char[256];
		do {
			c = fgetc(myfile);
			token[i++]= c;
		} while (!feof(myfile) && !isspace(c));
      
		token[i-1]= '\0';         /* just the end of a command  */
	}
	return token;
}


InsetFig::InsetFig(int tmpx, int tmpy, Buffer * o)
	: owner(o)
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
	raw_wid = raw_hgh = 0;
	changedfname = false;
	RegisterFigure(this);
}


InsetFig::~InsetFig()
{
	if (lyxerr.debugging()) {
		lyxerr << "Figure destructor called" << endl;
	}
	UnregisterFigure(this);
}


int InsetFig::Ascent(LyXFont const &) const
{
	return hgh + 3;
}


int InsetFig::Descent(LyXFont const &) const
{
	return 1;
}


int InsetFig::Width(LyXFont const &) const
{
	return wid + 2;
}


void InsetFig::Draw(LyXFont font, LyXScreen & scr, int baseline, float & x)
{
	if (bitmap_waiting) getbitmaps();

	// I wish that I didn't have to use this
	// but the figinset code is so complicated so
	// I don't want to fiddle with it now.
	unsigned long pm = scr.getForeground();
	
	if (figure && figure->data && figure->data->bitmap &&
	    !figure->data->reading && !figure->data->broken) {
		// draw the bitmap
		XCopyArea(fl_display, figure->data->bitmap, pm, getGC(gc_copy),
			  0, 0, wid, hgh, int(x+1), baseline-hgh);
		XFlush(fl_display);
		if (flags & 4) XDrawRectangle(fl_display, pm, getGC(gc_copy),
					      int(x), baseline - hgh - 1,
					      wid+1, hgh+1);
	} else {
		char * msg = 0;
		// draw frame
		XDrawRectangle(fl_display, pm, getGC(gc_copy),
			       int(x),
			       baseline - hgh - 1, wid+1, hgh+1);
		if (figure && figure->data) {
			if (figure->data->broken)  msg = _("[render error]");
			else if (figure->data->reading) msg = _("[rendering ... ]");
		} else 
			if (fname.empty()) msg = _("[no file]");
			else if ((flags & 3) == 0) msg = _("[not displayed]");
			else if (lyxrc->ps_command.empty()) msg = _("[no ghostscript]");

		if (!msg) msg = _("[unknown error]");
		
		font.setFamily (LyXFont::SANS_FAMILY);
		font.setSize (LyXFont::SIZE_FOOTNOTE);
		string justname = OnlyFilename (fname);
		font.drawString(justname, pm,
				baseline - font.maxAscent() - 4,
				int(x) + 8);
		font.setSize (LyXFont::SIZE_TINY);
		font.drawText (msg, strlen(msg), pm,
			       baseline - 4,
			       int(x) + 8);

	}
	x += Width(font);    // ?
}


void InsetFig::Write(ostream & os)
{
	Regenerate();
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


void InsetFig::Read(LyXLex & lex)
{
	string buf;
	bool finished = false;
	
	while (lex.IsOK() && !finished) {
		lex.next();

		string const token = lex.GetString();
		lyxerr.debug() << "Token: " << token << endl;
		
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
				lyxerr.debug() << "Unknown type!" << endl;
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
				lyxerr.debug() << "Unknown type!" << endl;
				break;
			}
			thtype = htype;
		}
	}
	Regenerate();
	Recompute();
}


int InsetFig::Latex(ostream & os, signed char /* fragile*/ )
{
	Regenerate();
	if (!cmd.empty()) os << cmd << " ";
	return 0;
}


int InsetFig::Latex(string & file, signed char /* fragile*/ )
{
	Regenerate();
	file += cmd + ' ';
	return 0;
}


int InsetFig::Linuxdoc(string &/*file*/)
{
	return 0;
}


int InsetFig::DocBook(string & file)
{
	string figurename = fname;

	if(suffixIs(figurename, ".eps"))
		figurename.erase(fname.length() - 5);

	file += "@<graphic fileref=\"" + figurename + "\"></graphic>";
	return 0;
}


void InsetFig::Validate(LaTeXFeatures & features) const
{
	features.graphics = true;
	if (subfigure) features.subfigure = true;
}


unsigned char InsetFig::Editable() const
{
	return 1;
}


bool InsetFig::Deletable() const
{
	return false;
}


void InsetFig::Edit(int, int)
{
	lyxerr.debug() << "Editing InsetFig." << endl;
	Regenerate();

	// We should have RO-versions of the form instead.
	// The actual prevention of altering a readonly doc
	// is done in CallbackFig()
	if(current_view->buffer()->isReadonly()) 
		WarnReadonly();

	if (!form) {
		form = create_form_Figure();
		fl_set_form_atclose(form->Figure, CancelCloseBoxCB, 0);
		fl_set_object_return(form->Angle, FL_RETURN_ALWAYS);
		fl_set_object_return(form->Width, FL_RETURN_ALWAYS);
		fl_set_object_return(form->Height, FL_RETURN_ALWAYS);
	}
	RestoreForm();
	if (form->Figure->visible) {
		fl_raise_form(form->Figure);
	} else {
		fl_show_form(form->Figure, FL_PLACE_MOUSE | FL_PLACE_SIZE,
			     FL_FULLBORDER, _("Figure"));
	}
}


Inset * InsetFig::Clone() const
{
	InsetFig * tmp = new InsetFig(100, 100, owner);

	if (lyxerr.debugging()) {
		lyxerr << "Clone Figure: buffer:["
		       << current_view->buffer()
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
	if (!fname.empty() && (flags & 3) && !lyxrc->ps_command.empty()) { 
		// do not display if there is
		// "do not display" chosen (Matthias 260696)
		tmp->figure->data = getfigdata(wid, hgh, fname, psx, psy,
					       pswid, pshgh, raw_wid, raw_hgh,
					       angle, flags & (3|8));
	} else tmp->figure->data = 0;
	tmp->subcaption = subcaption;
	tmp->changedfname = false;
	tmp->owner = owner;
	tmp->Regenerate();
	return tmp;
}


Inset::Code InsetFig::LyxCode() const
{
	return Inset::GRAPHICS_CODE;
}


void InsetFig::Regenerate()
{
	string cmdbuf;
	string resizeW, resizeH;
	string rotate, recmd;

	if (fname.empty()) {
		cmd = "\\fbox{\\rule[-0.5in]{0pt}{1in}";
	        cmd += _("empty figure path");
		cmd += '}';
		//if (form) fl_set_object_label(form->cmd, "");
		return;
	}

	string buf1 = OnlyPath(owner->fileName());
	string fname2 = MakeRelPath(fname, buf1);

	string gcmd = "\\includegraphics{" + fname2 + '}';
	
	switch (wtype) {
	case DEF:
		break;
	case CM:{// \resizebox*{h-length}{v-length}{text}
		char buf[10];
		sprintf(buf, "%g", xwid); // should find better
		resizeW = buf;
		resizeW += "cm";
		break;
	}
	case IN: {
		char buf[10];
		sprintf(buf, "%g", xwid);
		resizeW = buf;
		resizeW += "in";
		break;
	}
	case PER_PAGE:{
		char buf[10];
		sprintf(buf, "%g", xwid/100);
		resizeW = buf;
		resizeW += "\\textwidth";
		break;
	}
	case PER_COL:{
		char buf[10];
		sprintf(buf, "%g", xwid/100);
		resizeW = buf;
		resizeW += "\\columnwidth";
		break;
	}
	}

	switch (htype) {
	case DEF:
		break;
	case CM: {
		char buf[10];
		sprintf(buf, "%g", xhgh);
		resizeH = buf;
		resizeH += "cm";
		break;
	}
	case IN:{
		char buf[10];
		sprintf(buf, "%g", xhgh);
		resizeH = buf;
		resizeH += "in";
		break;
	}
	case PER_PAGE: {
		char buf[10];
		sprintf(buf, "%g", xhgh/100);
		resizeH = buf;
		resizeH += "\\textheight";
		break;
	}
	case PER_COL: {
                // Doesn't occur; case exists to suppress compiler warnings.
	        break;
	}
	}

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
		char buf[10];
		sprintf(buf, "%g", angle);
		// \rotatebox{angle}{text}
		rotate = "\\rotatebox{";
		rotate += buf;
		rotate += "}{";
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

	//if (form) fl_set_object_label(form->cmd, cmd.c_str());
}


void InsetFig::TempRegenerate()
{
	string cmdbuf;
	string resizeW, resizeH;
	string rotate, recmd;
	
	char const * tfname = fl_get_input(form->EpsFile);
	string tsubcap = fl_get_input(form->Subcaption);
	float tangle = atof(fl_get_input(form->Angle));
	float txwid = atof(fl_get_input(form->Width));
	float txhgh = atof(fl_get_input(form->Height));

	if (!tfname || !*tfname) {
		//fl_set_object_label(form->cmd, "");
		//fl_redraw_object(form->cmd);
	        cmd = "\\fbox{\\rule[-0.5in]{0pt}{1in}";
	        cmd += _("empty figure path");
		cmd += '}';
		return;
	}

	string buf1 = OnlyPath(owner->fileName());
	string fname2 = MakeRelPath(tfname, buf1);
	// \includegraphics*[<llx,lly>][<urx,ury>]{file}
	string gcmd = "\\includegraphics{" + fname2 + '}';
	
	switch (twtype) {
	case DEF:
		break;
	case CM: {// \resizebox*{h-length}{v-length}{text}
		char buf[10];
		sprintf(buf, "%g", txwid); // should find better
		resizeW = buf;
		resizeW += "cm";
		break;
	}
	case IN: {
		char buf[10];
		sprintf(buf, "%g", txwid);
		resizeW = buf;
		resizeW += "in";
		break;
	}
	case PER_PAGE: {
		char buf[10];
		sprintf(buf, "%g", txwid/100);
		resizeW = buf;
		resizeW += "\\textwidth";
		break;
	}
	case PER_COL: {
		char buf[10];
		sprintf(buf, "%g", txwid/100);
		resizeW = buf;
		resizeW += "\\columnwidth";
		break;
	}
	}

	switch (thtype) {
	case DEF:
		break;
	case CM: {
		char buf[10];
		sprintf(buf, "%g", txhgh);
		resizeH = buf;
		resizeH += "cm";
		break;
	}
	case IN: {
		char buf[10];
		sprintf(buf, "%g", txhgh);
		resizeH = buf;
		resizeH += "in";
		break;
	}
	case PER_PAGE: {
		char buf[10];
		sprintf(buf, "%g", txhgh/100);
		resizeH = buf;
		resizeH += "\\textheight";
		break;
	}
	case PER_COL: {
                // Doesn't occur; case exists to suppress compiler warnings.
	        break;
	}
	}

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
		char buf[10];
		sprintf(buf, "%g", tangle);
		// \rotatebox{angle}{text}
		rotate = "\\rotatebox{";
		rotate += buf;
		rotate += "}{";
	}

	cmdbuf = recmd;
	cmdbuf += rotate;
	cmdbuf += gcmd;
	if (!rotate.empty()) cmdbuf += '}';
	if (!recmd.empty()) cmdbuf += '}';
	if (psubfigure && !tsubcap.empty()) {
		cmdbuf = string("\\subfigure{") + tsubcap
			+ string("}{") + cmdbuf + "}";
	}
}


void InsetFig::Recompute()
{
	bool changed = changedfname;
	int newx, newy, nraw_x, nraw_y;

	if (changed) GetPSSizes();

	float sin_a = sin (angle / DEG2PI);  /* rotation; H. Zeller 021296 */
	float cos_a = cos (angle / DEG2PI);
	int frame_wid = int(ceil(fabs(cos_a * pswid) + fabs(sin_a * pshgh)));
	int frame_hgh= int(ceil(fabs(cos_a * pshgh) + fabs(sin_a * pswid)));

	/* now recompute wid and hgh, and if that is changed, set changed */
	/* this depends on chosen size of the picture and its bbox */
	// This will be redone in 0.13 ... (hen)
	if (!fname.empty()) {
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
		if (!fname.empty() && (flags & 3)
		    && !lyxrc->ps_command.empty()) {
			// do not display if there is "do not display"
			// chosen (Matthias 260696)
			figure->data = getfigdata(wid, hgh, fname,
						  psx, psy, pswid, pshgh,
						  raw_wid, raw_hgh,
						  angle, flags & (3|8));
		} else figure->data = 0;

		// free the old data
		if (pf) freefigdata(pf);
	}

	changedfname = false;
}


void InsetFig::GetPSSizes()
{
#ifdef WITH_WARNINGS
#warning rewrite this method to use ifstream
#endif
	/* get %%BoundingBox: from postscript file */
	char * p = 0;
	
	/* defaults to associated size
	 * ..just in case the PS-file is not readable (Henner, 24-Aug-97) 
	 */
	psx = 0;
	psy = 0;
	pswid = wid;
	pshgh = hgh;

	if (fname.empty()) return;

	FilePtr f(fname, FilePtr::read);

	if (!f()) return;	// file not found !!!!

	/* defaults to A4 page */
	psx = 0;
	psy = 0;
	pswid = 595;
	pshgh = 842;

	int lastchar = fgetc(f);
	for (;;) {
		int c = fgetc(f);
		if (c == EOF) {
			lyxerr.debug() << "End of (E)PS file reached and"
				" no BoundingBox!" << endl;
			break;
		}
		if (c == '%' && lastchar == '%') {
			p = NextToken(f);
			if (!p) break;
			// we should not use this, with it we cannot
			// discover bounding box and end of file.
			//if (strcmp(p, "EndComments") == 0) break;
			if (strcmp(p, "BoundingBox:") == 0) {
				float fpsx, fpsy, fpswid, fpshgh;
				if (fscanf(f, "%f %f %f %f", &fpsx, &fpsy,
					   &fpswid, &fpshgh) == 4) {
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
			delete[] p;
			p = 0;
		}
		lastchar = c;
	}
	if (p) delete[] p;
	pswid -= psx;
	pshgh -= psy;

}


void InsetFig::CallbackFig(long arg)
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
			lyxerr.debug() << "Unknown type!" << endl;
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
			lyxerr.debug() << "Unknown type!" << endl;
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
		BrowseFile();
		regen = true;
		break;
	case 1:				/* preview */
		p = fl_get_input(form->EpsFile);
		Preview(p);
		break;
	case 7:				/* apply */
	case 8:				/* ok (apply and close) */
		if(!current_view->buffer()->isReadonly()) {
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
					fname.clear();
				}
			}
			subcaption = fl_get_input(form->Subcaption);
	
			Regenerate();
			Recompute();
			/* now update inset */
			if (lyxerr.debugging()) {
				lyxerr << "Update: ["
				       << wid << 'x' << hgh << ']' << endl;
			}
			UpdateInset(this);
			if (arg == 8) {
				fl_set_focus_object(form->Figure, form->OkBtn);
				fl_hide_form(form->Figure);
#if FL_REVISION == 89
#warning Reactivate this free_form calls
#else
				fl_free_form(form->Figure);
				free(form);
				form = 0;
#endif
			}
			break;
		} //if not readonly
		//  The user has already been informed about RO in ::Edit
		if(arg == 7) // if 'Apply'
			break;
		// fall through
	case 9:				/* cancel = restore and close */
		fl_set_focus_object(form->Figure, form->OkBtn);
		fl_hide_form(form->Figure);
#if FL_REVISION == 89
#warning Reactivate this free_form calls
#warning Jug, is this still a problem?
#else
		fl_free_form(form->Figure);
		free(form);
		form = 0;
#endif
		break;
	}

	if (regen) TempRegenerate();
}


inline void DisableFigurePanel(FD_Figure * const form)
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


inline void EnableFigurePanel(FD_Figure * const form)
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


void InsetFig::RestoreForm()
{
	char buf[32];

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

	int pflags = flags & 3;
	fl_set_button(form->Wysiwyg0, (pflags == 0));
	fl_set_button(form->Wysiwyg1, (pflags == 1));
	fl_set_button(form->Wysiwyg2, (pflags == 2));
	fl_set_button(form->Wysiwyg3, (pflags == 3));
	fl_set_button(form->Frame, ((flags & 4) != 0));
	fl_set_button(form->Translations, ((flags & 8) != 0));
	fl_set_button(form->Subfigure, (subfigure != 0));
	pflags = flags;
	psubfigure = subfigure;
	sprintf(buf, "%g", xwid);
	fl_set_input(form->Width, buf);
	sprintf(buf, "%g", xhgh);
	fl_set_input(form->Height, buf);
	sprintf(buf, "%g", angle);
	fl_set_input(form->Angle, buf);
	if (!fname.empty()){
		string buf1 = OnlyPath(owner->fileName());
		string fname2 = MakeRelPath(fname, buf1);
		fl_set_input(form->EpsFile, fname2.c_str());
	}
	else fl_set_input(form->EpsFile, "");
	fl_set_input(form->Subcaption, subcaption.c_str());
	if(current_view->buffer()->isReadonly()) 
	        DisableFigurePanel(form);

	TempRegenerate();
}


void InsetFig::Preview(char const * p)
{
 	int pid = fork();

  	if (pid == -1) {
  		lyxerr << "Cannot fork process!" << endl;
  		return;		// error
  	}
  	if (pid > 0) {
  		addpidwait(pid);
  		return;		// parent process
  	}

	string buf1 = OnlyPath(owner->fileName());
	string buf2 = MakeAbsPath(p, buf1);
	
	lyxerr << "Error during rendering "
	       << execlp(lyxrc->view_pspic_command.c_str(),
			 lyxrc->view_pspic_command.c_str(),
			 buf2.c_str(), 0)
	       << endl;
	_exit(0);
}


void InsetFig::BrowseFile()
{
	static string current_figure_path;
	static int once = 0;
	LyXFileDlg fileDlg;

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
		buf = OnlyPath(owner->fileName().c_str());
	}
	
	// Does user clipart directory exist?
	string bufclip = AddName (user_lyxdir, "clipart");	
	FileInfo fileInfo(bufclip);
	if (!(fileInfo.isOK() && fileInfo.isDir()))
		// No - bail out to system clipart directory
		bufclip = AddName (system_lyxdir, "clipart");	


	fileDlg.SetButton(0, _("Clipart"), bufclip); 
	fileDlg.SetButton(1, _("Document"), buf); 

	bool error = false;
	do {
		ProhibitInput();
		if (once) {
			p = fileDlg.Select(_("EPS Figure"),
					   current_figure_path,
					   "*ps", string());
		} else {
			p = fileDlg.Select(_("EPS Figure"), buf,
					   "*ps", string());
		}
		AllowInput();

		if (p.empty()) return;

		buf = MakeRelPath(p, buf2);
		current_figure_path = OnlyPath(p);
		once = 1;
		
		if (contains(p, "#") || contains(p, "~") || contains(p, "$")
		    || contains(p, "%") || contains(p, " ")) 
			{
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
	for (int i = 0; i < figinsref; ++i)
		if (figures[i]->inset->form && figures[i]->inset->form->Figure
		    == obj->form) {
	    
			if (lyxerr.debugging()) {
				lyxerr << "Calling back figure " << i << endl;
			}
			figures[i]->inset->CallbackFig(arg);
			return;
		}
}


void HideFiguresPopups()
{
	for (int i = 0; i < figinsref; ++i)
		if (figures[i]->inset->form 
		    && figures[i]->inset->form->Figure->visible) {
			if (lyxerr.debugging()) {
				lyxerr << "Hiding figure " << i << endl;
			}
			// hide and free the form
			figures[i]->inset->CallbackFig(9);
		}
}
