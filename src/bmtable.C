/*
 *  File:        bmtable.c
 *  Purpose:     Implementation of the XForms object bmtable. 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     November 1995
 *  Description: A bitmap table uses a single bitmap to simulate a 2d array 
 *               of bitmap buttons. It can be used to build bitmap menus.
 *               
 *  Copyright 1995,1996 Alejandro Aguilar Sierra 
 *
 *  You are free to use and modify this code under the terms of
 *  the GNU General Public Licence version 2 or later. 
 *  
 */ 

#include <config.h>

#include <cstdlib>
#include "bmtable.h"
#include XPM_H_LOCATION

typedef struct   {   
   int nx, ny;   /* Dimensions of the table */
   int dx, dy;   /* Size of each item */ 
   int bx, by;   /* Bitmap's position */
   int bw, bh;   /* Bitmap dimensions */
   unsigned char * bdata;  /* Bitmap data */
   int maxi;     /* Number of items */
   int i;        /* Current position */
   int mousebut; /* mouse button pushed */  
   Pixmap pix;   /* Pixmap from data (temporal) */
} BMTABLE_SPEC;
                 

extern "C" int handle_bitmaptable(FL_OBJECT *ob, int event, FL_Coord mx, 
				  FL_Coord my, int key, void *xev);


FL_OBJECT *fl_create_bmtable(int type, FL_Coord x, FL_Coord y, 
				 FL_Coord w, FL_Coord h, char const *label)
{
   FL_OBJECT *ob;
   
   ob = fl_make_object(FL_BMTABLE, type, x, y, w, h, label, handle_bitmaptable);
   ob->boxtype = FL_BMTABLE_BOXTYPE;
   ob->spec = fl_calloc(1, sizeof(BMTABLE_SPEC));
   ((BMTABLE_SPEC *)ob->spec)->pix = 0;
   ((BMTABLE_SPEC *)ob->spec)->bdata= 0;
   ((BMTABLE_SPEC *)ob->spec)->mousebut= -1;
   return ob;
}


FL_OBJECT *fl_add_bmtable(int type, FL_Coord x, FL_Coord y, 
			      FL_Coord w, FL_Coord h, char const *label)
{
   FL_OBJECT *ob;
   
   ob = fl_create_bmtable(type, x, y, w,h, label);  
   fl_add_object(fl_current_form, ob); 
   
   return ob;
}


static void draw_bitmaptable(FL_OBJECT *ob)
{
	int i, j, lx;
	FL_Coord mx, my;
	FL_Coord xx, yy, ww, hh;
	BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;
	if (!sp) return;
	GC gc = fl_state[fl_get_vclass()].gc[0];
   
	/* draw the bounding box first */
	lx = sp->maxi % sp->nx;
	fl_drw_box(ob->boxtype, ob->x, ob->y, ob->w, ob->h, ob->col1, ob->bw);
	if (lx) {
		i = FL_abs(ob->bw);
		xx = ob->x+ sp->dx*lx + i;
		yy = ob->y+ (sp->ny-1)*sp->dy+i;
		ww = ob->x+ob->w - xx - i;
		hh = ob->y+ob->h-yy-i;
		fl_drw_frame(FL_DOWN_FRAME, xx, yy, ww, hh, ob->col1, ob->bw);
		fl_rectf(xx, yy, ww+i, hh+i, ob->col1);
	}
    
	/* draw the background bitmap */
	if (sp->bdata)  {
		if (!sp->pix) {
			sp->pix =  XCreatePixmapFromBitmapData(fl_display, fl_winget(), 
					reinterpret_cast<char*>(sp->bdata),
							       sp->bw, sp->bh,
					fl_get_flcolor(ob->lcol), fl_get_flcolor(ob->col1),
					DefaultDepth(fl_display, DefaultScreen(fl_display)));
			XFlush(fl_display);
		}
	}
	if (sp->pix) {
		/* Adjust position */ 
		if (sp->bx < FL_abs(ob->bw) + 1) {
			xx = FL_abs(ob->bw) - sp->bx + 1;
			mx = ob->x + FL_abs(ob->bw) + 1;
		} else  {
			xx =0;
			mx = ob->x + sp->bx;
		}
		if (sp->by < FL_abs(ob->bw) + 1)  {	 
			yy = FL_abs(ob->bw) - sp->by + 1;
			my = ob->y + FL_abs(ob->bw) + 1;
		} else   {
			yy =0;
			my = ob->y + sp->by;
		}                 
		ww = (mx + sp->bw < ob->x + ob->w - FL_abs(ob->bw)) ? 
			sp->bw: ob->x + ob->w - FL_abs(ob->bw) - mx;
		hh = (my + sp->bh < ob->y + ob->h - FL_abs(ob->bw)) ?
			sp->bh: ob->y + ob->h - FL_abs(ob->bw) - my; 
      
		i = FL_abs(ob->bw);
		j = hh - ((lx) ? sp->dy+2*i: 0);
		XCopyArea(fl_display, sp->pix, fl_winget(), gc, xx, yy, ww, j, mx, my);
		XFlush(fl_display);
		if (lx) {
			XCopyArea(fl_display, sp->pix, fl_winget(), gc, xx,
					  yy+j, lx*sp->dx-2*i, hh-j, mx, my+j);
			XFlush(fl_display);
		}
	}
   
   
	/* draw the grid if type > FLAT */
	if (ob->type > FL_BMTABLE_FLAT)  {
		mx = ob->x + ob->w; 
		my = ob->y + ob->h; 
		ww = ob->w; 
		for (yy=ob->y; yy<=my; yy+=sp->dy) {
			if (ob->boxtype!=FL_FLAT_BOX && (yy==ob->y || yy>my-sp->dy)) 
				continue;
			if (lx>0 && yy>=my-sp->dy - sp->dy/2)
				ww = lx*sp->dx;
			fl_diagline(ob->x, yy, ww, 1, FL_BOTTOM_BCOL); 
			fl_diagline(ob->x, yy+1, ww-2, 1, FL_TOP_BCOL); 
		}	   
		hh = ob->h;
		for (xx=ob->x; xx<=mx; xx+=sp->dx)  {
			if (ob->boxtype!=FL_FLAT_BOX && (xx==ob->x || xx>mx-sp->dx))
				continue;
			if (lx>0 && xx>=ob->x+lx*sp->dx)
				hh = (sp->ny-1)*sp->dy;
			fl_diagline(xx, ob->y, 1, hh, FL_RIGHT_BCOL);
			fl_diagline(xx+1, ob->y+1, 1, hh-2, FL_LEFT_BCOL);
		}	 
	}  
   
	/* Simulate a pushed button */
	if (ob->pushed && 0 <= sp->i && sp->i < sp->maxi)  {  
		i = sp->i % sp->nx;
		j = sp->i/sp->nx;
		ww = sp->dx-2*FL_abs(ob->bw);
		hh = sp->dy-2*FL_abs(ob->bw);
		xx = ob->x + sp->dx*i + FL_abs(ob->bw);
		yy = ob->y + sp->dy*j + FL_abs(ob->bw);
		fl_drw_frame(FL_DOWN_FRAME, xx, yy, ww, hh, ob->col1, ob->bw);
	}
}


extern "C" int handle_bitmaptable(FL_OBJECT *ob, int event, FL_Coord mx, 
				  FL_Coord my, int key, void */*xev*/)
{
	int i, j;
	BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;
   
	switch (event)  {
    case FL_DRAW: 
		draw_bitmaptable(ob);
		break;
    case FL_MOUSE:
		if (!ob->belowmouse) {    /* This never happens. Why? */
			sp->i = -1;
			fl_redraw_object(ob);
			break;
		}
		i = (mx - ob->x)/sp->dx;  j = (my - ob->y)/sp->dy;
		if (i>=0 && i< sp->nx && j>=0 && j< sp->ny)   {
			i += j*sp->nx;
			if (i >= sp->maxi) i = -1;
			if (sp->i !=  i)  {
				sp->i = i;
				fl_redraw_object(ob);
			}
		}
		break;        
    case FL_PUSH:
		sp->mousebut = key;
		i = (mx - ob->x)/sp->dx + ((my - ob->y)/sp->dy)*sp->nx; 
		if (0 <= i && i < sp->maxi)  {
			sp->i =  i;
			fl_redraw_object(ob);
		} else
			sp->i =  -1; 
		break;
    case FL_RELEASE:    
		fl_redraw_object(ob);
		return 1;
    case FL_FREEMEM:
	    if (sp->pix) {
		    XFreePixmap(fl_display, sp->pix);
		    XFlush(fl_display);
	    }
		fl_free(((BMTABLE_SPEC*)ob->spec));      
		break;
	}
	return 0;
}


/*
 * The table has nx columns of dx width each and ny rows of dy height each. 
 * Initially the position of the firts item is supposed to be the same that
 * the object position (x, y), and the number of items is supposed to be
 * exactly nx*ny.
 * 
 * The user could change these later. See below.
 */ 
void fl_set_bmtable_data(FL_OBJECT *ob, int nx, int ny, int bw, int bh, 
			unsigned char * bdata)
{
   BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;
   if (sp) {
     sp->nx = nx;
     sp->ny = ny; 
     sp->bx = FL_abs(ob->bw);
     sp->by = FL_abs(ob->bw);
     sp->dx = ob->w/nx; 
     sp->dy = ob->h/ny;
     sp->i = -1;
     sp->maxi = sp->nx * sp->ny;
     sp->bw = bw;
     sp->bh = bh;
     sp->bdata = bdata;
   }
}


void fl_set_bmtable_pixmap_data(FL_OBJECT *ob, int nx, int ny,
			char **pdata)
{
	BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;
	extern Colormap color_map;
	if (sp) {
		sp->nx = nx;
		sp->ny = ny; 
		sp->bx = FL_abs(ob->bw);
		sp->by = FL_abs(ob->bw);
		sp->dx = ob->w/nx; 
		sp->dy = ob->h/ny;
		sp->i = -1;
		sp->maxi = sp->nx * sp->ny;
		sp->bdata = 0;
		Pixmap dummy_shapemask = 0;
		XpmAttributes dumb_attributes= {0};
		dumb_attributes.colormap = color_map;
		dumb_attributes.closeness = 30000;
		dumb_attributes.valuemask = XpmColormap | XpmCloseness;
		if (XCreatePixmapFromData(fl_display, fl_winget(), pdata,
		                          &(sp->pix), &dummy_shapemask,
	        	                  &dumb_attributes) == XpmSuccess) {
			sp->bw = dumb_attributes.width;
			sp->bh = dumb_attributes.height;
			XpmFreeAttributes(&dumb_attributes);
			if (dummy_shapemask) {
				XFreePixmap(fl_display, dummy_shapemask);
			}
		}
		XFlush(fl_display);
	}
}


/*
 *  This function works only for X11R6 or later
 */
#if XlibSpecificationRelease > 5 

void fl_set_bmtable_file(FL_OBJECT *ob, int nx, int ny, char const *filename)
{	
   int xh, yh;
   unsigned int bw, bh;
   unsigned char *bdata;
   
   if(XReadBitmapFileData(filename, &bw, &bh,
			  &bdata, &xh, &yh)==BitmapSuccess)
     fl_set_bmtable_data(ob, nx, ny, bw, bh, bdata);
   XFlush(fl_display);
}

#else

void fl_set_bmtable_file(FL_OBJECT *, int, int, char const *) 
{
  fprintf(stderr, "Set bmtable file: Sorry, I need X11 release 6 to do " 
	   "work!\n");
}

#endif



void fl_set_bmtable_pixmap_file(FL_OBJECT *ob, int nx, int ny, char const *filename)
{	
	extern Colormap color_map;
	BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;
	if (sp) {
		sp->nx = nx;
		sp->ny = ny; 
		sp->bx = FL_abs(ob->bw);
		sp->by = FL_abs(ob->bw);
		sp->dx = ob->w/nx; 
		sp->dy = ob->h/ny;
		sp->i = -1;
		sp->maxi = sp->nx * sp->ny;
		sp->bdata = 0;

		Pixmap dummy_shapemask = 0;
		XpmAttributes dumb_attributes= {0};
		dumb_attributes.colormap = color_map;
		dumb_attributes.closeness = 30000;
		dumb_attributes.valuemask = XpmColormap | XpmCloseness;
   
		if (XReadPixmapFile(fl_display, fl_winget(), (char *)filename,
		                    &(sp->pix), &dummy_shapemask,
	        	            &dumb_attributes) == XpmSuccess) {
			sp->bw = dumb_attributes.width;
			sp->bh = dumb_attributes.height;
			XpmFreeAttributes(&dumb_attributes);
			if (dummy_shapemask) {
				XFreePixmap(fl_display, dummy_shapemask);
			}
		}
		XFlush(fl_display);
	}
}


/*
 * This function allows to adjust the position of the first item and its
 * size (dx, dy). The input values are incremental, not absolute.
 */
void fl_set_bmtable_adjust(FL_OBJECT *ob, int px, int py, int dx, int dy)
{
   BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;   
   if (sp) {
     sp->bx += px;
     sp->by += py;                         
     sp->dx += dx;
     sp->dy += dy;
   }
}

/* 
 * This function returns the table's selected position.
 */
int fl_get_bmtable(FL_OBJECT *ob)
{ 
   if ((BMTABLE_SPEC *)ob->spec)
     return  ((BMTABLE_SPEC *)ob->spec)->i;
   else 
     return 0;
}


/* 
 * You can change the max number of items if you want.
 */
void fl_set_bmtable_maxitems(FL_OBJECT *ob, int i)
{
   if (i>0 && (BMTABLE_SPEC *)ob->spec)
     ((BMTABLE_SPEC *)ob->spec)->maxi = i;
}   


int fl_get_bmtable_maxitems(FL_OBJECT *ob)
{
   if ((BMTABLE_SPEC *)ob->spec)
     return  ((BMTABLE_SPEC *)ob->spec)->maxi;
   else
     return 0;
}


void fl_replace_bmtable_item(FL_OBJECT */*ob*/, int /*id*/, int  /*cw*/, int /*ch*/, char */*data*/)
{
   fprintf(stderr, "Replace bmtable item: Sorry, not yet implemented!\n");
}


void fl_get_bmtable_item(FL_OBJECT */*ob*/, int /*id*/, int */*cw*/, int */*ch*/, char */*data*/)
{
   fprintf(stderr, "Get bmtable item: Sorry, not yet implemented!\n");
}  

void fl_set_bmtable(FL_OBJECT *ob, int pushed, int pos)
{
   if ((BMTABLE_SPEC *)ob->spec)
     ((BMTABLE_SPEC *)ob->spec)->i = (pushed) ? pos: -1;
}


int fl_get_bmtable_numb(FL_OBJECT *ob)
{
   if ((BMTABLE_SPEC *)ob->spec)
     return ((BMTABLE_SPEC *)ob->spec)->mousebut;
   else
     return 0;
}


Pixmap fl_get_bmtable_pixmap(FL_OBJECT *ob)
{
   if ((BMTABLE_SPEC *)ob->spec)
     return ((BMTABLE_SPEC *)ob->spec)->pix;
   else
     return 0;
}


void fl_draw_bmtable_item(FL_OBJECT *ob, int i, Drawable d, int xx, int yy)
{
   int x, y, w, h;
   GC gc = fl_state[fl_get_vclass()].gc[0];
   BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;
   
   if (sp && sp->pix) {
      x = (i % sp->nx)*sp->dx + FL_abs(ob->bw);
      y = (i/sp->nx)*sp->dy + FL_abs(ob->bw);
      w = sp->dx-2*FL_abs(ob->bw);
      h = sp->dy-2*FL_abs(ob->bw);	 
      XCopyArea(fl_display, sp->pix, d, gc, x, y, w, h, xx, yy);
      XFlush(fl_display);
   }
}

/* Free the current bitmap and pixmap in preparation for installing a new one */
void fl_free_bmtable_bitmap(FL_OBJECT *ob)
{
  BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;

  /* dump the temporary pixmap */
  if (sp && sp->pix) { 
    XFreePixmap(fl_display, sp->pix);
    XFlush(fl_display);
    sp->pix = 0;
  }

  /* and free the space taken by bdata etc. */
  if (sp && sp->bdata) {
    fl_free(sp->bdata);
    sp->bdata = 0;
  }
}

/* Free the current pixmap in preparation for installing a new one */
/* This is needed when using data instead of files to set bitmaps  */
void fl_free_bmtable_pixmap(FL_OBJECT *ob)
{
  BMTABLE_SPEC *sp = (BMTABLE_SPEC *)ob->spec;

  /* dump the temporary pixmap */
  if (sp && sp->pix) { 
    XFreePixmap(fl_display, sp->pix);
    XFlush(fl_display);
    sp->pix = 0;
  }
}


