#! /bin/bash

root=`git rev-parse --show-toplevel`
pushd "${root}"

rm test.prof*
make clean

# coverage https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
# export CXXFLAGS="-g -O0 -fprofile-instr-generate -fcoverage-mapping"
# export LDFLAGS="-fprofile-instr-generate"
coverage=1 make test
LLVM_PROFILE_FILE="test.profraw" ./bin/test
xcrun llvm-profdata merge -sparse foo.profraw -o test.profdata
# xcrun llvm-cov show ./bin/test -instr-profile=test.profdata
xcrun llvm-cov report ./bin/test -instr-profile=test.profdata

popd


