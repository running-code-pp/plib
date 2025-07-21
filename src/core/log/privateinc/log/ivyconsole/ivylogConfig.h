#ifndef IVY_LOG_CONFIG_H
#define IVY_LOG_CONFIG_H

//**************************************************************************
// switching the operating systems

// First check for *WIN64* since the *WIN32* are also set on 64-bit platforms
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#ifndef IVY_OS_WIN32
#define IVY_OS_WIN32
#endif
#ifndef IVY_OS_WIN64
#define IVY_OS_WIN64
#endif
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#ifndef IVY_OS_WIN32
#define IVY_OS_WIN32
#endif
#if defined(__MINGW32__)
#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#endif
#elif defined(__MWERKS__) && defined(__INTEL__)
#ifndef IVY_OS_WIN32
#define IVY_OS_WIN32
#endif
#elif defined(__APPLE__)
#ifndef IVY_OS_MACOSX
#define IVY_OS_MACOSX
#endif
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__GLIBC__)
#ifndef IVY_OS_LINUX
#define IVY_OS_LINUX
#endif
#elif defined(EMSCRIPTEN) || defined(__EMSCRIPTEN) || defined(__EMSCRIPTEN__)
#ifndef IVY_OS_WASM
#define IVY_OS_WASM
#endif
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#ifndef IVY_OS_BSD
#define IVY_OS_BSD
#endif
#elif defined(__CYGWIN__)
#ifndef IVY_OS_CYGWIN
#define IVY_OS_CYGWIN
// Avoid conflicts with Inventor
#define HAVE_INT8_T
#define HAVE_UINT8_T
#define HAVE_INT16_T
#define HAVE_UINT16_T
#define HAVE_INT32_T
#define HAVE_UINT32_T
#define HAVE_INT64_T
#define HAVE_UINT64_T
#define HAVE_INTPTR_T
#define HAVE_UINTPTR_T
#endif

#else
#error "IVY is not ported to this OS yet. For help see www.freecad.org"
#endif

#ifdef IVY_OS_WIN32
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif

//**************************************************************************
// Standard types for Windows

#if defined(__MINGW32__)
// nothing specific here
#elif defined(IVY_OS_WIN64) || defined(IVY_OS_WIN32)

#ifndef HAVE_INT8_T
#define HAVE_INT8_T
typedef signed char int8_t;
#endif

#ifndef HAVE_UINT8_T
#define HAVE_UINT8_T
typedef unsigned char uint8_t;
#endif

#ifndef HAVE_INT16_T
#define HAVE_INT16_T
typedef short int16_t;
#endif

#ifndef HAVE_UINT16_T
#define HAVE_UINT16_T
typedef unsigned short uint16_t;
#endif

#ifndef HAVE_INT32_T
#define HAVE_INT32_T
typedef int int32_t;
#endif

#ifndef HAVE_UINT32_T
#define HAVE_UINT32_T
typedef unsigned int uint32_t;
#endif

#ifndef HAVE_INT64_T
#define HAVE_INT64_T
typedef __int64 int64_t;
#endif

#ifndef HAVE_UINT64_T
#define HAVE_UINT64_T
typedef unsigned __int64 uint64_t;
#endif

/* avoid to redefine the HAVE_* in Coin's inttypes.h file */
#define COIN_CONFIGURE_BUILD
/* The <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1
/* The <stdint.h> header file. */
#define HAVE_STDINT_H 1
/* The <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1
/* The <stddef.h> header file. */
#define HAVE_STDDEF_H 1

#endif

// FIXME: Move to modules where OCC is needed
//**************************************************************************
//  Open CasCade

#ifdef _MSC_VER
#ifndef WNT
#define WNT
#endif
#ifndef WIN32
#define WIN32
#endif
#ifndef _WINDOWS
#define _WINDOWS
#endif
#endif

#ifdef IVY_OS_LINUX
#define LIN
#define LININTEL
// #       define NO_CXX_EXCEPTION
#endif

#define CSFDB

/// enables the use of the OCC DocumentBrowser
#ifndef IVY_OS_LINUX
#define IVY_USE_OCAFBROWSER
#endif

#ifdef IVY_OCC_DEBUG
#ifdef IVY_DEBUG
#define DEBUG 1
#else
#undef DEBUG
#ifndef NDEBUG
#define NDEBUG
#endif
#endif
#endif

//**************************************************************************
// Exception handling

// Don't catch C++ exceptions in DEBUG!
#ifdef IVY_DEBUG
#define DONT_CATCH_CXX_EXCEPTIONS 1
#define DBG_TRY
#define DBG_CATCH(X)
#else
/// used to switch a catch with the debug state
#define DBG_TRY \
  try           \
  {
/// see docu DBGTRY
#define DBG_CATCH(X) \
  }                  \
  catch (...) { X }
#endif

//**************************************************************************
// Windows import export DLL defines
// #include <ivyglobal.h>

//**************************************************************************
// point at which warnings of overly long specifiers disabled (needed for VC6)
#ifdef _MSC_VER
#pragma warning(disable : 4251)
// #   pragma warning( disable : 4503 )
// #   pragma warning( disable : 4786 )  // specifier longer then 255 chars
// #   pragma warning( disable : 4290 )  // not implemented throw specification
#pragma warning(disable : 4996) // suppress deprecated warning for e.g. open()
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#endif
// #	define _PreComp_                  // use precompiled header
#endif

#endif // IVY_CONFIG_H
