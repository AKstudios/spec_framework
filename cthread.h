//==========================================================================================================
// cthread.h - Defines a wrapper around the POSIX pthread class
//==========================================================================================================
#pragma once
#include "pthread.h"

//==========================================================================================================
// CMutex() - Implement a critical-section mutex.  These should NEVER be declared on the stack (because they
//            will be ineffective
//==========================================================================================================
class CMutex
{
public:

	// Constructor() - Initializes the mutex
	CMutex();

	// Waits until a lock is obtained on the mutex
	void	lock() 		{pthread_mutex_lock(&m_mutex);}

	// Unlocks the mutex
	void	unlock()	{pthread_mutex_unlock(&m_mutex);}

	// Tries to lock the mutex and returns 'false' if it cant
	bool	try_lock()	{return pthread_mutex_trylock(&m_mutex);}

protected:

	pthread_mutex_t m_mutex;
};
//==========================================================================================================


//==========================================================================================================
// CUniqueLock - Use these to perform convenient locking of a CMutex
//
// Note: These are intended to be created on the stack
//==========================================================================================================
class UniqueLock
{
public:

	// Constructor
	UniqueLock(CMutex& mtx, bool initially_lock = true)
	{
		m_is_locked = false;
		m_mutex = &mtx;
		if (initially_lock) lock();
	}

	// Destructor, unlocks the mutex
	~UniqueLock() {unlock();}

	// Call this to lock the mutex
	void lock()
	{
		if (!m_is_locked) m_mutex->lock();
		m_is_locked = true;
	}

	// Call this to unlock the mutex
	void unlock()
	{
		if (m_is_locked) m_mutex->unlock();
		m_is_locked = false;
	}

	// Call this to attempt a lock on the mutex
	bool try_lock()
	{
		if (m_is_locked) return true;
		m_is_locked = m_mutex->try_lock();
		return m_is_locked;
    }

protected:
	CMutex*	m_mutex;
	bool	m_is_locked;
};
//==========================================================================================================


//==========================================================================================================
// CThread - Encapsulated a POSIX thread
//==========================================================================================================
class CThread
{

//----------------------------------------------------------------------------
// These routines implement the spawned thread
//----------------------------------------------------------------------------
protected:

	// This is the entry point to the thread when it starts
	virtual void main() = 0;

	// Call this to terminate the thread
	virtual void terminate();

	// This gets called just prior to the thread terminating
	virtual void on_terminate() {}

	// This gets call just prior to the thread spawning
	virtual void on_spawn() {}

//----------------------------------------------------------------------------
// These API's are for other threads to interface with this thread
//----------------------------------------------------------------------------
public:

	// Default Constructor
	CThread();

	// All base-classes should have virtual destructors
	virtual  ~CThread() {};

	// Call this to spawn the thread
	int		spawn(const void* p1=0, const void* p2=0, const void* p3=0);

	// Call this to manually change the thread ID
	void	set_thread_id(int id) {m_thread_id = id;}

	// Retrieved the thread ID
	int		get_thread_id() {return m_thread_id;}

	// Call this to join (i.e., merge wait on) this thread
	void	join();


//----------------------------------------------------------------------------
// Thread-local storage
//----------------------------------------------------------------------------
protected:

	// The POSIX thread object
	pthread_t	m_thread;

	// The ID of this thread
	int			m_thread_id;

	// These are the parameters we spawned with
	const void* m_spawn_param[3];

	// Global count of the # of CThread objects that have been created
	static int  m_thread_count;

	// The helper function that gets spun-up in a new thread
	static void* launch_cthread(void*);

};
//==========================================================================================================