/*
 * David Pearson
 * 1050197
 * CIS*3050 A1
 * 2022-10-12
 * A1SampleMain
 */

#include "A1Header.h"

// Comparison function for the ints
bool comIntPtrs(void* arg1, void* arg2) {
    int* n1 = (int*)arg1;
    int* n2 = (int*)arg2;
    if (*n1 == *n2) {
        return true;
    } else {
        return false;
    }
}

// Adds one to some int
void* add1int(void* arg) {
    int* num = arg;
    *num = *num + 1;

    return arg;
}

// Simulates a segfault
void* segFault(void* arg) {
    int* num = arg;
    *num = *num + 1;
    raise(SIGSEGV);

    return arg;
}

// Simulates an arithmetic error
void* div0int(void* arg) {
    int* num = arg;
    *num = *num / 1;
    raise(SIGFPE);

    return arg;
}

// Simulates an infinite loop requiring the user to SIGINT to exit
// This allows the testing of the signal handlers and graceful exit
void* infloop(void* arg) {
    while (1) {
    }

    return arg;
}

int main(void) {
    // Populate the test args
    TestArgs* t = malloc(sizeof(TestArgs));

    // Init the testing args
    t->logFileName = malloc(sizeof(char) * 100);
    strcpy(t->logFileName, "log.txt");
    t->logging = 1;
    t->timing = 1;

    // Init tests
    initUnitTests(t);

    free(t->logFileName);
    free(t);

    int expRes = 3;
    int testArg = 0;
    runTest(&expRes, add1int, &testArg, comIntPtrs);  // Correct termination, test fails

    testArg = 2;
    runTest(&expRes, add1int, &testArg, comIntPtrs);  // Correct termination, test passes

    runTest(&expRes, div0int, &testArg, comIntPtrs);  // Killed by floating point exception, test fails - Arithmetic error

    runTest(&expRes, segFault, &testArg, comIntPtrs);  // Killed by SIGSEVG, test fails - Seg fault

    runTest(&expRes, infloop, &testArg, comIntPtrs);  // Killed by SIGINT, test fails - Infinite loop

    cleanupTests();  // Required to free data and clean up file pointers

    return 0;
}
