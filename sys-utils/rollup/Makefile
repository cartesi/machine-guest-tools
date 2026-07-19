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
CXXFLAGS += -Wall -Wextra -pedantic -O2 -std=c++23 -I../libcmt/include -I../third-party/cartesi \
	-Ithird-party/nlohmann-json
LDFLAGS  += -L../libcmt/build/riscv64 -l:libcmt.a
BASE64_SRC  := ../third-party/cartesi/base64.cpp
BASE64_HDRS := ../third-party/cartesi/base64.hpp
HEX_SRC  := ../third-party/cartesi/hex.cpp
HEX_HDRS := ../third-party/cartesi/hex.hpp ../third-party/cartesi/concepts.hpp

all: rollup

rollup: rollup.cpp $(BASE64_SRC) $(BASE64_HDRS) $(HEX_SRC) $(HEX_HDRS)
	$(CXX) $(CXXFLAGS) -o $@ rollup.cpp $(BASE64_SRC) $(HEX_SRC) $(LDFLAGS)

base64-test: base64-test.cpp $(BASE64_SRC) $(BASE64_HDRS) ../third-party/cartesi/concepts.hpp
	$(CXX) $(CXXFLAGS) -o $@ base64-test.cpp $(BASE64_SRC)

test: base64-test
	./base64-test

install: rollup
	install -Dm755 rollup $(DESTDIR)$(PREFIX)/bin/rollup

clean:
	@rm -f rollup base64-test

.PHONY: all test install clean
