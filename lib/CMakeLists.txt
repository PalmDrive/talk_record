# Copyright (c) BBP/EPFL 2011-2014
#                        Stefan.Eilemann@epfl.ch
# All rights reserved. Do not distribute without further notice.

source_group(\\ FILES CMakeLists.txt)

set(Agora_HEADERS)
set(Agora_SOURCES lib/libvoip.a)
set(Agora_LIBRARY_TYPE STATIC)

common_library(Agora)
