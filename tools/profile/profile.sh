#! /bin/bash

# # do changes to makefile
# make clean all
# export LLVM_PROFILE_FILE=./llvm.prof
# ./main

# xcrun llvm-profdata merge -output=llvm.merge -instr llvm.pr
# xcrun llvm-profdata show -all-functions -counts -ic-targets
# xcrun llvm-cov show ./main -instr-profile=llvm.merge > prof
# echo see profiling.log

root=`git rev-parse --show-toplevel`
pushd "${root}"

rm -rf 'instruments.memset.nodelay.trace'
make clean all
xcrun xctrace record --time-limit 5m --template "Time Profile" --output 'instruments.memset.nodelay.trace' --launch -- ./bin/minity
open -a instruments 'instruments.memset.nodelay.trace'

echo See report in instruments app
popd


