// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2002 The LyX Team.
 *
 * ====================================================== */

#ifndef FACTORY_H
#define FACTORY_H

class Inset;
class FuncRequest;

/// creates inset according to 'cmd'
Inset * createInset(FuncRequest const & cmd);

#endif

