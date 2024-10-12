#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
pthread_mutex_t read_mutex;
pthread_mutex_t write_mutex;

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
#define BUFFER_SIZE 10
char buffer[BUFFER_SIZE];
int bufferIndex = 0;
unsigned long bufferSize = (sizeof(buffer) / sizeof(buffer[bufferIndex]));
int bufferOverflowFlag = 0;

pthread_mutex_t read_lock;
pthread_mutex_t write_lock;

void *writer(void *arg)
{
    char ch;
    for (;;)
    {

        if (!bufferOverflowFlag)
        {
            pthread_mutex_lock(&read_lock);
            pthread_mutex_lock(&write_lock);        // write lock not really necessary, as only one writer
            scanf("%c", &ch);

            if (ch != '\n') // check if input is from return char
            {
                buffer[bufferIndex] = ch;
                bufferIndex++;
                // here we can detect if buffer overflow will happen next cycle:
                if (bufferIndex == bufferSize - 1)
                {
                    bufferOverflowFlag = 1;
                    printf("buffer full! must wait for reader to consume a char before continuing.\n");
                }
            }
            pthread_mutex_unlock(&write_lock);
            pthread_mutex_unlock(&read_lock);
            usleep(1);
        }
    }

    return 0;
}
void *reader(void *arg)
{
    // maybe create local copies ? 
    for (;;)
    {
        pthread_mutex_lock(&read_lock);
        if (buffer[bufferIndex-1] != '0' && bufferIndex != 0)
        {
            printf("character entered: %c. | ", buffer[bufferIndex -1]);
            printf("buffer: %s\n", buffer);
            printf("%lu remaining spots in buffer.\n", bufferSize - bufferIndex);
            bufferIndex--;
            buffer[bufferIndex] = '0';
            bufferOverflowFlag = 0;
        }
        pthread_mutex_unlock(&read_lock);
        usleep(1);

    }

    return 0;
}
int main(void)
{
    printf("Read write program\n");
    memset(buffer, '0', sizeof(buffer));
    printf("buffer reset: %s.\n", buffer);

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

    // Initialize  mutex variables
    if (pthread_mutex_init(&read_lock, NULL) != 0)
    {
        printf("\n read lock init has failed\n");
        return 1;
    }

    if (pthread_mutex_init(&write_lock, NULL) != 0)
    {
        printf("\n Write lock init has failed\n");
        return 1;
    }

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
