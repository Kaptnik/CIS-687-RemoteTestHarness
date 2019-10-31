#pragma once
#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H
///////////////////////////////////////////////////////////////
// BlockingQueue.h - Thread-safe Blocking Queue        //
// ver 1.3                                                   //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2015 //
///////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package contains one thread-safe class: BlockingQueue<T>.
* Its purpose is to support sending messages between threads.
* It is implemented using C++11 threading constructs including
* std::condition_variable and std::mutex.  The underlying storage
* is provided by the non-thread-safe std::queue<T>.
*
* Required Files:
* ---------------
* BlockingQueue.h
*
*
* Maintenance History:
* --------------------
* ver 1.3 : 04 Mar 2016
* - changed behavior of front() to throw exception
*   on empty queue.
* - added comment about std::unique_lock in DeQueue()
* ver 1.2 : 27 Feb 2016
* - added front();
* - added move ctor and move assignment
* - deleted copy ctor and copy assignment
* ver 1.1 : 26 Jan 2015
* - added copy constructor and assignment operator
* ver 1.0 : 03 Mar 2014
* - first release
*
*/

#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <string>
#include <iostream>
#include <sstream>

template <typename T>
class BlockingQueue {
public:
	BlockingQueue() {}
	BlockingQueue(BlockingQueue<T>&&);
	BlockingQueue<T>& operator=(BlockingQueue<T>&&);
	BlockingQueue(const BlockingQueue<T>&) = delete;
	BlockingQueue<T>& operator=(const BlockingQueue<T>&) = delete;
	T DeQueue();
	void EnQueue(const T& t);
	T& Front();
	void Clear();
	size_t Size();
private:
	std::queue<T> _queue;
	std::mutex _mutex;
	std::condition_variable _conditionVariable;
};
//----< move constructor >---------------------------------------------
template<typename T>
BlockingQueue<T>::BlockingQueue(BlockingQueue<T>&& blockingQueue) // need to lock so can't initialize
{
	std::lock_guard<std::mutex> lockock(_mutex);
	_queue = blockingQueue._queue;
	while (blockingQueue._queue.Size() > 0)  // clear bq
		blockingQueue._queue.pop();
	/* can't copy  or move mutex or condition variable, so use default members */
}
//----< move assignment >----------------------------------------------

template<typename T>
BlockingQueue<T>& BlockingQueue<T>::operator=(BlockingQueue<T>&& blockingQueue)
{
	if (this == &blockingQueue) return *this;
	std::lock_guard<std::mutex> lockock(_mutex);
	_queue = blockingQueue._queue;
	while (blockingQueue._queue.Size() > 0)  // clear bq
		blockingQueue._queue.pop();
	/* can't move assign mutex or condition variable so use target's */
	return *this;
}
//----< remove element from front of queue >---------------------------

template<typename T>
T BlockingQueue<T>::DeQueue()
{
	std::unique_lock<std::mutex> lock(_mutex);
	/*
	This lock type is required for use with condition variables.
	The operating system needs to lock and unlock the mutex:
	- when wait is called, below, the OS suspends waiting thread
	and releases lock.
	- when notify is called in EnQueue() the OS relocks the mutex,
	resumes the waiting thread and sets the condition variable to
	signaled state.
	std::lock_quard does not have public lock and unlock functions.
	*/
	if (_queue.size() > 0)
	{
		T temp = _queue.front();
		_queue.pop();
		return temp;
	}
	// may have spurious returns so loop on !condition

	while (_queue.size() == 0)
		_conditionVariable.wait(lock, [this]() { return _queue.size() > 0; });
	T temp = _queue.front();
	_queue.pop();
	return temp;
}
//----< push element onto back of queue >------------------------------

template<typename T>
void BlockingQueue<T>::EnQueue(const T& t)
{
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_queue.push(t);
	}
	_conditionVariable.notify_one();
}
//----< peek at next item to be popped >-------------------------------

template <typename T>
T& BlockingQueue<T>::Front()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_queue.Size() > 0)
		return _queue.front();
	throw std::exception("attempt to deQue empty queue");
}
//----< remove all elements from queue >-------------------------------

template <typename T>
void BlockingQueue<T>::Clear()
{
	std::lock_guard<std::mutex> lock(_mutex);
	while (_queue.Size() > 0)
		_queue.pop();
}
//----< return number of elements in queue >---------------------------

template<typename T>
size_t BlockingQueue<T>::Size()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _queue.Size();
}

#endif
