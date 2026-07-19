# Copyright Cartesi and individual authors (see AUTHORS)
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

PREFIX   ?= /usr
CXX      := $(TOOLCHAIN_PREFIX)g++
CXXFLAGS += -Wall -Wextra -pedantic -O2 -std=c++23 -I../third-party/cartesi
HEX_SRC  := ../third-party/cartesi/hex.cpp
HEX_HDRS := ../third-party/cartesi/hex.hpp ../third-party/cartesi/concepts.hpp

all: hex

hex: hex.cpp $(HEX_SRC) $(HEX_HDRS)
	$(CXX) $(CXXFLAGS) -o $@ hex.cpp $(HEX_SRC) $(LDFLAGS)

hex-test: hex-test.cpp $(HEX_SRC) $(HEX_HDRS)
	$(CXX) $(CXXFLAGS) -o $@ hex-test.cpp $(HEX_SRC) $(LDFLAGS)

test: hex hex-test
	./hex-test
	./test.sh

install: hex
	install -Dm755 hex $(DESTDIR)$(PREFIX)/bin/hex

clean:
	@rm -f hex hex-test

.PHONY: all test install clean
