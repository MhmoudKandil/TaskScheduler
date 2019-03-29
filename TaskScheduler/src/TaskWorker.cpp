#include "TaskWorker.h"
#include "TaskScheduler.h"
#include <iostream>
using namespace std;

using namespace std::chrono;

TaskWorker::TaskWorker(TaskSchedular& t_parent_taskscheduler)
	:m_reftaskscheduler(t_parent_taskscheduler)
{
}

int TaskWorker::startWorkerThread()
{
	std::thread workerthead(&TaskWorker::workerMainloop, this);
	m_workerthread = std::move(workerthead);
	return 0;
}

void TaskWorker::stopWorkerThread()
{
	if (m_workerthread.joinable())
		m_workerthread.join();
}

int TaskWorker::addTaskFront(std::unique_ptr<ITask> t_task)
{
	t_task->setTaskState(TaskState::Waiting);
	m_reftaskscheduler.updateTaskState(t_task->id(), TaskState::Waiting);
	std::lock_guard<std::mutex> mu(m_mutexlock);
	m_queuetasks.push_front(std::move(t_task));
	m_conditionvar.notify_all();
	return 0;
}

int TaskWorker::addTaskToBack(std::unique_ptr<ITask> t_task)
{
	t_task->setTaskState(TaskState::Waiting);
	m_reftaskscheduler.updateTaskState(t_task->id(), TaskState::Waiting);
	std::lock_guard<std::mutex> mu(m_mutexlock);
	m_queuetasks.push_back(std::move(t_task));
	m_conditionvar.notify_all();
	return 0;
}

int TaskWorker::addSuspendedTask(std::unique_ptr<ITask> t_task)
{
	std::lock_guard<std::mutex> mu(m_mutexlock);
	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	t_task->setSuspenionReleaseTime(ms + std::chrono::milliseconds(SUSPENSION_IN_MS));
	t_task->setTaskState(TaskState::Suspended);
	m_reftaskscheduler.updateTaskState(t_task->id(), TaskState::Suspended);
	m_suspendedqtasks.push_back(std::move(t_task));
	return 0;
}

TaskWorker::~TaskWorker()
{
}

void TaskWorker::workerMainloop()
{
	while (m_reftaskscheduler.schedulerState() == SchedulerState::STARTING)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	while (m_reftaskscheduler.schedulerState() == SchedulerState::RUNNING)
	{
		std::unique_ptr<ITask> toptask = NULL;
		{
			std::unique_lock<std::mutex> mu(m_mutexlock);
			if (m_conditionvar.wait_for(mu, std::chrono::seconds(1), [&] {return !m_queuetasks.empty(); }))
			{
				m_isbusy = true;
				RetryStartgy currtaskstratgy = RetryStartgy::DropAfterTheFirstFailure;
				TaskState currtaskstate = TaskState::Failed;
				bool s1 = false;
				bool s2 = false;
				int failedtrials = 0;
				toptask = std::move(m_queuetasks.front());
				currtaskstratgy = toptask->retryStratgy();
				m_queuetasks.pop_front();
				mu.unlock();
				toptask->onTaskStarted();
				do
				{
					toptask->setTaskState(TaskState::Running);
					m_reftaskscheduler.updateTaskState(toptask->id(), TaskState::Running);
					toptask->runTask();
					currtaskstate = toptask->taskState();
					failedtrials = toptask->failCount();
					// notify scheduler with task state
					m_reftaskscheduler.updateTaskState(toptask->id(), currtaskstate);

					s1 = (currtaskstratgy == RetryStartgy::NeverDropTask && currtaskstate == TaskState::Failed);
					s2 = (currtaskstratgy == RetryStartgy::DropAfterNFailureAndSuspend && currtaskstate == TaskState::Failed && failedtrials%MAX_FAILED_TRIAL != 0);

					if (s1)
					{
						// Sleep for one second before retrying
						// Give the CPU the oppertunity to rest.
						std::this_thread::sleep_for(std::chrono::seconds(1));
					}

				} while (m_reftaskscheduler.schedulerState() == SchedulerState::RUNNING && (s1 || s2));
				toptask->onTaskFinished();

				//Check if the task will be suspended.
				if (currtaskstratgy == RetryStartgy::DropAfterNFailureAndSuspend && currtaskstate == TaskState::Failed)
				{
					this->addSuspendedTask(std::move(toptask));
				}
				m_isbusy = false;
			}
		}
		//Check for suspended tasks to enqueue tham again if an hour is elapsed.
		std::unique_lock<std::mutex> mu2(m_suspendedqmutexlock);
		while (!m_suspendedqtasks.empty())
		{
			if (m_suspendedqtasks.front()->isReady())
			{
				std::unique_ptr<ITask> r = nullptr;
				r = std::move(m_suspendedqtasks.front());
				addTaskToBack(std::move(r));
				m_suspendedqtasks.pop_front();
			}
			else
				break;
		}

	}
}

int TaskWorker::workLoad() const
{
	return (int)m_queuetasks.size() + m_isbusy;
}