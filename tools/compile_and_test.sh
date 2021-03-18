#!/bin/bash
# Copyright 2021 4Paradigm
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -eE
set -o nounset

cd "$(dirname "$0")"
cd "$(git rev-parse --show-toplevel)"

if uname -a | grep -q Darwin; then
    # in case coreutils not install on mac
    alias nproc='sysctl -n hw.logicalcpu'
fi

export PATH=${PWD}/thirdparty/bin:$PATH

rm -rf build
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE=ON -DCOVERAGE_ENABLE=OFF -DTESTING_ENABLE=ON -DEXAMPLES_ENABLE=ON
make fesql_proto && make fesql_parser
make -j"$(nproc)"
make test -j"$(nproc)"
