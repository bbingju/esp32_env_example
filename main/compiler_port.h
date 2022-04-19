#pragma once

#ifdef __cplusplus
# define APP_C__BEGIN_DECLS	extern "C" {
# define APP_C__END_DECLS		}
#else
# define APP_C__BEGIN_DECLS
# define APP_C__END_DECLS
#endif

#ifdef __GNUC__
#define _WEAK    __attribute__((weak))
#define _UNUSED  __attribute__((unused))
#else
#define _WEAK
#define _UNUSED
#endif
