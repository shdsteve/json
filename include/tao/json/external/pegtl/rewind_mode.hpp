// Copyright (c) 2016-2018 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#ifndef TAO_JSON_PEGTL_INCLUDE_REWIND_MODE_HPP
#define TAO_JSON_PEGTL_INCLUDE_REWIND_MODE_HPP

#include "config.hpp"

namespace tao
{
   namespace TAO_JSON_PEGTL_NAMESPACE
   {
      enum class rewind_mode : char
      {
         ACTIVE,
         REQUIRED,
         DONTCARE
      };

   }  // namespace TAO_JSON_PEGTL_NAMESPACE

}  // namespace tao

#endif
