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

/// creates inset according to 'cmd'
Inset * createInset(FuncRequest const & cmd);

#endif

