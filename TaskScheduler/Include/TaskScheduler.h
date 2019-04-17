#pragma once

#include <deque>
#include <vector>
#include "ITask.h"
#include "SchedulerStates.h"
#include "TaskWorker.h"
#include <memory>
#include <atomic>
#include <unordered_map>

#define MAX_TASK_HISTORY 10000

class TaskSchedular
{
public:
	TaskSchedular(int t_workersize);
	int addTaskToback(std::unique_ptr<ITask> t_task);
	int addTaskToFront(std::unique_ptr<ITask> t_task);
	void startWorkers();
	void stopWorkersAsync();
	void waitForTermination();
	void updateTaskState(unsigned int t_id, TaskState t_taskstate);
	// Const Member Function 
	int workLoad()	const;
	SchedulerState schedulerState() const;
	TaskState getTaskState(unsigned int t_taskid) const;
	double getSuccessPercentage() const;
	double getFailurePercentage() const;

private:
	const int m_iworksize;
	std::vector<std::unique_ptr<TaskWorker> > m_vectaskworkers;
	SchedulerState			m_schedulerstate;
	std::atomic_uint							m_taskid;
	std::unordered_map<unsigned int, TaskState> m_taskstates;
	std::deque<unsigned int>					m_taskids;
	std::mutex									m_mutexlock;
	std::atomic_uint							m_totalexecuted;
	std::atomic_uint							m_totalsucceeded;
};