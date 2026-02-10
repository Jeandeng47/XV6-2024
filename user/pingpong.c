#include "kernel/types.h"
#include "user/user.h"

/*
 * parent: send 1 byte to child
 * child:  print "<pid>: received ping", send 1 byte back, exit
 * parent: read 1 byte back, print "<pid>: received pong", exit
 */
int main(int argc, char* argv[]) {
    int pipe1[2]; 
    int pipe2[2]; 
    char buf[1];

    // After pipe, parent:
    // pipe1: fd [ r1 | w1 ]
    // pipe2: fd [ r2 | w2 ]

    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        fprintf(2, "err: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    // After fork():
    //        parent            child
    // pipe1: fd [ r1 | w1 ]    fd [ r1 | w1 ]
    // pipe2: fd [ r2 | w2 ]    fd [ r2 | w2 ]
    if (pid < 0) {
        fprintf(2, "err: fork failed\n");
        exit(1);
    }

    // child & parent exec diff logics
    if (pid == 0) {
        //        parent          |  child
        // pipe1: fd [ r1 | w1 ]  |  fd [ r1 | -- ]
        // pipe2: fd [ r2 | w2 ]  |  fd [ -- | w2 ]
        close(pipe1[1]);
        close(pipe2[0]);

        // read from parent
        int n = read(pipe1[0], buf, 1);
        if (n != 1) {
            fprintf(2, "child read failed\n");
            exit(1);
        }

        fprintf(1, "%d: received ping\n", getpid());
        
        // send back 1 byte
        n = write(pipe2[1], buf, 1);
        if (n != 1) {
            fprintf(2, "child write failed\n");
            exit(1);
        }

        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);

    } else if (pid > 0) {
        //        parent          |  child
        // pipe1: fd [ -- | w1 ]  |  fd [ r1 | w1 ]
        // pipe2: fd [ r2 | -- ]  |  fd [ r2 | w2 ]
        close(pipe1[0]);
        close(pipe2[1]);

        // write to child
        buf[0] = 'x';
        int n = write(pipe1[1], buf, 1);
        if (n != 1) {
            fprintf(2, "parent write failed\n");
            exit(1);
        }

        // read 1 byte
        n = read(pipe2[0], buf, 1);
        if (n != 1) {
            fprintf(2, "parent read failed\n");
            exit(1);
        }

        fprintf(1, "%d: received pong\n", getpid());

        close(pipe1[1]);
        close(pipe2[0]);
        
        wait(0); // wait for child;
        exit(0);
    }    

}