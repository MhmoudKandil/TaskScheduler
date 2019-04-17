#include "ITask.h"


void ITask::setLastError(int ti_error)
{
	m_lasterror = ti_error;
}

int ITask::lastError() const
{
	return m_lasterror;
}

unsigned int ITask::id() const
{
	return m_id;
}

ITask::ITask()
{
    m_failcount = 0;
	m_retystratgy = RetryStartgy::DropAfterNFailureAndSuspend;
	m_suspend_timestamp = std::chrono::milliseconds(0);
	m_task_timestamp = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
	m_lasterror = 0;

}
ITask::ITask(RetryStartgy ti_retystrtgy)
	:ITask()
{
	m_retystratgy = ti_retystrtgy;
}

void ITask::setTaskState(TaskState ti_taskstate)
{
    m_taskstate = ti_taskstate;
    if(ti_taskstate == TaskState::Failed)
    {
        m_failcount++;
    }
}

void ITask::setTaskRetyStratgy(RetryStartgy ti_retystrtgy)
{
	m_retystratgy = ti_retystrtgy;
}

void ITask::setSuspenionReleaseTime(milliseconds ti_time)
{
	this->m_suspend_timestamp = ti_time;
}

void ITask::setId(unsigned int ti_id)
{
	this->m_id = ti_id;
}

milliseconds ITask::suspenionReleaseTime() const
{
	return m_suspend_timestamp;
}

milliseconds ITask::taskTimeStamp() const
{
	return m_task_timestamp;
}

bool ITask::isReady() const
{
	milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
	if (ms >= suspenionReleaseTime() || suspenionReleaseTime() == std::chrono::milliseconds(0))
		return true;
	return false;
}

TaskState ITask::taskState() const
{
    return m_taskstate;
}

RetryStartgy ITask::retryStratgy() const
{
	return m_retystratgy;
}

void ITask::resetAttemptCount() 
{
    m_failcount = 0;
}

int ITask::failCount() const
{
    return m_failcount;
}


ITask::~ITask()
{

}
