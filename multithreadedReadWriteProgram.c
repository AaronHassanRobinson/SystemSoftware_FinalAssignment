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
    for (;;)
    {
        char ch;
        if (bufferOverflowFlag == 0)
        {
            scanf("%c", &ch);
        }
        pthread_mutex_lock(&write_mutex);
        pthread_mutex_lock(&read_mutex);
        if (ch != '\n')
        {
            if (bufferIndex >= bufferSize)
            {
                printf("buffer overflow! must wait for reader to consume a char.\n");
                bufferOverflowFlag = 1;
                pthread_mutex_unlock(&write_mutex);
                pthread_mutex_unlock(&read_mutex);
            }
            else
            {
                bufferOverflowFlag = 0;
                bufferIndex++;
                buffer[bufferIndex] = ch;
            }
        }
        pthread_mutex_unlock(&read_mutex);
        pthread_mutex_unlock(&write_mutex);
    }
    return 0;
}
void *reader(void *arg)
{
    for (;;)
    {
        if (buffer[bufferIndex] != '0')
        {
            pthread_mutex_lock(&read_mutex);
            printf("character entered: %c. | ", buffer[bufferIndex]);
            printf("%lu remaining spots in buffer.\n", bufferSize - bufferIndex);
            pthread_mutex_unlock(&read_mutex);
            pthread_mutex_lock(&write_mutex);
            buffer[bufferIndex] = '0';
            bufferIndex--;
            pthread_mutex_unlock(&write_mutex);
        }
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
        printf("\n read mutex init has failed\n");
        return 1;
    }

    if (pthread_mutex_init(&write_lock, NULL) != 0)
    {
        printf("\n Write mutex init has failed\n");
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
