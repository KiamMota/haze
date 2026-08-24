#ifndef MACROS_H
#define MACROS_H

#define PTR_FREE_ASSERT(x) \
    if (!(x) || !*(x)) return;


#ifdef __cplusplus

#define EXPORT_CPP_BEGIN extern "C" {
#define EXPORT_CPP_END   }

#else

#define EXPORT_CPP_BEGIN
#define EXPORT_CPP_END

#endif

#endif
