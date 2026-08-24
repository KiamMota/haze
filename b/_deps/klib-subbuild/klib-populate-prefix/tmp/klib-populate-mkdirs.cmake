# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/kimas/projects/haze/b/_deps/klib-src")
  file(MAKE_DIRECTORY "/home/kimas/projects/haze/b/_deps/klib-src")
endif()
file(MAKE_DIRECTORY
  "/home/kimas/projects/haze/b/_deps/klib-build"
  "/home/kimas/projects/haze/b/_deps/klib-subbuild/klib-populate-prefix"
  "/home/kimas/projects/haze/b/_deps/klib-subbuild/klib-populate-prefix/tmp"
  "/home/kimas/projects/haze/b/_deps/klib-subbuild/klib-populate-prefix/src/klib-populate-stamp"
  "/home/kimas/projects/haze/b/_deps/klib-subbuild/klib-populate-prefix/src"
  "/home/kimas/projects/haze/b/_deps/klib-subbuild/klib-populate-prefix/src/klib-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/kimas/projects/haze/b/_deps/klib-subbuild/klib-populate-prefix/src/klib-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/kimas/projects/haze/b/_deps/klib-subbuild/klib-populate-prefix/src/klib-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
