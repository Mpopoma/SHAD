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

#ifndef INCLUDE_SHAD_RUNTIME_ASYNCHRONOUS_INTERFACE_H_
#define INCLUDE_SHAD_RUNTIME_ASYNCHRONOUS_INTERFACE_H_

#include <cstddef>
#include <cstdint>
#include <memory>

#include "shad/runtime/handle.h"
#include "shad/runtime/locality.h"

namespace shad {
namespace rt {

namespace impl {

template <typename TargetSystemTag>
struct AsynchronousInterface {
  template <typename FunT, typename InArgsT>
  static void asyncExecuteAt(Handle &handle, const Locality &loc, FunT &&func,
                             const InArgsT &args);

  template <typename FunT>
  static void asyncExecuteAt(Handle &handle, const Locality &loc, FunT &&func,
                             const std::shared_ptr<uint8_t> &argsBuffer,
                             const uint32_t bufferSize);

  template <typename FunT, typename InArgsT>
  static void asyncExecuteAtWithRetBuff(Handle &handle, const Locality &loc,
                                        FunT &&func, const InArgsT &args,
                                        uint8_t *resultBuffer,
                                        uint32_t *resultSize);

  template <typename FunT>
  static void asyncExecuteAtWithRetBuff(
      Handle &handle, const Locality &loc, FunT &&func,
      const std::shared_ptr<uint8_t> &argsBuffer, const uint32_t bufferSize,
      uint8_t *resultBuffer, uint32_t *resultSize);

  template <typename FunT, typename InArgsT, typename ResT>
  static void asyncExecuteAtWithRet(Handle &handle, const Locality &loc,
                                    FunT &&func, const InArgsT &args,
                                    ResT *result);

  template <typename FunT, typename ResT>
  static void asyncExecuteAtWithRet(Handle &handle, const Locality &loc,
                                    FunT &&func,
                                    const std::shared_ptr<uint8_t> &argsBuffer,
                                    const uint32_t bufferSize, ResT *result);

  template <typename FunT, typename InArgsT>
  static void asyncExecuteOnAll(Handle &handle, FunT &&func,
                                const InArgsT &args);

  template <typename FunT>
  static void asyncExecuteOnAll(Handle &handle, FunT &&func,
                                const std::shared_ptr<uint8_t> &argsBuffer,
                                const uint32_t bufferSize);

  template <typename FunT, typename InArgsT>
  static void asyncForEachAt(Handle &handle, const Locality &loc, FunT &&func,
                             const InArgsT &args, const size_t numIters);

  template <typename FunT>
  static void asyncForEachAt(Handle &handle, const Locality &loc, FunT &&func,
                             const std::shared_ptr<uint8_t> &argsBuffer,
                             const uint32_t bufferSize, const size_t numIters);

  template <typename FunT, typename InArgsT>
  static void asyncForEachOnAll(Handle &handle, FunT &&func,
                                const InArgsT &args, const size_t numIters);

  template <typename FunT>
  static void asyncForEachOnAll(Handle &handle, FunT &&func,
                                const std::shared_ptr<uint8_t> &argsBuffer,
                                const uint32_t bufferSize,
                                const size_t numIters);
};

}  // namespace impl

}  // namespace rt
}  // namespace shad

#endif  // INCLUDE_SHAD_RUNTIME_ASYNCHRONOUS_INTERFACE_H_
