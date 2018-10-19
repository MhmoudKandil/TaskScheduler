# TaskScheduler
Simple C++ Taskbased scheduler.

Usage

1-Create TaskScheduler object and specify the number of workers (threads) that will consume the tasks from the queue more than //one worker mean that tasks will run in parallel.

TaskSchedular ts(1);

2-Create Task from unique pointer you will have to implement the ITask interface. virtual method void runTask()  

std::unique_ptr<TaskMockTester> tasktester0 = std::make_unique<TaskMockTester>(TaskState::Failed);

3-Set The Rety Stratgy with one of the following values (DropAfterNFailureAndSuspend - NeverDrop - DropAfterFirstFailure)

tasktester0->setTaskRetyStratgy(RetryStartgy::DropAfterNFailureAndSuspend);

4-Start the workers

ts.startWorkers();

5-Add Tasks to the front or the back of the queue (Now any task is pushed to the TaskScheuler will automatically dispatched to specific worker to run on)

ts.addTaskToback(std::move(tasktester0));

6-Wait For Termination (the function will block until all Taskworkers threads can terminate. 

ts.waitForTermination();

7-Finally you will have to stop the scheduler (you will have to do that from another thread)

ts.stopWorkersAsync();