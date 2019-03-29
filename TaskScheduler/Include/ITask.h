#pragma once

#define MAX_FAILED_TRIAL  3
#define SUSPENSION_IN_MS  5000
#include <chrono>
using namespace std::chrono;

enum class TaskState
{
	None=0,
	Running = 1,
	Failed = 2,
	Waiting = 3, // On The Execution Queue.
	Suspended = 4, // On The Suspended Queue.
	Succeeded = 5,

};

enum class RetryStartgy
{
	DropAfterTheFirstFailure = 0,
	DropAfterNFailureAndSuspend = 1,
	NeverDropTask = 2
};

class ITask
{
public:
	ITask();
	ITask(RetryStartgy ti_retystrtgy);
	virtual ~ITask();
	
	virtual void onTaskFinished() {}
	virtual void onTaskStarted() {}
	virtual void runTask() = 0;

	void setLastError(int ti_error);
	void setTaskState(TaskState ti_taskstate);
	void setTaskRetyStratgy(RetryStartgy ti_retystrtgy);
	void setSuspenionReleaseTime(milliseconds time);
	void setId(unsigned int ti_id);
	void resetAttemptCount();

	milliseconds suspenionReleaseTime() const;
	milliseconds taskTimeStamp() const;
	bool isReady() const;
	TaskState taskState() const;
	RetryStartgy retryStratgy() const;
	int failCount() const;
	int lastError() const;
	unsigned int id() const;

private:
	unsigned int m_id;
	int m_lasterror;
	int m_failcount;
	TaskState m_taskstate = TaskState::None;
	RetryStartgy m_retystratgy;
	milliseconds m_suspend_timestamp;
	milliseconds m_task_timestamp;


};
