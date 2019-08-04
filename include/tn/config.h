#ifndef TN_CONFIG_H
#define TN_CONFIG_H

#if _MSC_VER
#    ifdef _DEBUG
#        define TN_BUILD_DEBUG
#    endif
#elif defined(TN_CMAKE_BUILD_DEBUG)
#    define TN_BUILD_DEBUG
#endif

#endif
