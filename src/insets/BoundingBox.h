// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 the LyX Team.
 *
 * ====================================================== */

#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

///
struct BoundingBox {
	///
	BoundingBox() 
		: llx(-1), lly(-1), urx(-1), ury(-1) {}
	///
	bool isSet() const {
		return llx != -1 && lly != - 1
			&& urx != -1 && ury != -1;
	}
	///
	int llx;
	///
	int lly;
	///
	int urx;
	//
	int ury;
};

#endif
