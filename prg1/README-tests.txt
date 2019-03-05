Short explanation of test files:

Files named "...-in.txt" are input files containing commands for different tests.

Files named "...-out.txt" are output files containing expected output of the corresponding input.

You can either just run the input file with command read "...-in.txt" or compare your own output with the expected output with command testread "...-in.txt" "...-out.txt" (the graphical UI's file selector allows you to select both files at once by control-clicking with mouse).

Files named "performance-....txt" are performance tests of various types of operations. They can be run with command read "performance-....txt".

Correctness tests:
- simpletest-compulsory/all-in/out.txt: Really simple test to find most obvious bugs
- example-compulsory/all-in/out.txt: The example in the assignment description, showing basic operation of all commands
- basictest-compulsory/all-in/out.txt: A little bit more testing with example's data
- biggertest-compulsory/all-in/out.txt: Somewhat bigger test with more beacons

Performance tests:
- perftest-sorting.txt: Test sorting operations, adding beacons in between
- perftest-minmax.txt: Test min/max brightness, adding beacns in between
- perftest-change.txt: Test change_name/change_color
- perftest-outbeam.txt: Test path_outbeam
- perftest-compulsory.txt: Test all of the above
- perftest-inbeam.txt: Test non-compulsory path_inbeam
- perftest-totalcolor.txt: Test non-compulsory total_color
- perftest-all.txt: Test all of the above (compulsory+non-compulsory)

More info on testing on the course web pages!
