#ifndef _PORTABLE_SNPRINTF_H_
#define _PORTABLE_SNPRINTF_H_

#define PORTABLE_SNPRINTF_VERSION_MAJOR 2
#define PORTABLE_SNPRINTF_VERSION_MINOR 1

#ifdef __cplusplus
extern "C" {
#endif
  
#if defined(HAVE_DECL_SNPRINTF) || defined(HAVE_DECL_VSNPRINTF)
#include <stdio.h>
#endif
#ifndef HAVE_DECL_SNPRINTF
int snprintf(char *, size_t, const char *, /*args*/ ...);
#endif
#ifndef HAVE_DECL_VSNPRINTF
int vsnprintf(char *, size_t, const char *, va_list);
#endif

#if defined(HAVE_SNPRINTF) && defined(PREFER_PORTABLE_SNPRINTF)
int portable_snprintf(char *str, size_t str_m, const char *fmt, /*args*/ ...);
int portable_vsnprintf(char *str, size_t str_m, const char *fmt, va_list ap);
#define snprintf  portable_snprintf
#define vsnprintf portable_vsnprintf
#endif

int asprintf  (char **ptr, const char *fmt, /*args*/ ...);
int vasprintf (char **ptr, const char *fmt, va_list ap);
int asnprintf (char **ptr, size_t str_m, const char *fmt, /*args*/ ...);
int vasnprintf(char **ptr, size_t str_m, const char *fmt, va_list ap);

#ifdef __cplusplus
} /* end of extern "C" */
#endif
  
#endif
