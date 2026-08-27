# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-src")
  file(MAKE_DIRECTORY "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-src")
endif()
file(MAKE_DIRECTORY
  "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-build"
  "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-subbuild/fmt-populate-prefix"
  "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-subbuild/fmt-populate-prefix/tmp"
  "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-subbuild/fmt-populate-prefix/src/fmt-populate-stamp"
  "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-subbuild/fmt-populate-prefix/src"
  "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-subbuild/fmt-populate-prefix/src/fmt-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-subbuild/fmt-populate-prefix/src/fmt-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/zhubulandeshuizhuyu/Project/Code/C++/CppLearning/day32/build/_deps/fmt-subbuild/fmt-populate-prefix/src/fmt-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
