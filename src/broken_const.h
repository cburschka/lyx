/* -*- C++ -*- */
/* This file is part of
* ======================================================
* 
*           LyX, the High Level Word Processor
*        
*           Copyright (C) 1995,1996 Matthias Ettrich
*
*======================================================
 broken_const.h -- inhibits the `const' keyword for compilation */

#ifdef const
#undef const
#endif /* const */
#define const
#ifdef __CONSTVALUE2
#undef __CONSTVALUE2
#define	__CONSTVALUE2	/* We don't want empty __attribute__ (()). */
#endif /* __CONSTVALUE2 */

