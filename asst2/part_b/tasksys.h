#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <atomic>
#include <map>
#include <set>
#include <queue>
#include <iterator>
/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */

class WaitingTask {
    public:
        IRunnable* runnable;
        int num_total_tasks;
        int task_group_id; // the task group id of the task that is waiting to finish
        WaitingTask(IRunnable* runnable, int num_total_tasks, int task_group_id) {
            this->runnable = runnable;
            this->num_total_tasks = num_total_tasks;
            this->task_group_id = task_group_id;
        }
};

class ReadyTask {
    public:
        IRunnable* runnable;
        int num_total_tasks;
        int task_group_id; // the task group id of the tasks that are waiting to finish
        int current_task_id; // the current task id that is being executed
        ReadyTask(IRunnable* runnable, int num_total_tasks, int task_group_id, int current_task_id) {
            this->runnable = runnable;
            this->num_total_tasks = num_total_tasks;
            this->task_group_id = task_group_id;
            this->current_task_id = current_task_id;
        }
};

class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
        void sleepingthread_func();
        void appendReadyTaskGroup(TaskID task_group_id);
        void searchReadyTaskGroup();
        int num_threads_;
        int current_task_group_id;
        std::thread* threads;
        bool done_pool;
        std::queue<ReadyTask*> ready_tasks;
        std::queue<TaskID> finished_task_groups;
        std::map<TaskID, WaitingTask*> task_group_id_to_task_group; //task group id -> task group
        std::map<TaskID, std::set<TaskID> > task_group_deps; //task group id -> set of task group ids which this task group depends on
        std::map<TaskID, int> left_tasks; //task group id -> number of tasks left to finish in each group
        std::mutex* finished_task_groups_mutex;
        std::mutex* run_task_mutex;
        std::condition_variable* finished_task_groups_cv;
        std::condition_variable* run_task_cv;

};

#endif
