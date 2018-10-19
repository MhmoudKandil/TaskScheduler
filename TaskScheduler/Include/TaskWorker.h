#pragma once
#include <deque>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ITask.h"

class TaskSchedular;
class TaskWorker
{
public:
	friend class TaskSchedular;
	int startWorkerThread();
	void stopWorkerThread();
	int addTaskFront(std::unique_ptr<ITask> ti_task);
	int addTaskToBack(std::unique_ptr<ITask> ti_task);
	int addSuspendedTask(std::unique_ptr<ITask> ti_task);
	~TaskWorker();
	int workLoad() const;

private:
	bool m_isbusy = false;
	std::deque<std::unique_ptr<ITask> > m_queuetasks;
	std::deque<std::unique_ptr<ITask> > m_suspendedqtasks;
	TaskSchedular&   m_reftaskscheduler;

	std::thread					   m_workerthread;
	std::mutex					   m_mutexlock;
	std::mutex					   m_suspendedqmutexlock;
	std::condition_variable		   m_conditionvar;
	void workerMainloop();
	TaskWorker(TaskSchedular&  ti_parenttaskscheduler);
};
