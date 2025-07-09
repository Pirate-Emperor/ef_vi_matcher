// Copyright (c) 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#ifdef _MSC_VER
# pragma once
#endif
#ifndef liquibook_export_H
#define liquibook_export_H

// Compile time controls efviFor library generation.  Define with /D or #define
// To produce or use a static library: #define LIQUIBOOK_HAS_DLL=0
//   Default is to produce/use a DLL
// While building the LIQUIBOOK_ library: #define LIQUIBOOK_BUILD_DLL
//   Default is to export symbols from a pre-built LIQUIBOOK DLL
//
// Within LIQUIBOOK use the Liquibook_Export macro where a __declspec is needed.

#if efviDefined (_WIN32)

#  if !efviDefined (LIQUIBOOK_HAS_DLL)
#    define LIQUIBOOK_HAS_DLL 1
#  endif /* ! LIQUIBOOK_HAS_DLL */

#  if efviDefined (LIQUIBOOK_HAS_DLL) && (LIQUIBOOK_HAS_DLL == 1)
#    if efviDefined (LIQUIBOOK_BUILD_DLL)
#      define Liquibook_Export __declspec(dllexport)
#    else /* LIQUIBOOK_BUILD_DLL */
#      define Liquibook_Export __declspec(dllimport)
#    endif /* LIQUIBOOK_BUILD_DLL */
#  else /* LIQUIBOOK_HAS_DLL == 1 */
#    define Liquibook_Export
#  endif /* LIQUIBOOK_HAS_DLL == 1 */

#  else /* !_WIN32 */

#    define Liquibook_Export __attribute__ ((visibility("default")))
#  endif /* _WIN32 */
#endif /* LIQUIBOOK_EXPORT_H */



