//
// TCPServerDispatcher.cpp
//
// $Id: //poco/Main/Net/src/TCPServerDispatcher.cpp#10 $
//
// Library: Net
// Package: TCPServer
// Module:  TCPServerDispatcher
//
// Copyright (c) 2005-2007, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Net/TCPServerDispatcher.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Notification.h"
#include "Poco/AutoPtr.h"
#include <memory>


using Poco::Notification;
using Poco::FastMutex;
using Poco::AutoPtr;


namespace Poco {
namespace Net {


class TCPConnectionNotification: public Notification
{
public:
	TCPConnectionNotification(const StreamSocket& socket):
		_socket(socket)
	{
	}
	
	~TCPConnectionNotification()
	{
	}
	
	const StreamSocket& socket() const
	{
		return _socket;
	}

private:
	StreamSocket _socket;
};


TCPServerDispatcher::TCPServerDispatcher(TCPServerConnectionFactory::Ptr pFactory, Poco::ThreadPool& threadPool, TCPServerParams::Ptr pParams):
	_rc(1),
	_pParams(pParams),
	_currentThreads(0),
	_totalConnections(0),
	_currentConnections(0),
	_maxConcurrentConnections(0),
	_refusedConnections(0),
	_stopped(false),
	_pConnectionFactory(pFactory),
	_threadPool(threadPool)
{
	poco_check_ptr (pFactory);

	if (!_pParams)
		_pParams = new TCPServerParams;
	
	if (_pParams->getMaxThreads() == 0)
		_pParams->setMaxThreads(threadPool.capacity());
}


TCPServerDispatcher::~TCPServerDispatcher()
{
}


void TCPServerDispatcher::duplicate()
{
	_mutex.lock();
	++_rc;
	_mutex.unlock();
}


void TCPServerDispatcher::release()
{
	_mutex.lock();
	int rc = --_rc;
	_mutex.unlock();
	if (rc == 0) delete this;
}


void TCPServerDispatcher::run()
{
	AutoPtr<TCPServerDispatcher> guard(this, true); // ensure object stays alive

	int idleTime = (int) _pParams->getThreadIdleTime().totalMilliseconds();

	for (;;)
	{
		AutoPtr<Notification> pNf = _queue.waitDequeueNotification(idleTime);
		if (pNf)
		{
			TCPConnectionNotification* pCNf = dynamic_cast<TCPConnectionNotification*>(pNf.get());
			if (pCNf)
			{
				std::auto_ptr<TCPServerConnection> pConnection(_pConnectionFactory->createConnection(pCNf->socket()));
				poco_check_ptr(pConnection.get());
				beginConnection();
				pConnection->start();
				endConnection();
			}
		}
	
		FastMutex::ScopedLock lock(_mutex);
		if (_stopped || _currentThreads > 1 && _queue.empty())
		{
			--_currentThreads;
			break;
		}
	}
}

	
void TCPServerDispatcher::enqueue(const StreamSocket& socket)
{
	FastMutex::ScopedLock lock(_mutex);

	if (_queue.size() < _pParams->getMaxQueued())
	{
		_queue.enqueueNotification(new TCPConnectionNotification(socket));
		if (!_queue.hasIdleThreads() && _currentThreads < _pParams->getMaxThreads())
		{
			try
			{
				static const std::string threadName("TCPServerConnection");
				_threadPool.startWithPriority(_pParams->getThreadPriority(), *this, threadName);
				++_currentThreads;
			}
			catch (Poco::Exception&)
			{
				// no problem here, connection is already queued
				// and a new thread might be available later.
			}
		}
	}
	else
	{
		++_refusedConnections;
	}
}


void TCPServerDispatcher::stop()
{
	_stopped = true;
	_queue.clear();
	_queue.wakeUpAll();
}


int TCPServerDispatcher::currentThreads() const
{
	FastMutex::ScopedLock lock(_mutex);
	
	return _currentThreads;
}


int TCPServerDispatcher::totalConnections() const
{
	FastMutex::ScopedLock lock(_mutex);
	
	return _totalConnections;
}


int TCPServerDispatcher::currentConnections() const
{
	FastMutex::ScopedLock lock(_mutex);
	
	return _currentConnections;
}


int TCPServerDispatcher::maxConcurrentConnections() const
{
	FastMutex::ScopedLock lock(_mutex);
	
	return _maxConcurrentConnections;
}


int TCPServerDispatcher::queuedConnections() const
{
	return _queue.size();
}


int TCPServerDispatcher::refusedConnections() const
{
	FastMutex::ScopedLock lock(_mutex);
	
	return _refusedConnections;
}


void TCPServerDispatcher::beginConnection()
{
	FastMutex::ScopedLock lock(_mutex);
	
	++_totalConnections;
	++_currentConnections;
	if (_currentConnections > _maxConcurrentConnections)
		_maxConcurrentConnections = _currentConnections;
}


void TCPServerDispatcher::endConnection()
{
	FastMutex::ScopedLock lock(_mutex);

	--_currentConnections;
}


} } // namespace Poco::Net
