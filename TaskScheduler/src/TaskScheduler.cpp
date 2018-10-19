#include "TaskScheduler.h"
#include "TaskWorker.h"
#include <vector>
#include <algorithm>

TaskSchedular::TaskSchedular(int ti_workersize)
	:m_iworksize(ti_workersize),
	m_schedulerstate(SchedulerState::PARKED)
{
	for (int i = 0; i < ti_workersize; i++)
	{
		std::unique_ptr<TaskWorker> temp{ new TaskWorker(*this) };
		m_vectaskworkers.push_back(std::move(temp));
	}
}

int TaskSchedular::addTaskToback(std::unique_ptr<ITask> ti_task)
{
	if (this->m_schedulerstate == SchedulerState::RUNNING)
	{
		auto worker = std::min_element(m_vectaskworkers.begin(), m_vectaskworkers.end(), [](auto& lhs, auto& rhs) 
		{
			return lhs->workLoad() < rhs->workLoad();
		}
		);
		(*worker)->addTaskToBack(std::move(ti_task));
	}
	else
	{
		return -1;
	}
	return 0;
}

int TaskSchedular::addTaskToFront(std::unique_ptr<ITask> ti_task)
{
	if (this->m_schedulerstate == SchedulerState::RUNNING)
	{
		auto worker = std::min_element(m_vectaskworkers.begin(), m_vectaskworkers.end(), [](auto& lhs, auto& rhs)
		{
			return lhs->workLoad() < rhs->workLoad();
		}
		);
		(*worker)->addTaskFront(std::move(ti_task));
	}
	else
	{
		return -1;
	}
	return 0;
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
