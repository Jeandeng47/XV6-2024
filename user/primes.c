#include "kernel/types.h"
#include "user/user.h"

static void primes(int) __attribute__((noreturn));
static void primes(int r1) {
    int prime; // read prime passed from left
    
    // base case: if no num from left, stop recursion
    if (read(r1, &prime, sizeof(int)) != sizeof(int)) {
        close(r1);
        exit(0);
    }
    printf("prime %d\n", prime);

    // recursive case: creat new pipe to pass num to right
    int p2[2];
    if (pipe(p2) < 0) exit(1);

    //         c            
    // pipe:  [ r1 | -- ]  
    // pipe2: [ r2 | w2 ] 

    if (fork() == 0) {
        // grandchild: wait for child to pass filtered nums
        //         c             gc
        // pipe:  [ r1 | -- ] |  [ xx | -- ]
        // pipe2: [ r2 | w2 ] |  [ r2 | xx ]
        close(r1);
        close(p2[1]);
        primes(p2[0]); // no-return
        exit(0);
    }

    // child: read first prime(=2) from parent, filter out
    // nums(3,4,5,6,7...), pass results to grandchild

    //         c             gc
    // pipe:  [ r1 | -- ] |  [ r1 | -- ]
    // pipe2: [ xx | w2 ] |  [ r2 | w2 ]
    int num;
    close(p2[0]);
    while (read(r1, &num, sizeof(int)) == sizeof(int)) {
        if (num % prime != 0) {
            write(p2[1], &num, sizeof(int));
        }
    }
    close(r1);
    close(p2[1]);

    wait(0);
    exit(0);

}

int main(int argc, char* argv[]) {
    int p[2];
    if(pipe(p) < 0) exit(1);

    if (fork() == 0) {
        // child: read prime
        //        p             c
        // pipe: [ r1 | w1 ] |  [ r1 | -- ]
        close(p[1]);
        primes(p[0]);

        exit(0);
    }

    // parent: feed nums to child
    //        p             c
    // pipe: [ -- | w1 ] |  [ r1 | w1 ]
    close(p[0]);
    for (int i = 2; i <= 280; i++) {
        write(p[1], &i, sizeof(int));
    }
    close(p[1]);

    wait(0);
    exit(0);
    
}