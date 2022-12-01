/*
 * David Pearson
 * 1050197
 * CIS*3050 A1
 * 2022-10-12
 * utest
 */

#include "A1Header.h"

// Global status for the testing lib
bool timing = 0;
bool logging = 0;
bool libinit = 0;
static int testID = 1;
pid_t c_pid = 0;
FILE* fptr = NULL;

void initUnitTests(TestArgs* arg) {
    // init signal handling in parent
    if (signal(SIGINT, parent_sigint) == SIG_ERR)  // SIGINT handler
        fprintf(stderr, "Can't catch SIGINT");
    if (signal(SIGUSR1, sigusr_handle) == SIG_ERR)  // This is used to let the parent know test passed
        fprintf(stderr, "Can't catch SIGUSR1");
    if (signal(SIGUSR2, sigusr_handle) == SIG_ERR)  // Test failed signal
        fprintf(stderr, "Can't catch SIGUSR2");

    // This is a saftey var to ensure the things are init before we try to access.
    libinit = 1;        // Once init has been called we can use run test
    if (arg == NULL) {  // Set Defaults
        timing = 0;
        logging = 0;
        return;
    }

    // If arg != NULL
    timing = arg->timing;
    logging = arg->logging;
    // If logging is enabled, open file stream
    if (logging) {
        if (arg->logFileName == NULL) {  // Open default filename
            fptr = freopen("defaultLog.txt", "w", stderr);
        } else {  // Open user spec
            fptr = freopen(arg->logFileName, "w", stderr);
        }
        // We will output all testing results to the file
        // To make this easier, relevant testing data is output to stderr so we redirect that file stream
        if (fptr == NULL) {
            printf("Failed to open filestream\n");
            _exit(0);
        } else {  // Let the user know that test outputs will go to the file and not terminal
            printf("Logging is now enabled, all testing outputs will be redirected to file\n");
        }
    }
}

void cleanupTests(void) {
    if (fptr) {  // Dont close if not open
        fclose(fptr);
    }
}

void runTest(void* refValue, testCase* testFunc, void* testArg, equalComp* compFunc) {
    // Validate that the args are all valid
    if (refValue == NULL) {
        fprintf(stderr, "refValue must be defined!\n");
        return;
    }
    if (testFunc == NULL) {
        fprintf(stderr, "testFunc must be defined!\n");
        return;
    }
    if (testArg == NULL) {
        fprintf(stderr, "testArg must be defined!\n");
        return;
    }
    if (compFunc == NULL) {
        fprintf(stderr, "compFunc must be defined!\n");
        return;
    }

    // If the user has not run initUnitTests then necessary data is not usable
    if (!libinit) {
        fprintf(stderr, "You must initialize the testing lib before continuing!\n");
        return;  // We return to the main function to allow the user to possibly init later on
    }

    fprintf(stderr, "TEST ID [%d]\n", testID);
    // Lets the user know which tests are running, useful in case of a hanging test case
    // This will let the user know when logging is enabled when to issue SIGINT
    if (logging) {
        printf("Initiating Test [%d]\n", testID);
    }

    int status = 0;
    // Start timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    pid_t pid = fork();  // spwan child
    c_pid = pid;

    if (pid == 0) {  // Child

        signal(SIGINT, SIG_IGN);  // ignore sig int so we can deal with it elsewhere

        void* resVal = testFunc(testArg);  // Run test
        // We can use signals to indicate to the parent if the test fail/pass
        // Another way to handle this would be different exit codes but thats no fun
        if (compFunc(refValue, resVal))
            kill(getppid(), SIGUSR1);  // Pass
        else
            kill(getppid(), SIGUSR2);  // Fail

        _exit(0);

    } else if (pid > 0) {  // Parent

        int wpid, err;
        // Here in instances where we get an interrupt, waitpid returns an error
        // so while we are in the interrupt, we must keep checking for errono EINTR
        // This took something like 3 hours to find the solution too
        do {
            wpid = waitpid(c_pid, &status, 0);
            err = errno;
        } while ((wpid == -1) && (err = EINTR));

        if (wpid == -1) {
            printf("Failed to wait for init process to end\n");
        }
        // end timer
        gettimeofday(&end, NULL);
        // If there was an unusual termination
        if (WIFSIGNALED(status)) {
            fprintf(stderr, "FAIL\n");
            char* exitSignalStr = strsignal(WTERMSIG(status));
            fprintf(stderr, "Child %d exited with status=%d (%s).\n", wpid, WEXITSTATUS(status), exitSignalStr);
        }
        // Calculate time taken in exec
        if (timing) {
            long seconds = (end.tv_sec - start.tv_sec);
            long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
            fprintf(stderr, "Time taken: %lds %ldms\n", seconds, micros);
        }
        // Make it look purdy
        fprintf(stderr, "-----------\n");
        // On sigkill after a SIGINT we must clean and exit
        if (WTERMSIG(status) == SIGKILL) {  // User opted to end testing on hung test case, sigkill
            cleanupTests();
            _exit(0);
            printf("Unreach\n");
        }

    } else {
        fprintf(stderr, "Failed to fork!");
    }

    testID++;
}
// Catch sigusers for pass fail
void sigusr_handle(int signo) {
    // Reinstall
    if (signal(SIGUSR1, sigusr_handle) == SIG_ERR)
        fprintf(stderr, "Can't catch SIGUSR1");
    if (signal(SIGUSR2, sigusr_handle) == SIG_ERR)
        fprintf(stderr, "Can't catch SIGUSR2");

    if (signo == SIGUSR1) {  // pass
        fprintf(stderr, "PASS - Correct Termination and return value\n");
    } else if (signo == SIGUSR2) {  // fail
        fprintf(stderr, "FAIL - Return value did not match expected\n");
    }
}

void parent_sigint(int signo) {
    if (signal(SIGINT, parent_sigint) == SIG_ERR) {
        fprintf(stderr, "Unable to catch SIGINT\n");
    }

    printf("\nTest [%d] has been halted\n", testID);
    kill(c_pid, SIGSTOP);  // Pause process execution of child c_pid

    // Get user input
    int cont = 0;
    bool valid = 0;
    printf("Enter 1 to continue, 0 to exit: ");
    while (!valid) {  // Wait for valid int input
        scanf("%d", &cont);
        if (cont == 1 || cont == 0) {
            valid = 1;
        } else {
            printf("Enter 1 to continue, 0 to exit: ");
        }
    }
    // Resume testing
    if (cont == 1) {
        kill(c_pid, SIGCONT);  // sigcont to child process
        printf("Test [%d] has resumed\n", testID);
    } else {  // Kill the test
        printf("Terminating...\n");
        kill(c_pid, SIGCONT);
        kill(c_pid, SIGKILL);
    }
}
