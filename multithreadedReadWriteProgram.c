#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
//----------------------------------------------------------------
// read write, to shared buffer program.
// purpose: this program spawns two threads, one for writing to a buffer, and one for reading from the same buffer.
// Program follows the following rules:
// Writer thread:
//     1. Reads one character at a time from keyboard.
//     2. Adds the character to the buffer.
// Reader thread:
//     1. Reads one character at a time from the buffer.
//     2. Removes the character from the buffer.
//     3. Displays the character that is removed along with the number of remaining characters in the buffer.
//----------------------------------------------------------------

// Using circular buffer, plus three counters, one for writer to keep track of next available char,
// one for reader to keep track of the next char to read.
// one to keep track of the buffer index

#define BUFFER_SIZE 10
char buffer[BUFFER_SIZE];
int32_t bufferIndex = 0; // number of characters in the buffer.
int32_t writeIndex = 0;     // index for the writer thread to keep track of position
int32_t readIndex = 0;      // index for the reader thread to keep track of position
uint32_t bufferSize = (sizeof(buffer) / sizeof(buffer[writeIndex]));
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// signals for the threads can communicate if the buffer is full or empty, taking them out of wait state.
pthread_cond_t NOT_FULL_SIGNAL = PTHREAD_COND_INITIALIZER;
pthread_cond_t NOT_EMPTY_SIGNAL = PTHREAD_COND_INITIALIZER;

void *writer(void *arg)
{
    char ch;
    for (;;)
    {
        //printf("Enter a char: \n");
        scanf("%c", &ch);

        if (ch != '\n' && ch != '\t') // check if input is a new line or return char.
        {
            pthread_mutex_lock(&lock);

            // if the buffer is full, wait for space to free up.
            while (bufferIndex == BUFFER_SIZE)
            {
                pthread_cond_wait(&NOT_FULL_SIGNAL, &lock);
            }

            buffer[writeIndex] = ch;
            writeIndex = (writeIndex + 1) % BUFFER_SIZE;
            bufferIndex++;
            pthread_cond_signal(&NOT_EMPTY_SIGNAL); // Signal reader that buffer is not empty
            pthread_mutex_unlock(&lock);
        }
    }
}

void *reader(void *arg)
{
    char readChar;
    for (;;)
    {
        pthread_mutex_lock(&lock); 

        // Wait if buffer is empty
        while (bufferIndex == 0)
        {
            pthread_cond_wait(&NOT_EMPTY_SIGNAL, &lock);
        }

        readChar = buffer[readIndex];
        readIndex = (readIndex + 1) % BUFFER_SIZE;
        printf("character entered:    |%c|  ", readChar);
        printf("buffer: |%s| \n", buffer);
        printf("Remaining characters: |%d|\n", BUFFER_SIZE-bufferIndex);
        bufferIndex--;
        pthread_cond_signal(&NOT_FULL_SIGNAL); // Signal writer that buffer is not full
        pthread_mutex_unlock(&lock);

    }

    return 0;
}
int main(void)
{
    printf("Read write program\n");
    memset(buffer, '0', sizeof(buffer));
    printf("buffer reset: %s.\n", buffer);
    printf("Enter some characters!\n");

    // the following changes terminal settings to automatically enter a character without pressing return.
    // ----------------------------------------------------------------
    // static struct termios oldt, newt;
    // /*tcgetattr gets the parameters of the current terminal
    // STDIN_FILENO will tell tcgetattr that it should write the settings
    // of stdin to oldt*/
    // tcgetattr(STDIN_FILENO, &oldt);
    // /*now the settings will be copied*/
    // newt = oldt;
    // /*ICANON normally takes care that one line at a time will be processed
    // that means it will return if it sees a "\n" or an EOF or an EOL*/
    // newt.c_lflag &= ~(ICANON | ECHO);
    // /*Those new settings will be set to STDIN
    // TCSANOW tells tcsetattr to change attributes immediately. */
    // tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    // ----------------------------------------------------------------

    pthread_t thread_id_write, thread_id_read;
    if (pthread_create(&thread_id_write, NULL, &writer, NULL))
    {
        printf("error creating thread.");
        abort();
    }
    if (pthread_create(&thread_id_read, NULL, &reader, NULL))
    {
        printf("error creating thread.");
        abort();
    }

    if (pthread_join(thread_id_write, NULL))
    {
        printf("error joining thread.");
        abort();
    }
    if (pthread_join(thread_id_read, NULL))
    {
        printf("error joining thread.");
        abort();
    }
    /*restore the old terminal settings. (needed if terminal settings are altered)*/
    // tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return 0;
}
