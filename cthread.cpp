//==========================================================================================================
// cthread.cpp - Implements a class that wraps POSIX threads
//==========================================================================================================
#include <unistd.h>
#include "cthread.h"

//==========================================================================================================
// This is a count of how many CThread objects have been created
//==========================================================================================================
int CThread::m_thread_count = 0;
//==========================================================================================================


//==========================================================================================================
// LaunchCThread() - This is a helper function responsible for actually bringing the thread into existence.
//
// Passed: thread_ptr = Pointer to a CThread object
//==========================================================================================================
void* CThread::launch_cthread(void* object)
{
	// Spin up "main()" in our new thread.  When this returns, the thread is done executing
	((CThread*)object)->main();

	// Give the derived class a chance to do cleanup prior to thread termination
	((CThread*)object)->on_terminate();

	// There's nothing to hand back to the caller
	return NULL;
}
//==========================================================================================================


//==========================================================================================================
// Default constructor
//==========================================================================================================
CThread::CThread()
{
	// The default thread ID is the # of CThread objects that have been constructed.
	// A different thread ID can be set manually by calling set_thread_id();
	m_thread_id = m_thread_count++;
}
//==========================================================================================================


//==========================================================================================================
// spawn() - Called in order to start this thread
//==========================================================================================================
int CThread::spawn(const void* P1, const void* P2, const void* P3)
{
	// Fill in the object's spawn parameters
	m_spawn_param[0] = P1;
	m_spawn_param[1] = P2;
	m_spawn_param[2] = P3;

	// Give the derived class an opportunity to do stuff before we spin-up
	on_spawn();

	// Spin up the thread
	int ret = pthread_create(&m_thread, NULL, launch_cthread, this);

	// Tell the caller if it worked
	return ret;
}
//==========================================================================================================


//==========================================================================================================
// join() - join (i.e., wait on) this thread
//==========================================================================================================
void CThread::join()
{
	// Join this thread
	pthread_join(m_thread, NULL);
}
//==========================================================================================================

//==========================================================================================================
// terminate() - Terminates this thread.  Should only be called internally to this thread!!
//==========================================================================================================
void CThread::terminate()
{
	// Give the derived class an opportunity to do cleanup before we terminate
	on_terminate();
	
	// Kill the thread
	pthread_exit(NULL);
}
//==========================================================================================================


//==========================================================================================================
// Constructor() - Initializes the mutex
//==========================================================================================================
CMutex::CMutex()
{
	pthread_mutexattr_t attributes;

	// Initialize the mutex attributes
	pthread_mutexattr_init(&attributes);

	// We want a thread to be able to recursively lock this mutex
	pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);

	// Create a recursive mutex
	pthread_mutex_init(&m_mutex, &attributes);
}
//==========================================================================================================



