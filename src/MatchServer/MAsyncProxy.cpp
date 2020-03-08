#include "stdafx.h"
#include "MAsyncProxy.h"
#include "MMatchConfig.h"
#include "MMatchServer.h"
#include "MCrashDump.h"
#include "MFile.h"

bool MAsyncProxy::Create(int ThreadCount)
{
	return Create(ThreadCount, MakeDatabaseFromConfig);
}

bool MAsyncProxy::Create(int ThreadCount, function_view<IDatabase*()> GetDatabase)
{
	ThreadCount = min(ThreadCount, MAX_THREADPOOL_COUNT);

	for (int i = 0; i < ThreadCount; i++)
	{
		std::thread{[this, Database = GetDatabase()] {
			OnRun(Database);
		}}.detach();
	}

	return true;
}

void MAsyncProxy::Destroy()
{
	EventShutdown.SetEvent();
}

void MAsyncProxy::PostJob(MAsyncJob* pJob)
{
	WaitQueue.Lock();
		pJob->SetPostTime(GetGlobalTimeMS());
		WaitQueue.AddUnsafe(pJob);
	WaitQueue.Unlock();

	EventFetchJob.SetEvent();
}

void MAsyncProxy::OnRun(IDatabase* Database)
{
	MSignalEvent* EventArray[]{
		&EventShutdown,
		&EventFetchJob,
	};

	bool bShutdown = false;

	while (!bShutdown)
	{
		const auto Timeout = 1000; // Milliseconds
		const auto WaitResult = WaitForMultipleEvents(EventArray, Timeout);

		if (WaitResult == MSync::WaitTimeout) {
			if (WaitQueue.GetCount() > 0) {
				EventFetchJob.SetEvent();
			}
			continue;
		}

		switch(WaitResult)
		{
		case 0: // Shutdown
			bShutdown = true;
			break;
		case 1:	// Fetch Job
			{
				WaitQueue.Lock();
					MAsyncJob* pJob = WaitQueue.GetJobUnsafe();
				WaitQueue.Unlock();

				if (pJob) {
					pJob->Run(Database);
					pJob->SetFinishTime(GetGlobalTimeMS());

					ResultQueue.Lock();
						ResultQueue.AddUnsafe(pJob);
					ResultQueue.Unlock();
				}

				if (WaitQueue.GetCount() > 0) {
					EventFetchJob.SetEvent();
				}
#ifndef WIN32
				else {
					EventFetchJob.ResetEvent();
				}
#endif
			}
			break;
		};
	};
}
