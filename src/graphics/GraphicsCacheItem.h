// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#ifndef GRAPHICSCACHEITEM_H
#define GRAPHICSCACHEITEM_H

#ifdef __GNUG__
#pragma interface
#endif

class GraphicsCacheItem {
public:
    ~GraphicsCacheItem() {}
    
private:
    GraphicsCacheItem() {}

    friend class GraphicsCache;
};

#endif
