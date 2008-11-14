// -*- C++ -*-
/**
 * \file factory.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FACTORY_H
#define FACTORY_H

namespace lyx {

class Buffer;
class FuncRequest;
class Inset;
class Lexer;


/// creates inset according to 'cmd'
Inset * createInset(Buffer & buf, FuncRequest const & cmd);

/// read inset from a file
Inset * readInset(Lexer & lex, Buffer const & buf);


} // namespace lyx

#endif // FACTORY_H
