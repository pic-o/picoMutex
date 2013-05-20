/***********************************************************************************************************************************************************
Original Web Link:
http://blog.pic-o.com/

Personal request:
Drop me an email if you do use this / encounter problems =]
picoCreator (Eugene Cheah Hui Qin | pico.creator{at}gmail.com)

License Type:
BSD 3-Clause (This is both "GPL" and "commercial with acknowledgement" compatible)

Copyright (c) 2013, picoCreator (Eugene Cheah Hui Qin | pico.creator{at}gmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

+ Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
+ Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
+ Neither the name of ANY ORGANIZATION(S) AFFLIATED WITH THE CREATOR (picoCreator) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////
// The following is a demo file, showing the usage of picoMutex and its alternatives //
///////////////////////////////////////////////////////////////////////////////////////
#define THIS_IS_MAIN_FILE

#include "picoMutex.hpp"
#include <iostream> //std::cout
#include <thread> //std::thread
#include <time.h> //std::clock
#include <vector> //std::vector
#include <mutex> //std::mutex

#include "x_semaphore.hpp" //x_semaphore

#include "charTools.hpp"
using namespace std;

int trdMax = 2;
int ptMax = 10000000; //10 million

int finalValue;
picoMutex valuePicoMutex;
std::mutex valueStdMutex;
x_semaphore valueSemaphore;

vector<int> threadReturn;
vector<thread*> threadVector;

void countThread(int thrdPt) {
	int added = 0;
	while( finalValue < ptMax ) {
		added++;
		finalValue++;
	}
	threadReturn[thrdPt] = added;
}

void countThread_picoMutexed(int thrdPt) {
	int added = 0;
	while( true ) {
		valuePicoMutex.lock();
		if( finalValue >= ptMax ) {
			valuePicoMutex.unlock();
			threadReturn[thrdPt] = added;
			return;
		}
		finalValue++;
		added++;
		valuePicoMutex.unlock();
	}
	threadReturn[thrdPt] = -1;
	return;
}

void countThread_semaphore(int thrdPt) {
	int added = 0;
	while( true ) {
		valueSemaphore.lock();
		if( finalValue >= ptMax ) {
			valueSemaphore.unlock();
			threadReturn[thrdPt] = added;
			return;
		}
		finalValue++;
		added++;
		valueSemaphore.unlock();
	}
	threadReturn[thrdPt] = -1;
	return;
}

void countThread_stdMutexed(int thrdPt) {
	int added = 0;
	while( true ) {
		valueStdMutex.lock();
		if( finalValue >= ptMax ) {
			valueStdMutex.unlock();
			threadReturn[thrdPt] = added;
			return;
		}
		finalValue++;
		added++;
		valueStdMutex.unlock();
	}
	threadReturn[thrdPt] = -1;
	return;
}

void resetBase() {
	finalValue = 0;
	threadVector.resize(trdMax);
	threadReturn.resize(trdMax);
	for(int a = 0; a < trdMax; ++a ) {
		if(threadVector[a]) {
			delete( threadVector[a] );
		}
		threadVector[a] = NULL;
		threadReturn[a] = 0;
	}
}

/*
int countThread_guarded(int thrdPt) {
	int added = 0;
	while( true ) {
		picoMutex_lockGuard lGuard( &valuePicoMutex );
		if( finalValue >= ptMax ) {
			return added;
		}
		finalValue++;
		added++;
	}
}
*/
int main(int argc, char* argv[]) {
	cout << "picoMutex demo example \n";

	int a, totalRuns;
	if(argc >= 2) {
		a = (int)charTools::toLong(argv[1]);
		if(a >= 1) {
			trdMax = a;
		}
	}
	cout << "Assuming maximum thread of : " << trdMax << "\n";

	if(argc >= 3) {
		a = (int)charTools::toLong(argv[2]);
		if(a >= 1) {
			ptMax = a;
		}
	}
	cout << "Assuming maximum count of : " << ptMax << "\n";
	valueSemaphore = x_semaphore(1);

	clock_t startTimer;
	clock_t endTimer;
	double timerDuration;
	
	//Start of benchmark set
	resetBase();
	cout << "----------------------------------------\n";
	cout << "Starting base example (no mutex)\n";
	startTimer = clock();
	//actual benchmarked code
	for( a = 0; a < trdMax; ++a ) {
		threadVector[a] = new thread(countThread,a);
	}
	for( a = 0; a < trdMax; ++a ) {
		threadVector[a] -> join();
	}
	//Result output code
	endTimer = clock();
	timerDuration = (endTimer - startTimer) / ((double) CLOCKS_PER_SEC);
	cout << "Example duration : " << timerDuration << " s \n";
	cout << "Final value : " << finalValue << "\n";
	totalRuns = 0;
	for( a = 0; a < trdMax; ++a ) {
		cout << "Thread " << a << " added value : " << threadReturn[a] << "\n";
		totalRuns += threadReturn[a];
	}
	cout << "Total added by thread: " << totalRuns << "\n";
	//End of benchmark set

	
	//Start of benchmark set
	resetBase();
	cout << "----------------------------------------\n";
	cout << "Starting base example (picoMutex)\n";
	startTimer = clock();
	//actual benchmarked code
	for( a = 0; a < trdMax; ++a ) {
		threadVector[a] = new thread(countThread_picoMutexed,a);
	}
	for( a = 0; a < trdMax; ++a ) {
		threadVector[a] -> join();
	}
	//Result output code
	endTimer = clock();
	timerDuration = (endTimer - startTimer) / ((double) CLOCKS_PER_SEC);
	cout << "Example duration : " << timerDuration << " s \n";
	cout << "Final value : " << finalValue << "\n";
	totalRuns = 0;
	for( a = 0; a < trdMax; ++a ) {
		cout << "Thread " << a << " added value : " << threadReturn[a] << "\n";
		totalRuns += threadReturn[a];
	}
	cout << "Total added by thread: " << totalRuns << "\n";
	//End of benchmark set

	//Start of benchmark set
	resetBase();
	cout << "----------------------------------------\n";
	cout << "Starting base example (stdMutex)\n";
	startTimer = clock();
	//actual benchmarked code
	for( a = 0; a < trdMax; ++a ) {
		threadVector[a] = new thread(countThread_stdMutexed,a);
	}
	for( a = 0; a < trdMax; ++a ) {
		threadVector[a] -> join();
	}
	//Result output code
	endTimer = clock();
	timerDuration = (endTimer - startTimer) / ((double) CLOCKS_PER_SEC);
	cout << "Example duration : " << timerDuration << " s \n";
	cout << "Final value : " << finalValue << "\n";
	totalRuns = 0;
	for( a = 0; a < trdMax; ++a ) {
		cout << "Thread " << a << " added value : " << threadReturn[a] << "\n";
		totalRuns += threadReturn[a];
	}
	cout << "Total added by thread: " << totalRuns << "\n";
	//End of benchmark set
	

	//Start of benchmark set
	resetBase();
	cout << "----------------------------------------\n";
	cout << "Starting base example (x_semaphore)\n";
	startTimer = clock();
	//actual benchmarked code
	for( a = 0; a < trdMax; ++a ) {
		threadVector[a] = new thread(countThread_semaphore,a);
	}
	for( a = 0; a < trdMax; ++a ) {
		threadVector[a] -> join();
	}
	//Result output code
	endTimer = clock();
	timerDuration = (endTimer - startTimer) / ((double) CLOCKS_PER_SEC);
	cout << "Example duration : " << timerDuration << " s \n";
	cout << "Final value : " << finalValue << "\n";
	totalRuns = 0;
	for( a = 0; a < trdMax; ++a ) {
		cout << "Thread " << a << " added value : " << threadReturn[a] << "\n";
		totalRuns += threadReturn[a];
	}
	cout << "Total added by thread: " << totalRuns << "\n";
	//End of benchmark set


	return 0;
}