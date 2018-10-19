#pragma once

#include <deque>
#include <vector>
#include "ITask.h"
#include "SchedulerStates.h"
#include "TaskWorker.h"
#include <memory>


class TaskSchedular
{
public:
	TaskSchedular(int t_workersize);
	int addTaskToback(std::unique_ptr<ITask> t_task);
	int addTaskToFront(std::unique_ptr<ITask> t_task);
	void startWorkers();
	void stopWorkersAsync();
	void waitForTermination();
	// Const Member Function 
	int workLoad()	const;
	SchedulerState schedulerState() const;

private:
	const int m_iworksize;
	std::vector<std::unique_ptr<TaskWorker> > m_vectaskworkers;
	SchedulerState			m_schedulerstate;
};