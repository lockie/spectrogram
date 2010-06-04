
#ifndef CONFIG_H
#define CONFIG_H

#ifndef _WIN32
# define _TCHAR char
# define _T(x) x
#else  // _WIN32
# if _MSC_VER >= 1400
#  define _CRT_SECURE_NO_WARNINGS
# endif  // _MSC_VER >= 1400
# include <tchar.h>
# include <windows.h>
#endif  // _WIN32

#endif  // CONFIG_H
