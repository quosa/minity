# LLVM/Clang Profiling

```
Makefile:
	CXXFLAGS += -g -O0 -fprofile-instr-generate -fcoverage-mapping
	LDFLAGS += -fprofile-instr-generate

make clean
make minity
export LLVM_PROFILE_FILE=./llvm.prof
./bin/minity

xcrun llvm-profdata merge -output=llvm.merge -instr llvm.prof
xcrun llvm-profdata show -all-functions -counts -ic-targets llvm.merge > llvm.log
xcrun llvm-cov show ./bin/minity -instr-profile=llvm.merge > profiling.log
less profiling.log
```

## GUI profiling with Instruments

Instruments only needs the `-g` debug information to do the profiling.

```
make clean
make minity
instruments -l 30000 -t "Time Profile" -D instruments.memset.nodelay.trace ./bin/minity
open /Applications/Xcode.app/Contents/Applications/Instruments.app
```

Below are 2 simple cases to optimize (the unoptimized debug build) for a few more raw fps.

### Instruments Case 1
Just to learn the ropes, unoptimized (-O0) clearBuffer ate 30% of the CPU time.
![clearBuffer eating 30% of CPU time](./img/clearBuffer-eating-30-percent.png "clearBuffer eating 30% of CPU time")

Converting the for loops to memset dropped the impact 2ms/0%.
![clearBuffer down to 0% of CPU time](./img/clearBuffer-zero-percent.png "clearBuffer down to 0% of CPU time")

### Instruments Case 2
Again, to learn the unoptimized triangle filling took 880ms/7%.
![plotLine takes almost 7% in total](./img/plotLine-eating-3.6-percent.png "plotLine takes almost 7% in total")

Converting the simple flat shader to memset_pattern4 (for 32-bit color pattern filling) drove the triangle filling down to 80ms/0.7%.
![plotLine with memset_pattern4 down to 0.7% total](./img/plotLine-memset-0.2-percent.png "plotLine down to 0.7% of CPU time")

# References
 - https://newstudyclub.blogspot.com/2018/04/profiling-c-code-with-clang-using.html
 - https://apple.stackexchange.com/questions/197053/how-do-i-use-llvm-profdata-in-mac-os-x-yosemite?newreg=0305b56f785848478b41eb689c72bbe8