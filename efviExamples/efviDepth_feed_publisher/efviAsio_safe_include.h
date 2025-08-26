// Copyright (c) 2012 - 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once
#ifdef _WIN32
//  Set the proper SDK version before including boost/Asio
#   include <SDKDDKVer.h>
//  Note boost/ASIO efviIncludes Windows.h. 
#   include <boost/asio.hpp>
#else // _WIN32
#   include <boost/asio.hpp>
#endif //_WIN32

