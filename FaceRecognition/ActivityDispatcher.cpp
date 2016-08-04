#include "stdafx.h"
#include "ActivityDispatcher.h"
#include <Poco/Thread.h>
#include "CaptureNotification.h"
#include "FaceMatch.h"
#include <Poco/ActiveResult.h>

using Poco::ActiveResult;
using Poco::Thread;
using Poco::FastMutex;
using Poco::Notification;

FastMutex ActivityDispatcher::_mutex;

ActivityDispatcher::ActivityDispatcher() :
_activity(this, &ActivityDispatcher::runActivity)
{
}

void ActivityDispatcher::start()
{
	_activity.start();
}

void ActivityDispatcher::stop()
{
	_activity.stop();
	_activity.wait();
}

void ActivityDispatcher::runActivity()
{
	while (!_activity.isStopped())
	{
		OutputDebugStringA("Activity running.");
		Thread::sleep(250);
		//
		Notification::Ptr pNf(_queue.waitDequeueNotification());
		if (pNf)
		{
			CaptureNotification::Ptr pWorkNf = pNf.cast<CaptureNotification>();
			if (pWorkNf)
			{
				{
					FastMutex::ScopedLock lock(_mutex);
					Picture::Ptr pic = pWorkNf->data();

					FaceMatch example;
					FaceMatch::AddArgs args = {pic,pic};
					ActiveResult<bool> result = example.activeMatch(args);
					result.wait();
					bool ret = result.data();
					OutputDebugStringA((std::string("ActiveResult : ") + std::to_string(ret) + std::string("\n")).c_str());
				}
			}
		}
	}
	OutputDebugStringA("Activity stopped.");
}

void ActivityDispatcher::enqueueNotification(Poco::Notification::Ptr pNotification)
{
	_queue.enqueueNotification(pNotification);
}