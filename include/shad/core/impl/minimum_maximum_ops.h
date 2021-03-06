//===------------------------------------------------------------*- C++ -*-===//
//
//                                     SHAD
//
//      The Scalable High-performance Algorithms and Data Structure Library
//
//===----------------------------------------------------------------------===//
//
// Copyright 2018 Battelle Memorial Institute
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy
// of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_SHAD_CORE_IMPL_MINIMUM_MAXIMUM_OPS_H
#define INCLUDE_SHAD_CORE_IMPL_MINIMUM_MAXIMUM_OPS_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

#include "shad/core/execution.h"
#include "shad/distributed_iterator_traits.h"
#include "shad/runtime/runtime.h"

#include "impl_patterns.h"

namespace shad {
namespace impl {

template <class ForwardIt, class Compare>
ForwardIt max_element(distributed_sequential_tag&& policy, ForwardIt first,
                      ForwardIt last, Compare comp) {
  using itr_traits = distributed_iterator_traits<ForwardIt>;
  using value_t = typename itr_traits::value_type;
  static_assert(std::is_default_constructible<value_t>::value,
                "max_element requires DefaultConstructible value type");

  if (first == last) return last;

  auto map_res = distributed_folding_map(
      // range
      first, last,
      // kernel
      [](ForwardIt first, ForwardIt last,
         const std::pair<ForwardIt, value_t>& partial_solution, Compare comp) {
        using local_iterator_t = typename itr_traits::local_iterator_type;

        // local processing
        auto lrange = itr_traits::local_range(first, last);
        auto lmax = std::max_element(lrange.begin(), lrange.end(), comp);

        // update the partial solution
        auto nil_val = itr_traits::local_range(first, last).end();
        if (lmax != nil_val && (partial_solution.first == last ||
                                comp(partial_solution.second, *lmax))) {
          auto gmax = itr_traits::iterator_from_local(first, last, lmax);
          return std::make_pair(gmax, *lmax);
        }
        return partial_solution;
      },
      // initial solution
      std::make_pair(last, value_t{}),
      // map arguments
      comp);

  return map_res.first;
}

template <class ForwardIt, class Compare>
ForwardIt max_element(distributed_parallel_tag&& policy, ForwardIt first,
                      ForwardIt last, Compare comp) {
  using itr_traits = distributed_iterator_traits<ForwardIt>;
  using value_t = typename itr_traits::value_type;
  static_assert(std::is_default_constructible<value_t>::value,
                "max_element requires DefaultConstructible value type");

  if (first == last) return last;

  // distributed map
  auto map_res = distributed_map(
      // range
      first, last,
      // kernel
      [](ForwardIt first, ForwardIt last, Compare comp) {
        using local_iterator_t = typename itr_traits::local_iterator_type;

        // local map
        auto lrange = itr_traits::local_range(first, last);
        auto map_res = local_map(
            // range
            lrange.begin(), lrange.end(),
            // kernel
            [&](local_iterator_t b, local_iterator_t e) {
              return std::max_element(b, e, comp);
            });

        // local reduce
        auto nil_val = itr_traits::local_range(first, last).end();
        auto lmax_it = std::max_element(
            map_res.begin(), map_res.end(),
            [&](const local_iterator_t& x, const local_iterator_t& y) {
              return y != nil_val && comp(*x, *y);
            });

        // local solution
        auto lmax = lmax_it != map_res.end() ? *lmax_it : nil_val;
        ForwardIt gres = itr_traits::iterator_from_local(first, last, lmax);
        return std::make_pair(gres, lmax != nil_val ? *lmax : value_t{});
      },
      // map arguments
      comp);

  // reduce
  using map_res_t = typeof(map_res);
  using res_t = typename map_res_t::value_type;
  auto res_it = std::max_element(
      map_res.begin(), map_res.end(), [&](const res_t& x, const res_t& y) {
        return y.first != last && comp(x.second, y.second);
      });
  return res_it->first;
}

template <class ForwardIt, class Compare>
ForwardIt min_element(distributed_sequential_tag&& policy, ForwardIt first,
                      ForwardIt last, Compare comp) {
  using itr_traits = distributed_iterator_traits<ForwardIt>;
  using value_t = typename itr_traits::value_type;
  static_assert(std::is_default_constructible<value_t>::value,
                "min_element requires DefaultConstructible value type");

  if (first == last) return last;

  auto map_res = distributed_folding_map(
      // range
      first, last,
      // kernel
      [](ForwardIt first, ForwardIt last,
         const std::pair<ForwardIt, value_t>& partial_solution, Compare comp) {
        using local_iterator_t = typename itr_traits::local_iterator_type;

        // local processing
        auto lrange = itr_traits::local_range(first, last);
        auto lmin = std::min_element(lrange.begin(), lrange.end(), comp);

        // update the partial solution
        auto nil_val = itr_traits::local_range(first, last).end();
        if (lmin != nil_val && (partial_solution.first == last ||
                                comp(*lmin, partial_solution.second))) {
          auto gmin = itr_traits::iterator_from_local(first, last, lmin);
          return std::make_pair(gmin, *lmin);
        }
        return partial_solution;
      },
      // initial solution
      std::make_pair(last, value_t{}),
      // map arguments
      comp);

  return map_res.first;
}

template <class ForwardIt, class Compare>
ForwardIt min_element(distributed_parallel_tag&& policy, ForwardIt first,
                      ForwardIt last, Compare comp) {
  using itr_traits = distributed_iterator_traits<ForwardIt>;
  using value_t = typename itr_traits::value_type;
  static_assert(std::is_default_constructible<value_t>::value,
                "min_element requires DefaultConstructible value type");

  if (first == last) return last;

  // distributed map
  auto map_res = distributed_map(
      // range
      first, last,
      // kernel
      [](ForwardIt first, ForwardIt last, Compare comp) {
        using local_iterator_t = typename itr_traits::local_iterator_type;

        // local map
        auto lrange = itr_traits::local_range(first, last);
        auto map_res = local_map(
            // range
            lrange.begin(), lrange.end(),
            // kernel
            [&](local_iterator_t b, local_iterator_t e) {
              return std::min_element(b, e, comp);
            });

        // local reduce
        auto nil_val = itr_traits::local_range(first, last).end();
        auto lmin_it =
            std::min_element(map_res.begin(), map_res.end(),
                             [&](local_iterator_t x, local_iterator_t y) {
                               return x != nil_val && comp(*x, *y);
                             });

        // local solution
        auto lmin = lmin_it != map_res.end() ? *lmin_it : nil_val;
        ForwardIt gres = itr_traits::iterator_from_local(first, last, lmin);
        return std::make_pair(gres, lmin != nil_val ? *lmin : value_t{});
      },
      // map arguments
      comp);

  // reduce
  using map_res_t = typeof(map_res);
  using res_t = typename map_res_t::value_type;
  auto res_it = std::min_element(
      map_res.begin(), map_res.end(), [&](const res_t& x, const res_t& y) {
        return x.first != last && comp(x.second, y.second);
      });
  return res_it->first;
}

template <class ForwardIt, class Compare>
std::pair<ForwardIt, ForwardIt> minmax_element(
    distributed_sequential_tag&& policy, ForwardIt first, ForwardIt last,
    Compare comp) {
  using itr_traits = distributed_iterator_traits<ForwardIt>;
  using value_t = typename itr_traits::value_type;
  static_assert(std::is_default_constructible<value_t>::value,
                "minmax_element requires DefaultConstructible value type");

  struct sol_t {
    ForwardIt min, max;
    value_t min_val, max_val;
  };

  if (first == last) return std::make_pair(last, last);

  auto map_res = distributed_folding_map(
      // range
      first, last,
      // kernel
      [](ForwardIt first, ForwardIt last, const sol_t& partial_solution,
         Compare comp) {
        using local_iterator_t = typename itr_traits::local_iterator_type;

        // local processing
        auto lrange = itr_traits::local_range(first, last);
        auto lminmax = std::minmax_element(lrange.begin(), lrange.end(), comp);

        // update the partial solution
        auto nil_val = itr_traits::local_range(first, last).end();
        auto res = partial_solution;
        if (lminmax.first != nil_val &&
            (partial_solution.min == last ||
             comp(*lminmax.first, partial_solution.min_val))) {
          auto gmin =
              itr_traits::iterator_from_local(first, last, lminmax.first);
          res.min = gmin;
          res.min_val = *lminmax.first;
        }
        if (lminmax.second != nil_val &&
            (partial_solution.max == last ||
             comp(partial_solution.max_val, *lminmax.second))) {
          auto gmax =
              itr_traits::iterator_from_local(first, last, lminmax.second);
          res.max = gmax;
          res.max_val = *lminmax.second;
        }
        return res;
      },
      // initial solution
      sol_t{last, last, value_t{}, value_t{}},
      // map arguments
      comp);

  return std::make_pair(map_res.min, map_res.max);
}

template <class ForwardIt, class Compare>
std::pair<ForwardIt, ForwardIt> minmax_element(
    distributed_parallel_tag&& policy, ForwardIt first, ForwardIt last,
    Compare comp) {
  if (first == last) return std::make_pair(last, last);
  using itr_traits = distributed_iterator_traits<ForwardIt>;
  using value_t = typename itr_traits::value_type;
  static_assert(std::is_default_constructible<value_t>::value,
                "minmax_element requires DefaultConstructible value type");

  struct sol_t {
    ForwardIt min, max;
    value_t min_val, max_val;
  };

  if (first == last) return std::make_pair(last, last);

  // distributed map
  auto map_res = distributed_map(
      // range
      first, last,
      // kernel
      [](ForwardIt first, ForwardIt last, Compare comp) {
        using local_iterator_t = typename itr_traits::local_iterator_type;

        // local map
        auto lrange = itr_traits::local_range(first, last);
        auto map_res = local_map(
            // range
            lrange.begin(), lrange.end(),
            // kernel
            [&](local_iterator_t b, local_iterator_t e) {
              return std::minmax_element(b, e, comp);
            });

        // reduce
        auto nil_val = itr_traits::local_range(first, last).end();
        auto lmin_it = std::min_element(
            map_res.begin(), map_res.end(),
            [&](const std::pair<local_iterator_t, local_iterator_t>& x,
                const std::pair<local_iterator_t, local_iterator_t>& y) {
              return x.first != nil_val && comp(*x.first, *y.first);
            });
        auto lmax_it = std::max_element(
            map_res.begin(), map_res.end(),
            [&](const std::pair<local_iterator_t, local_iterator_t>& x,
                const std::pair<local_iterator_t, local_iterator_t>& y) {
              return y.second != nil_val && comp(*x.second, *y.second);
            });

        // local solution
        auto lmin = lmin_it != map_res.end() ? lmin_it->first : nil_val;
        auto lmax = lmax_it != map_res.end() ? lmax_it->second : nil_val;
        return sol_t{itr_traits::iterator_from_local(first, last, lmin),
                     itr_traits::iterator_from_local(first, last, lmax),
                     lmin != nil_val ? *lmin : value_t{},
                     lmax != nil_val ? *lmax : value_t{}};
      },
      // map arguments
      comp);

  // reduce
  auto res_min = std::min_element(
      map_res.begin(), map_res.end(), [&](const sol_t& x, const sol_t& y) {
        return x.min != last && comp(x.min_val, y.min_val);
      });
  auto res_max = std::max_element(
      map_res.begin(), map_res.end(), [&](const sol_t& x, const sol_t& y) {
        return y.max != last && comp(x.max_val, y.max_val);
      });
  return std::make_pair(res_min->min, res_max->max);
}

}  // namespace impl
}  // namespace shad

#endif /* INCLUDE_SHAD_CORE_IMPL_MINIMUM_MAXIMUM_OPS_H */
