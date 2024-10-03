#!/bin/bash
clear
cpu_count=`cat /proc/cpuinfo| grep "processor"| wc -l`
echo cpu count: ${cpu_count}
if [  ! -d "./build" ]; then
  mkdir build
fi

cd build
pwd

cmake -DCMAKE_CXX_FLAGS=--coverage -DCMAKE_C_FLAGS=--coverage ..
make clean all -j${cpu_count}
./gtest_rvcs_manager_test

cd CMakeFiles/gtest_rvcs_manager_test.dir/
find . -name "*.gcno" -exec sh -c 'gcov {} ' \;
lcov --capture --directory . --output-file gtest_coverage.info
lcov --remove gtest_coverage.info '*/include/third_party/*' '*/boost/*' '/usr/local/include/*' '/usr/include/*' '*/tests/*' -o gtest_coverage.info
genhtml gtest_coverage.info --output-directory CODE_COVERAGE

tar -zcvf rvcs_manager_CODE_COVERAGE.tar.gz CODE_COVERAGE 

mv rvcs_manager_CODE_COVERAGE.tar.gz ../../../
