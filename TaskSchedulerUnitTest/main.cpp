#include "gtest/gtest.h"
#include <memory>
#include "ITask.h"
#include "TaskScheduler.h"
#include <atomic>
#include <thread>
#include <chrono>
using namespace std;

std::atomic<int> g_atomic_count; 


struct TaskMockTester : public ITask
{
	TaskState m_taskstate;
	
	TaskMockTester(TaskState t_taskstate = TaskState::Succeeded)
		:m_taskstate(t_taskstate)
	{
	
	}

	// Inherited via ITask
	virtual void runTask() override
	{
		g_atomic_count	++;
		this->setLastError(-1);
		this->setTaskState(m_taskstate);
	}
};

TEST(ITask, Validate_Creating_Tasks)
{
	TaskMockTester t0(TaskState::Succeeded);
	TaskMockTester t1(TaskState::Failed);

	EXPECT_EQ(t0.taskState(), TaskState::None);
	EXPECT_EQ(t1.taskState(), TaskState::None);
	t0.runTask();
	t1.runTask();
	EXPECT_EQ(t0.taskState(), TaskState::Succeeded);
	EXPECT_EQ(t1.taskState(), TaskState::Failed);
	EXPECT_EQ(t0.lastError(), -1);

}


TEST(TaskSchedular, Validate_TaskSchedular_AddingTasks)
{
	std::unique_ptr<TaskMockTester> tasktester0 = std::make_unique<TaskMockTester>();
	std::unique_ptr<TaskMockTester> tasktester1 = std::make_unique<TaskMockTester>();
	std::unique_ptr<TaskMockTester> tasktester2 = std::make_unique<TaskMockTester>();
	std::unique_ptr<TaskMockTester> tasktester3 = std::make_unique<TaskMockTester>();


	TaskSchedular ts(1);
	int error = ts.addTaskToback(std::move(tasktester0));
	EXPECT_EQ(error, -1);
	error = ts.addTaskToFront(std::move(tasktester1));
	EXPECT_EQ(error, -1);
	ts.startWorkers();
	error = ts.addTaskToFront(std::move(tasktester2));
	EXPECT_EQ(error, 0);
	error = ts.addTaskToFront(std::move(tasktester3));
	ts.startWorkers();
	ts.stopWorkersAsync();
	ts.waitForTermination();
	EXPECT_EQ(error, 0);
}

TEST(TaskSchedular, Validate_TaskSchedular_Status)
{
	TaskSchedular ts(1);
	EXPECT_EQ(ts.schedulerState(), SchedulerState::PARKED);
	ts.startWorkers();
	EXPECT_EQ(ts.schedulerState(), SchedulerState::RUNNING);
	ts.stopWorkersAsync();
	ts.waitForTermination();
	EXPECT_EQ(ts.schedulerState(), SchedulerState::PARKED);
}

TEST(TaskSchedular, Validate_TaskSchedular_rety_stratgy_DropAfterTheFirstFailure)
{
	g_atomic_count = 0;
	const int sleep_n_seconds = 1;
	TaskSchedular ts(1);
	std::unique_ptr<TaskMockTester> tasktester0 = std::make_unique<TaskMockTester>(TaskState::Failed);
	tasktester0->setTaskRetyStratgy(RetryStartgy::DropAfterTheFirstFailure);
	ts.startWorkers();
	ts.addTaskToback(std::move(tasktester0));
	std::this_thread::sleep_for(std::chrono::seconds(sleep_n_seconds));
	ts.stopWorkersAsync();
	EXPECT_EQ(g_atomic_count, 1);
	ts.waitForTermination();
}

TEST(TaskSchedular, Validate_TaskSchedular_rety_stratgy_NeverDropTask)
{
	g_atomic_count = 0;
	const int sleep_n_seconds = 5;
	TaskSchedular ts(1);
	std::unique_ptr<TaskMockTester> tasktester0 = std::make_unique<TaskMockTester>(TaskState::Failed);
	tasktester0->setTaskRetyStratgy(RetryStartgy::NeverDropTask);
	ts.startWorkers();
	ts.addTaskToback(std::move(tasktester0));
	std::this_thread::sleep_for(std::chrono::seconds(sleep_n_seconds));
	ts.stopWorkersAsync();
	ts.waitForTermination();
	EXPECT_EQ(g_atomic_count, sleep_n_seconds);

}

TEST(TaskSchedular, Validate_TaskSchedular_rety_stratgy_DropAfterNFailureAndSuspendOneHour)
{
	g_atomic_count = 0;
	const int sleep_n_seconds = 15;
	TaskSchedular ts(1);
	std::unique_ptr<TaskMockTester> tasktester0 = std::make_unique<TaskMockTester>(TaskState::Failed);
	tasktester0->setTaskRetyStratgy(RetryStartgy::DropAfterNFailureAndSuspend);
	ts.startWorkers();
	ts.addTaskToback(std::move(tasktester0));
	std::this_thread::sleep_for(std::chrono::seconds(sleep_n_seconds));
	ts.stopWorkersAsync();
	ts.waitForTermination();
	EXPECT_EQ(g_atomic_count, sleep_n_seconds/(SUSPENSION_IN_MS/1000) * MAX_FAILED_TRIAL);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}