//  Copyright (c) 2015 Agustin Berge
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 0

#   include <hpx/config/defines.hpp>

#   ifdef HPX_HAVE_CXX11_NOEXCEPT
#       define HPX_NOEXCEPT noexcept
#       define HPX_NOEXCEPT_IF(Cond) noexcept(Cond)
#       define HPX_NOEXCEPT_EXPR(Expr) noexcept(Expr)
#   else
#       define HPX_NOEXCEPT
#       define HPX_NOEXCEPT_IF(Cond)
#       define HPX_NOEXCEPT_EXPR(Expr) false
#   endif

#   ifdef HPX_HAVE_CXX11_CONSTEXR
#       define HPX_CONSTEXPR constexpr
#       ifdef HPX_HAVE_CXX14_CONSTEXR
#           define HPX_RELAXED_CONSTEXPR constexpr
#       else
#           define HPX_RELAXED_CONSTEXPR inline
#       endif
#       define HPX_CONSTEXPR_OR_CONST constexpr
#       define HPX_STATIC_CONSTEXPR static constexpr
#   else
#       define HPX_CONSTEXPR inline
#       define HPX_RELAXED_CONSTEXPR inline
#       define HPX_CONSTEXPR_OR_CONST const
#       define HPX_STATIC_CONSTEXPR static const
#   endif

#   ifdef HPX_HAVE_CXX11_DELETED_FUNCTIONS
#       define HPX_DELETED_FUNCTION = delete
#   else
#       define HPX_DELETED_FUNCTION
#   endif

#   ifdef HPX_HAVE_CXX11_INLINE_NAMESPACE
#       define HPX_INLINE_NAMESPACE(name)  inline namespace name
#   else
#       define HPX_INLINE_NAMESPACE(name)  namespace name
#   endif

#   if defined(_MSC_VER)
#       define HPX_FORCEINLINE __forceinline
#       define HPX_NOINLINE __declspec(noinline)
#   else
#       define HPX_FORCEINLINE inline
#       define HPX_NOINLINE
#   endif

#elif HPX_CONFIG_INCLUDE_LEVEL == 0
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 1
#elif HPX_CONFIG_INCLUDE_LEVEL == 1
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 2
#elif HPX_CONFIG_INCLUDE_LEVEL == 2
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 3
#elif HPX_CONFIG_INCLUDE_LEVEL == 3
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 4
#elif HPX_CONFIG_INCLUDE_LEVEL == 4
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 5
#elif HPX_CONFIG_INCLUDE_LEVEL == 5
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 6
#elif HPX_CONFIG_INCLUDE_LEVEL == 6
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 7
#elif HPX_CONFIG_INCLUDE_LEVEL == 7
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 8
#elif HPX_CONFIG_INCLUDE_LEVEL == 8
#   undef HPX_CONFIG_INCLUDE_LEVEL
#   define HPX_CONFIG_INCLUDE_LEVEL 9
#else
#   error HPX inclusion level too deep
#endif
