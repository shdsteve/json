// Copyright (c) 2014-2017 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#ifndef TAOCPP_JSON_PEGTL_INCLUDE_CONTRIB_RAW_STRING_HPP
#define TAOCPP_JSON_PEGTL_INCLUDE_CONTRIB_RAW_STRING_HPP

#include "../apply_mode.hpp"
#include "../config.hpp"
#include "../nothing.hpp"
#include "../rewind_mode.hpp"

#include "../internal/action_input.hpp"
#include "../internal/must.hpp"
#include "../internal/skip_control.hpp"
#include "../internal/state.hpp"
#include "../internal/until.hpp"

#include "../analysis/generic.hpp"

namespace tao
{
   namespace TAOCPP_JSON_PEGTL_NAMESPACE
   {
      namespace internal
      {
         template< char Open, char Intermediate, char Close >
         struct raw_string_tag
         {
         };

         template< typename Tag >
         struct raw_string_state
         {
            template< typename Input, typename... States >
            raw_string_state( const Input& in, States&&... )
               : cdata( in.count() ),
                 size( in.size( 0 ) )
            {
            }

            template< apply_mode A,
                      rewind_mode,
                      template< typename... > class Action,
                      template< typename... > class Control,
                      typename Input,
                      typename... States >
            typename std::enable_if< ( ( A == apply_mode::ACTION ) && ( !is_nothing< Action, Tag >::value ) ) >::type
            success( const Input& in, States&&... st ) const
            {
               using eol_t = typename Input::eol_t;
               using action_t = typename Input::action_t;
               count_data data = { cdata.byte, cdata.line, cdata.byte_in_line, in.begin() - size + in.size( 0 ) + count };
               data.data += ( *data.data ) == eol_t::ch;
               const action_t ai( data, in.begin() - count, in.source() );
               Action< Tag >::apply( ai, st... );
            }

            template< apply_mode A,
                      rewind_mode,
                      template< typename... > class Action,
                      template< typename... > class Control,
                      typename Input,
                      typename... States >
            typename std::enable_if< !( ( A == apply_mode::ACTION ) && ( !is_nothing< Action, Tag >::value ) ) >::type
            success( const Input&, States&&... ) const
            {
            }

            raw_string_state( const raw_string_state& ) = delete;
            void operator=( const raw_string_state& ) = delete;

            count_data cdata;
            std::size_t size;
            std::size_t count = 0;
         };

         template< typename Tag, char Open, char Intermediate >
         struct raw_string_open
         {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;

            template< apply_mode A,
                      rewind_mode,
                      template< typename... > class Action,
                      template< typename... > class Control,
                      typename Input >
            static bool match( Input& in, raw_string_state< Tag >& ls )
            {
               if( in.empty() || ( in.peek_char( 0 ) != Open ) ) {
                  return false;
               }
               for( std::size_t i = 1; i < in.size( i + 1 ); ++i ) {
                  switch( const auto c = in.peek_char( i ) ) {
                     case Open:
                        ls.count = i + 1;
                        in.bump( ls.count );
                        return true;
                     case Intermediate:
                        break;
                     default:
                        return false;
                  }
               }
               return false;
            }
         };

         template< typename Tag, char Intermediate, char Close >
         struct raw_string_close
         {
            using analyze_t = analysis::generic< analysis::rule_type::ANY >;

            template< apply_mode A,
                      rewind_mode,
                      template< typename... > class Action,
                      template< typename... > class Control,
                      typename Input >
            static bool match( Input& in, const raw_string_state< Tag >& ls )
            {
               if( in.size( ls.count ) < ls.count ) {
                  return false;
               }
               if( in.peek_char( 0 ) != Close ) {
                  return false;
               }
               if( in.peek_char( ls.count - 1 ) != Close ) {
                  return false;
               }
               for( std::size_t i = 0; i < ls.count - 2; ++i ) {
                  if( in.peek_char( i + 1 ) != Intermediate ) {
                     return false;
                  }
               }
               in.bump( ls.count );
               return true;
            }
         };

         template< typename Tag, char Open, char Intermediate >
         struct skip_control< raw_string_open< Tag, Open, Intermediate > > : std::true_type
         {
         };

         template< typename Tag, char Intermediate, char Close >
         struct skip_control< raw_string_close< Tag, Intermediate, Close > > : std::true_type
         {
         };

      }  // namespace internal

      // raw_string matches Lua-style long literals.
      //
      // The following description was taken from the Lua documentation
      // (see http://www.lua.org/docs.html):
      //
      // - An "opening long bracket of level n" is defined as an opening square
      //   bracket followed by n equal signs followed by another opening square
      //   bracket. So, an opening long bracket of level 0 is written as `[[`,
      //   an opening long bracket of level 1 is written as `[=[`, and so on.
      // - A "closing long bracket" is defined similarly; for instance, a closing
      //   long bracket of level 4 is written as `]====]`.
      // - A "long literal" starts with an opening long bracket of any level and
      //   ends at the first closing long bracket of the same level. It can
      //   contain any text except a closing bracket of the same level.
      // - Literals in this bracketed form can run for several lines, do not
      //   interpret any escape sequences, and ignore long brackets of any other
      //   level.
      // - For convenience, when the opening long bracket is immediately followed
      //   by a newline, the newline is not included in the string.
      //
      // Note that unlike Lua's long literal, a raw_string is customizable to use
      // other characters than `[`, `=` and `]` for matching. Also note that Lua
      // introduced newline-specific replacements in Lua 5.2, which we do not
      // support on the grammar level.

      template< char Open, char Intermediate, char Close, typename Tag = internal::raw_string_tag< Open, Intermediate, Close > >
      struct raw_string
         : internal::state< internal::raw_string_state< Tag >,
                            internal::raw_string_open< Tag, Open, Intermediate >,
                            internal::must< internal::until< internal::raw_string_close< Tag, Intermediate, Close > > > >
      {
         // This is used to bind an action to the content.
         using content = Tag;

         // This is used for error-reporting when a raw string is not closed properly.
         using close = internal::until< internal::raw_string_close< Tag, Intermediate, Close > >;
      };

   }  // namespace TAOCPP_JSON_PEGTL_NAMESPACE

}  // namespace tao

#endif
