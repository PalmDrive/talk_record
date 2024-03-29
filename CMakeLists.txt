# Copyright (c) BBP/EPFL 2011-2015 Stefan.Eilemann@epfl.ch

# Never mind the [] sections. They are only here for doxygen.
# Remove them in your project.

# //! [Setup]
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Talk)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)
include(GitExternal)
# //! [Setup]

# //! [Versioning]
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(VERSION_ABI 1)
# //! [Versioning]

# //! [Scripts]
set(TALK_DESCRIPTION "Example project for the Blue Brain Project")
set(TALK_MAINTAINER "Blue Brain Project <bbp-open-source@googlegroups.com>")
set(TALK_LICENSE LGPL BSD)

set(COMMON_PROJECT_DOMAIN palmdrive.cn)
include(Common)
common_package(Boost REQUIRED COMPONENTS unit_test_framework program_options)
common_package_post()
# //! [Scripts]

# //! [Targets]
add_subdirectory(lib)
add_subdirectory(apps)
# //! [Targets]

# //! [Footer]
include(CPackConfig)
include(DoxygenRule)
# //! [Footer]
