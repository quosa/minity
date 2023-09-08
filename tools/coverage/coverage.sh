#! /bin/bash

root=`git rev-parse --show-toplevel`
pushd "${root}"

rm test.prof* minity.prof*
make clean

# install if not already installed
# https://github.com/hooddanielc/llvm-coverage-viewer
# npm install -g llvm-coverage-viewer
if ! command -v llvm-coverage-viewer &> /dev/null
then
    echo "llvm-coverage-viewer could not be found - installing"
    npm install -g llvm-coverage-viewer
fi

# coverage https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
# export CXXFLAGS="-g -O0 -fprofile-instr-generate -fcoverage-mapping"
# export LDFLAGS="-fprofile-instr-generate"

coverage=1 make test
LLVM_PROFILE_FILE="test.profraw" ./bin/test
xcrun llvm-profdata merge -sparse test.profraw -o test.profdata
#xcrun llvm-cov show ./bin/test -instr-profile=test.profdata
# add -dump to debug if some files fail to show correctly
xcrun llvm-cov report ./bin/test -instr-profile=test.profdata
xcrun llvm-cov export ./bin/test -instr-profile=test.profdata --format=text > coverage.json
llvm-coverage-viewer -j coverage.json -o coverage.html
open coverage.html

# coverage=1 make minity
# LLVM_PROFILE_FILE="minity.profraw" ./bin/minity
# xcrun llvm-profdata merge -sparse minity.profraw -o minity.profdata
# xcrun llvm-cov report ./bin/minity -instr-profile=minity.profdata
#
# xcrun llvm-cov show ./bin/minity -instr-profile=minity.profdata
# xcrun llvm-cov export ./bin/minity -instr-profile=minity.profdata --format=text > minity-coverage.json
# llvm-coverage-viewer -j minity-coverage.json -o minity-coverage.html
# open minity-coverage.html
popd


