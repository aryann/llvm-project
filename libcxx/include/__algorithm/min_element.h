//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___ALGORITHM_MIN_ELEMENT_H
#define _LIBCPP___ALGORITHM_MIN_ELEMENT_H

#include <__algorithm/comp.h>
#include <__algorithm/comp_ref_type.h>
#include <__config>
#include <__functional/identity.h>
#include <__iterator/iterator_traits.h>
#include <__iterator/segmented_iterator.h>
#include <__type_traits/invoke.h>
#include <__type_traits/is_callable.h>
#include <__utility/move.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

template <class _Comp, class _Iter, class _Sent, class _Proj>
inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14 _Iter
__min_element(_Iter __first, _Sent __last, _Comp& __comp, _Proj& __proj) {
  if (__first == __last) {
    return __first;
  }

  _Iter __min             = __first;
  auto __maybe_update_min = [&__comp, &__proj, &__min](const _Iter& __curr) {
    if (std::__invoke(__comp, std::__invoke(__proj, *__curr), std::__invoke(__proj, *__min))) {
      __min = __curr;
    }
  };

  if constexpr (is_same<_Iter, _Sent>::value && __is_segmented_iterator_v<_Iter>) {
    using _Traits        = __segmented_iterator_traits<_Iter>;
    using _LocalIterator = typename _Traits::__local_iterator;

    auto __sfirst = _Traits::__segment(__first);
    auto __slast  = _Traits::__segment(__last);

    auto __scan = [&](auto __segment, _LocalIterator __lfirst, _LocalIterator __llast) {
      _LocalIterator __lmin = std::__min_element(__lfirst, __llast, __comp, __proj);
      if (__lmin == __llast) {
        return;
      }
      _Iter __candidate = _Traits::__compose(__segment, __lmin);
      __maybe_update_min(__candidate);
    };

    // We are in a single segment, so we might not be at the beginning or end.
    if (__sfirst == __slast) {
      __scan(__sfirst, _Traits::__local(__first), _Traits::__local(__last));
      return __min;
    }

    // We have more than one segment. Iterate over the first segment, since we might not start at the beginning.
    __scan(__sfirst, _Traits::__local(__first), _Traits::__end(__sfirst));
    ++__sfirst;

    // Iterate over the segments which are guaranteed to be completely in the range.
    while (__sfirst != __slast) {
      __scan(__sfirst, _Traits::__begin(__sfirst), _Traits::__end(__sfirst));
      ++__sfirst;
    }

    // Iterate over the last segment.
    __scan(__sfirst, _Traits::__begin(__sfirst), _Traits::__local(__last));

  } else {
    while (++__first != __last) {
      __maybe_update_min(__first);
    }
  }

  return __min;
}

template <class _Comp, class _Iter, class _Sent>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14 _Iter __min_element(_Iter __first, _Sent __last, _Comp __comp) {
  auto __proj = __identity();
  return std::__min_element<_Comp>(std::move(__first), std::move(__last), __comp, __proj);
}

template <class _ForwardIterator, class _Compare>
[[__nodiscard__]] inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14 _ForwardIterator
min_element(_ForwardIterator __first, _ForwardIterator __last, _Compare __comp) {
  static_assert(
      __has_forward_iterator_category<_ForwardIterator>::value, "std::min_element requires a ForwardIterator");
  static_assert(
      __is_callable<_Compare&, decltype(*__first), decltype(*__first)>::value, "The comparator has to be callable");

  return std::__min_element<__comp_ref_type<_Compare> >(std::move(__first), std::move(__last), __comp);
}

template <class _ForwardIterator>
[[__nodiscard__]] inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX14 _ForwardIterator
min_element(_ForwardIterator __first, _ForwardIterator __last) {
  return std::min_element(__first, __last, __less<>());
}

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___ALGORITHM_MIN_ELEMENT_H
