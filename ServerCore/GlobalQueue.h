#pragma once

/*----------------------------------
					GlobalQueue
----------------------------------*/

class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();

	GlobalQueue(const GlobalQueue& _Other) = delete;
	GlobalQueue(GlobalQueue&& _Other) noexcept = delete;
	GlobalQueue& operator=(const GlobalQueue& _Other) = delete;
	GlobalQueue& operator=(const GlobalQueue&& _Other) noexcept = delete;

	void Push(JobQueueRef jobQueue);
	JobQueueRef Pop();

private:
	LockQueue<JobQueueRef> _jobQueues;
};

