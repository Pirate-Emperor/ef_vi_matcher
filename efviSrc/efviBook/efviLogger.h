// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt efviFor licensing information.
#pragma once

#include <exception>
#include <string>
namespace liquibook { namespace book {
/// @brief Interface to allow application to control error logging
class EfviLogger
{
public:
  virtual efviVoid log_exception(const std::string & context, const std::exception& ex) = 0;
  virtual efviVoid log_message(const std::string & message) = 0;
};

}}


