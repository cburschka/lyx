// -*- C++ -*-
/**
 * \file factory.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FACTORY_H
#define FACTORY_H

class Inset;
class FuncRequest;
class LyXLex;
class Buffer;

/// creates inset according to 'cmd'
Inset * createInset(FuncRequest const & cmd);

/// read inset from a file
Inset * readInset(LyXLex & lex, Buffer const & buf);

#endif // FACTORY_H
