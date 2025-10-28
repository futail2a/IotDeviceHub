# How to run UT
* Create build directory under project root directory
```
$mkdir build
```
* Move to build directory, and run cmake
```
$cd build
```
```
$cmake ..
```
```
$make all
```
```
$make run-test
```
* Generate coverage report
```
$lcov -d . -c -o coverage.info
$lcov -r coverage.info */googletest/* test/* */c++/* -o coverageFiltered.info
$genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info
```

# Reference
https://qiita.com/iydmsk/items/0021d1ef14660184f396
