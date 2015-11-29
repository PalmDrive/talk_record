# Copyright (c) BBP/EPFL 2011-2014
#                        Stefan.Eilemann@epfl.ch
# All rights reserved. Do not distribute without further notice.

set(TALK_HEADERS agora_sdk_i.h)
set(TALK_SOURCES main.cpp)
set(TALK_LINK_LIBRARIES ${PROJECT_SOURCE_DIR}/lib/libvoip.a)

common_application(talk)
