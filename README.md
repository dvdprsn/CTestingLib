# CIS\*3050 Unit Testing Library

> David Pearson
> 1050197
> 2022-10-12

## Program information

**Makefile**

- target: libutest.so
  - This target will compile utest.c using -fpic then use the generated .o file to create the shared library libutest.so with the -shared flag
- target: A1SampleMain
  - This target creates the executable a.out by creating the main.o file and linking it with -lutest -L. flags

**Running Instructions**

1. The user can run command 'make' and this will run all targets creating executable 'a.out'
   - Alternatively, the user can run 'make A1SampleMain' to generate 'a.out' as this uses the target 'libutest.so' as a dependency
2. run ./a.out
3. This will run the example test suite of 5 tests, each of which demonstrates a different function of the test harness.

**The 5 example test**

- Test 1 demonstrates correct termination, but the comparison fails - Test Fail
- Test 2 demonstrates correct termination where the comparison succeeds - Test Pass
- Test 3 demonstrates a function being killed by the OS, in this case we simulate an arithmetic error (div by 0) - Test fails and the harness proceeds to the next test
- Test 4 demonstrates a function being killed by the OS, in this case we simulate a SEGFAULT - Test fails and the harness proceeds to the next test
- Test 5 is the final test as we require the user to initiate a SIGINT (ctrl-c) to exit. This test simulates an infinite loop. The program, even when logging is enabled, notifies the user to terminal before each test is started so when 'test [5]' never finishes the user can see there is an issue. By issuing a SIGINT, the program will catch it, pause the execution of the process, and wait for the user to continue or exit the program. Upon continuing the process is resumed, the user can issue SIGINT again to halt once more. If the user opts to exit the program by inputting '0', a graceful shutdown is initiated. The process that is hung will be killed with -9 (SIGKILL) then the program will complete its logging information, either saving to file or outputting to console, if timing is enabled this will reflect total run time of this hung process. Logging information is saved to the file if enabled and the filestream is closed. Global allocated variables are freed and the program exits. The user will be able to see testing information about this hung process as this is saved before exit.

## Program Capabilities
- When logging is not enabled test results are still output to the terminal, this is under the assumption that our suite should work similar to something like valgrind. With valgrind when logging is not enabled results are output to the console, if --logging-enabled=yes is included valgrind outputs the bare minimum and results are printed to the file.
- If testArgs is null, the defaults are set: timing = false, logging = false.
- If logging is enabled, and logFileName is NULL, the default filename is used 'deafultLog.txt', otherwise the user can specify.
- runTest validates that initUnitTests has been called and that function arguments are NOT NULL, returns otherwise
- SIGINT handling to enable a graceful shutdown
- If a test is hungup and SIGINT exits the hung process will show (killed) in the logging information
- An assumption is that since TestArgs is specified by the user outside of the library that the user is responsible for freeing this.

