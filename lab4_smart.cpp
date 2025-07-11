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
#define BUFFER_SIZE 3

using namespace std;

int buffer_counter = 0;

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

class SmartBuffer // FIRST IN FIRST OUT
{
    int buffer_idx;
    vector<char> buffer;
    int buffer_size;
    unsigned int counter;
    Condition notFull;
    Condition notEmpty;
    Monitor monitor;
    public:
    SmartBuffer() 
    {
        buffer_size = BUFFER_SIZE;
        buffer_counter++;
        buffer_idx = buffer_counter;
    }
    
    SmartBuffer(int idx)
    {
        buffer_size = BUFFER_SIZE;
        buffer_idx = idx;
        cout << "constructor done";
    }

    SmartBuffer(int idx, int length)
    {
        buffer_size = length;
        buffer_idx = idx;
    }
    void push(string name, char item)
    {
        monitor.enter();

        // WAIT
        while(counter == buffer_size)
        {
            monitor.wait(notFull);
        }
    
        // DO THE THING
        buffer.push_back(item);
        counter++;

        // PRINT (TO DELETE IN SHIP)
        green();
        cout << "----- ITEM PRODUCED -----\nBuffer: " << buffer_idx << " Producer: " << name << " Item: " << item << endl;
        for(auto i: buffer)
        {
            cout << i << " ";
        }
        cout << endl;

        monitor.signal(notEmpty);

        monitor.leave();
    }

    char pop(string name)
    {
        monitor.enter();

        // WAIT
        while(counter == 0)
        {
            monitor.wait(notEmpty);
        }
        
        // DO THE THING
        char taken_item = buffer[0];
        buffer.erase(buffer.begin());
        counter--;

        // PRINT (TO DELETE IN SHIP)
        red();
        cout << "----- ITEM CONSUMED -----\nBuffer: " << buffer_idx << " Consumer: " << name << " Item: " << taken_item << endl;
        for(auto i: buffer)
        {
            cout << i << " ";
        }
        cout << endl;

        monitor.signal(notFull);

        monitor.leave();
        return taken_item;
    }
};
