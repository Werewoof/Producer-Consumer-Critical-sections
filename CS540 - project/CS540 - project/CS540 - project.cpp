#include "stdafx.h" //Remove if not using Visual Studio


#include <windows.h>
#include <process.h>
#include <iostream>
#include <time.h>
#include "buffer.h"
#include "conio.h"
using namespace std;


DWORD WINAPI Producer(LPVOID param);
DWORD WINAPI Consumer(LPVOID param);


HANDLE	Mutex = CreateMutex(NULL, FALSE, NULL);			
HANDLE	Empty = CreateSemaphore(0, BUFFER_SIZE, BUFFER_SIZE, 0);	
HANDLE	Full = CreateSemaphore(0, 0, BUFFER_SIZE, 0);		

//buffer_item is defined as in int in "buffer.h"
buffer_item	buffer[BUFFER_SIZE];
int	front = 0;
int	back = 0;
int kount = 0; //Visual Studio complained about ambiguity or something


int insertItem(buffer_item prod, int& id);
int removeItem(buffer_item* item, int& id);


int main(int argc, char **argv)
{
	DWORD	ThreadId;


	//Make sure we've got arguments passed in to the program
	if (argc < 3)
	{
		cout << "Please give 3 arguments for the command line!\n"
		<< "1st: how long to sleep (in milliseconds) after completion\n"
		<< "2nd: how many producer threads to create\n" 
		<< "2nd: how many consumer threads to create\n\n"
		<< "Press any key to exit.";
		_getch();
	}

	//Use the passed-in arguments!
	int sleepTime = strtol(argv[1], NULL, 0);
	int numOfProducers = strtol(argv[2], NULL, 0);
	int numOfConsumers = strtol(argv[3], NULL, 0);


	HANDLE* Producers = new HANDLE[numOfProducers];
	HANDLE* Consumers = new HANDLE[numOfConsumers];
	int* producerIDs = new int[numOfProducers];
	int* consumerIDs = new int[numOfConsumers];

	//Generate an array of producers based on the amount passed in.
	for (int i = 0; i < numOfProducers; i++) {
		producerIDs[i] = i;
		Producers[i] = CreateThread(0, 0, Producer, &producerIDs[i], 0, &ThreadId);
		cout << "Making producer #" << i << endl;
	}

	//Generate an array of consumers based on the amount passed in.
	for (int i = 0; i < numOfConsumers; i++) {
		consumerIDs[i] = i;
		Consumers[i] = CreateThread(0, 0, Consumer, &consumerIDs[i], 0, &ThreadId);
		cout << "Making consumer #" << i << endl;
	}

	for (int i = 0; i < numOfProducers; i++) {
		cout << "\nWaiting on producer " << i << "...\n";
		WaitForSingleObject(Producers[i], INFINITE);
	}

	cout << endl;

	for (int i = 0; i < numOfConsumers; i++) {
		cout << "\nWaiting on consumer " << i << "...\n";
		WaitForSingleObject(Consumers[i], INFINITE);
	}
	cout << endl;

	delete[] Producers;
	delete[] Consumers;

	cout << "\n\n\nDone!\nExiting after "<<sleepTime<<" ms...";
	Sleep(sleepTime);
}



//Deals with the producer threads. Calls the insertItem function,
//which in turn makes sure an item can be inserted/produced.
DWORD WINAPI Producer(LPVOID param)
{
	int	id = *(DWORD *)param;
	srand(time(NULL) + id);

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		buffer_item prod = (rand() % 4000);
		Sleep(rand() % 4000);

		WaitForSingleObject(Empty, INFINITE);
		WaitForSingleObject(Mutex, INFINITE);

		if (insertItem(prod, id))
			cout<<"!Error occured, producer "<< id <<endl;
		else
			cout<<"*Producer "<<id<< " produced "<< prod << endl;

		ReleaseMutex(Mutex);
		ReleaseSemaphore(Full, 1, 0);
	}

	return 0;
}

//Deals with the consumer threads. Calls the removeItem function,
//which in turn makes sure an item can be removed/consumed.
DWORD WINAPI Consumer(LPVOID param) {

	int	id = *(DWORD *)param;

	srand(time(NULL)+id);

		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			buffer_item	cons;
			Sleep(rand() % 4000);

			WaitForSingleObject(Full, INFINITE);
			WaitForSingleObject(Mutex, INFINITE);


			if (removeItem(&cons, id))
				cout << " !Error occured, consumer " << id << "\n";
			else
				cout << " /Consumer " << id << " consumed " << cons << endl;

			ReleaseMutex(Mutex);
			ReleaseSemaphore(Empty, 1, 0);

		}

		return 0;
}

//Deals with the inserting items in the buffer. 
//Called by producer threads.
int insertItem(buffer_item prod, int& id) {
	int success;


	if (kount < BUFFER_SIZE)
	{
		buffer[front] = prod;
		front = (front + 1) % BUFFER_SIZE;
		kount++;
		success = 0;
	}
	else {
		success = -1;
	}

	return success;
}

//Deals with the removing items from the buffer. 
//Called by consumer threads.
int removeItem(buffer_item* cons, int& id) {
	int success;

	if (kount < BUFFER_SIZE)
	{
		Sleep(rand() % 1000);

		*cons = buffer[back];
		back = (back + 1) % BUFFER_SIZE;

		kount--;
		success = 0;
	}
	else {
		success = -1;
	}
	
	return success;
}