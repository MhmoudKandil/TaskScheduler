#include "TaskScheduler.h"
#include "TaskWorker.h"
#include <vector>
#include <algorithm>

TaskSchedular::TaskSchedular(int ti_workersize)
	:m_iworksize(ti_workersize),
	m_schedulerstate(SchedulerState::PARKED)
{
	m_taskid = 0;
	for (int i = 0; i < ti_workersize; i++)
	{
		std::unique_ptr<TaskWorker> temp{ new TaskWorker(*this) };
		m_vectaskworkers.push_back(std::move(temp));
	}
}

int TaskSchedular::addTaskToback(std::unique_ptr<ITask> ti_task)
{
	unsigned int tid = -1;
	if (this->m_schedulerstate == SchedulerState::RUNNING)
	{
		auto worker = std::min_element(m_vectaskworkers.begin(), m_vectaskworkers.end(), [](auto& lhs, auto& rhs) 
		{
			return lhs->workLoad() < rhs->workLoad();
		}
		);
		tid = m_taskid++;
		ti_task->setId(tid);
		(*worker)->addTaskToBack(std::move(ti_task));
	}
	return tid;
}

int TaskSchedular::addTaskToFront(std::unique_ptr<ITask> ti_task)
{
	unsigned int tid = -1;
	if (this->m_schedulerstate == SchedulerState::RUNNING)
	{
		auto worker = std::min_element(m_vectaskworkers.begin(), m_vectaskworkers.end(), [](auto& lhs, auto& rhs)
		{
			return lhs->workLoad() < rhs->workLoad();
		}
		);
		tid = m_taskid++;
		ti_task->setId(tid);
		(*worker)->addTaskFront(std::move(ti_task));
	}
	return tid;
}

void TaskSchedular::startWorkers()
{
	if (m_schedulerstate == SchedulerState::PARKED)
	{
		m_schedulerstate = SchedulerState::STARTING;

		std::for_each(m_vectaskworkers.begin(),
			m_vectaskworkers.end(),
			[](auto& worker)
		{
			worker->startWorkerThread();
		}
		);
		m_schedulerstate = SchedulerState::RUNNING;
	}
}

void TaskSchedular::stopWorkersAsync()
{
	if (m_schedulerstate == SchedulerState::RUNNING)
	{
		m_schedulerstate = SchedulerState::STOPPING;
	}
}

void TaskSchedular::waitForTermination()
{
	std::for_each(m_vectaskworkers.begin(),
		m_vectaskworkers.end(),
		[](auto& worker)
	{
		worker->stopWorkerThread();
	}
	);
	m_schedulerstate = SchedulerState::PARKED;


}

void TaskSchedular::updateTaskState(unsigned int t_id, TaskState t_taskstate)
{
	std::unique_lock<std::mutex> mulock(m_mutexlock);
	if (m_taskstates.count(t_id) == 0)
	{
		while (m_taskids.size() >= MAX_TASK_HISTORY)
		{
			m_taskstates.erase(m_taskids.front());
			m_taskids.pop_front();
		}
		m_taskids.push_back(t_id);
	}
	m_taskstates[t_id] = t_taskstate;
	switch (t_taskstate)
	{
	case TaskState::Running:
		m_totalexecuted++;
		break;
	case TaskState::Succeeded:
		m_totalsucceeded++;
		break;
	}
}

int TaskSchedular::workLoad() const
{
	int workload = 0;
	std::for_each(m_vectaskworkers.begin(),
		m_vectaskworkers.end(),
		[&workload](auto& worker)
	{
		workload += worker->workLoad();
	}
	);
	return workload;
}

SchedulerState TaskSchedular::schedulerState() const
{
	return m_schedulerstate;
}

TaskState TaskSchedular::getTaskState(unsigned int t_taskid) const
{
	TaskState taskstate = TaskState::None;
	auto iter = m_taskstates.find(t_taskid);
	if (iter != m_taskstates.end())
	{
		taskstate = iter->second;
	}
	return taskstate;
}

double TaskSchedular::getSuccessPercentage() const
{
	return ((double)m_totalsucceeded.load() / m_totalexecuted.load()) * 100.0;
}

double TaskSchedular::getFailurePercentage() const
{
	return 100.0 - getSuccessPercentage();
}