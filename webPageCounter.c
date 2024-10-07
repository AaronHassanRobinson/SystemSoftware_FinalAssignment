#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
// repeat 100 times to mimic 100 random visits to the page
#define RPT 100
//web page visit counter
int count=0;
pthread_mutex_t lock; 
void* counter() {
    int countLocalCopy;
    float r;    
    pthread_mutex_lock(&lock); 
    countLocalCopy = count;
    // mimicking the work of the sever in serving the page to
    // the browser
    r = rand() % 2000; usleep(r);
    count = countLocalCopy + 1;
    pthread_mutex_unlock(&lock); 

    return NULL;
}
int main () {

    // Initialize mutex, aswell as log any failures
    if (pthread_mutex_init(&lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
    int i;
    float r;
    pthread_t tid[RPT];
    // seed the random number sequence
    srand(time(NULL));
    for (i=0; i<RPT; i++) {
        // mimicking the random access to the web page
        r = rand() % 2000; usleep(r);
        // a thread to respond to a connection request
        pthread_create (&tid[i], NULL, &counter, NULL);
    }
    // Wait till threads complete.
    for (i=0; i<RPT; i++) {
        pthread_join(tid[i], NULL);
    }
    // print out the counter value and the number of mimicked visits
    // the 2 values should be the same if the program is written
    // properly
    printf ("count=%d, repeat=%d\n", count, RPT);
}