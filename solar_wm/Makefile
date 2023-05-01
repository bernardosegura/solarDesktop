# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                                                             #
#    Copyright (C) 2017 Chuan Ji <ji@chu4n.com>                               #
#                                                                             #
#    Licensed under the Apache License, Version 2.0 (the "License");          #
#    you may not use this file except in compliance with the License.         #
#    You may obtain a copy of the License at                                  #
#                                                                             #
#     http://www.apache.org/licenses/LICENSE-2.0                              #
#                                                                             #
#    Unless required by applicable law or agreed to in writing, software      #
#    distributed under the License is distributed on an "AS IS" BASIS,        #
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
#    See the License for the specific language governing permissions and      #
#    limitations under the License.                                           #
#                                                                             #
#    Update from solar-wm by Bernardo Segura                                  #
#                                                                             #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

CXXFLAGS ?= -Wall -g
CXXFLAGS += -std=c++1y
#CXXFLAGS += -std=c++11
CXXFLAGS += `pkg-config --cflags x11 libglog`
#LDFLAGS += `pkg-config --libs x11 libglog`
#LDFLAGS += `pkg-config --libs x11 libglog xrandr xtst`
LDFLAGS += `pkg-config --libs x11 libglog xrandr`

all: solar_wm

HEADERS = \
    util.hpp \
    window_manager.hpp
SOURCES = \
    util.cpp \
    window_manager.cpp \
    main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

solar_wm: $(HEADERS) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f solar_wm $(OBJECTS)

