#include "monitor.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/prctl.h>
#include <vector>
#include <iostream>
#include <string>
#define TRUE 1

using namespace std;

const int BUFFER_NUM = 4;
const int BUFFER_SIZE = 10;
char buffers[BUFFER_NUM][BUFFER_SIZE];
unsigned int counter[BUFFER_NUM];   //how many items in buffer

Condition notFull[BUFFER_NUM];
Condition notEmpty[BUFFER_NUM];
Monitor monitor;

char choose_item(unsigned int item_seed)
{
    item_seed = (item_seed - 1) % 4 + 1;
    vector<char> items;
    switch (item_seed)
    {
        case 1:
            items = {'1', '2', '3'};
            return items[rand() % 3];
            break;
        case 2:
            items = {'A', 'B', 'C'};
            return items[rand() % 3];
            break;
        case 3:
            items = {'b', 'a', 'c'};
            return items[rand() % 3];
            break;  
        case 4:
            items = {'7', '8', '9'};
            return items[rand() % 3];
            break;
        default:
            return '-';
    }
}

void blue()
{
  printf("\033[0;34m");
}

void red()
{
  printf("\033[0;31m");
}

void green()
{
  printf("\033[0;32m");
}

void producer_produce(string name, char item, size_t buffer_idx)
{
    monitor.enter();

    // WAIT
    while(counter[buffer_idx] == BUFFER_SIZE)
    {
        monitor.wait(notFull[buffer_idx]);
    }
 
    // DO THE THING
    buffers[buffer_idx][counter[buffer_idx]++] = item;

    // PRINT
    green();
    cout << "----- ITEM PRODUCED -----\nBuffer: " << buffer_idx << " Producer: " << name << " Item: " << item << endl;
    cout << buffers[buffer_idx] << endl;

    monitor.signal(notEmpty[buffer_idx]);

    monitor.leave();
}

void consumer_consume(string name, size_t buffer_idx)
{
    monitor.enter();

    // WAIT
    while(counter[buffer_idx] == 0)
    {
        monitor.wait(notEmpty[buffer_idx]);
    }
    
    // DO THE THING
    counter[buffer_idx]--;
    char taken_item = buffers[buffer_idx][0];
    for(int i = 1;i < BUFFER_SIZE;i++)         // FIFOQUEUE MANIPULATION
    {
        buffers[buffer_idx][i-1] = buffers[buffer_idx][i];
    }

    // PRINT
    red();
    cout << "----- ITEM CONSUMED -----\nBuffer: " << buffer_idx << " Consumer: " << name << " Item: " << taken_item << endl;
    cout << buffers[buffer_idx] << endl;

    monitor.signal(notFull[buffer_idx]);

    monitor.leave();
}

void producer_run(string name, int item_seed, vector<int> buffers_idxs)
{
    char item;
    while(TRUE)
    {
        item = choose_item(item_seed);
        producer_produce(name, item, buffers_idxs[rand() % buffers_idxs.size()]);
        sleep(1);
    }
}

void consumer_run(string name, vector<int> buffers_idxs)
{
    while(TRUE)
    {
        consumer_consume(name, buffers_idxs[rand() % buffers_idxs.size()]);
        sleep(1);
    }
}

struct producer_args
{
    string name;
    int item_seed;
    vector<int> buffers_idxs;
};

struct consumer_args
{
    string name;
    vector<int> buffers_idxs;
};

void* producerThread(void* fargs)
{
    producer_args* args = static_cast<producer_args*>(fargs);
    string name = args->name;
    int item_seed = args->item_seed;
    vector<int> buffers_idxs = args->buffers_idxs;
    producer_run(name, item_seed, buffers_idxs);
    return nullptr;
}

void* consumerThread(void* fargs)
{
    consumer_args* args = static_cast<consumer_args*>(fargs);
    string name = args->name;
    vector<int> buffers_idxs = args->buffers_idxs;
    consumer_run(name, buffers_idxs);
    return nullptr;
}

int main()
{
    srand(time(NULL));
    
    pthread_t p1, p2, p3, c1, c2, c3, c4;
    
    vector<int> p1_buffers = {0, 2};
    producer_args* arg_p1 = new producer_args{"A", 1, p1_buffers};
    pthread_create(&p1, nullptr, producerThread, arg_p1);

    vector<int> p2_buffers = {0, 1, 2, 3};
    producer_args* arg_p2 = new producer_args{"B", 2, p2_buffers};
    pthread_create(&p2, nullptr, producerThread, arg_p2);

    vector<int> p3_buffers = {3};
    producer_args* arg_p3 = new producer_args{"C", 3, p3_buffers};
    pthread_create(&p3, nullptr, producerThread, arg_p3);

    vector<int> c1_buffers = {0};
    consumer_args* arg_c1 = new consumer_args{"Adam", c1_buffers};
    pthread_create(&c1, nullptr, consumerThread, arg_c1);

    vector<int> c2_buffers = {1};
    consumer_args* arg_c2 = new consumer_args{"Bartek", c2_buffers};
    pthread_create(&c2, nullptr, consumerThread, arg_c2);

    vector<int> c3_buffers = {2};
    consumer_args* arg_c3 = new consumer_args{"Cecyl", c3_buffers};
    pthread_create(&c3, nullptr, consumerThread, arg_c3);

    vector<int> c4_buffers = {3};
    consumer_args* arg_c4 = new consumer_args{"Damian", c4_buffers};
    pthread_create(&c4, nullptr, consumerThread, arg_c4);

    while(TRUE)
    {

    }
}
