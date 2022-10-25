#include "tasksys.h"


IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->workers.resize(num_threads);
    this->num_threads = num_threads;
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {
    delete this->_mutex;
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    this->thread_count = 0;
    this->runnable = runnable;
    this->num_total_tasks = num_total_tasks;
    this->num_thread_tasks = num_total_tasks/num_threads;
    this->_mutex = new std::mutex();
    for (int t = 0; t < num_threads; ++t) {
        workers[t] = std::thread(&TaskSystemParallelSpawn::runTaskRange,this);
    }
    for (int t = 0; t < num_threads; t++) {
        workers[t].join();
    }
}

void TaskSystemParallelSpawn::runTaskRange() {
    this->_mutex->lock();
    int tc = this->thread_count;
    this->thread_count++;
    int startID = tc*num_thread_tasks;
    int endID = (tc == this->num_threads-1) ? num_total_tasks : (tc+1)*num_thread_tasks;
    this->_mutex->unlock();
    for (int i = startID; i < endID; ++i) {
        this->runnable->runTask(i, num_total_tasks);
    }
    return;
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads;
    this->workers.resize(num_threads);
    this->_mutex = new std::mutex();
    this->_rc_mutex = new std::mutex();
    this->curret_ID = 0;
    for (int i = 0; i < this->num_threads; i++) {
        this->workers[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::wait_for_task, this);
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    // Terminate all threads
    destroy = true;
    for (int t = 0; t < num_threads; t++) {
        workers[t].join();
    }
    delete this->_mutex;
    delete this->_rc_mutex;
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    this->_mutex->lock();
    this->runnable = runnable;
    this->num_total_tasks = num_total_tasks;
    this->curret_ID = 0;
    this->remaining_tasks = num_total_tasks;
    this->_mutex->unlock();
    while (this->remaining_tasks) {}
    return;
}

void TaskSystemParallelThreadPoolSpinning::wait_for_task() {
    while (!destroy) {
        this->_mutex->lock();
        int task_ID = this->curret_ID;
        if (this->num_total_tasks == 0 || task_ID >= num_total_tasks) {
            this->_mutex->unlock();
            continue;
        }
        this->curret_ID++;
        int num_tot_tasks = this->num_total_tasks;
        this->_mutex->unlock();
        this->runnable->runTask(task_ID, num_tot_tasks);
        this->_rc_mutex->lock();
        this->remaining_tasks--;
        this->_rc_mutex->unlock();
    }
    return;
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    return 0;
}



void TaskSystemParallelThreadPoolSpinning::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads;
    this->num_waiting_threads = 0;
    this->workers.resize(num_threads);
    this->_mutex = new std::mutex();
    this->_task_mutex = new std::mutex();
    this->_condition_variable = new std::condition_variable();
    this->_finish_cond = new std::condition_variable();
    this->curret_ID = 0;
    for (int i = 0; i < this->num_threads; i++) {
        this->workers[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::wait_for_task,this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->destroy = true;
    // this->_mutex->lock();
    // this->remaining_tasks = 1;
    // this->_mutex->unlock();
    this->_condition_variable->notify_all();
    for (int t = 0; t < this->num_threads; t++) {
        workers[t].join();
    }
    delete this->_mutex;
    delete this->_task_mutex;
    delete this->_condition_variable;
    delete this->_finish_cond;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    this->_mutex->lock();
    this->runnable = runnable;
    this->curret_ID = 0;
    this->num_total_tasks = num_total_tasks;
    this->_mutex->unlock();
    this->_task_mutex->lock();
    this->remaining_tasks = num_total_tasks;
    this->_task_mutex->unlock();

    std::unique_lock<std::mutex> main_lock(*this->_task_mutex);
    this->_condition_variable->notify_all();
    this->_finish_cond->wait(main_lock);
    main_lock.unlock();
    return;
}

void TaskSystemParallelThreadPoolSleeping::wait_for_task() {
    while (!destroy) {
        std::unique_lock<std::mutex> lock(*this->_mutex);
        if (!this->num_total_tasks || this->curret_ID >= this->num_total_tasks) {
            this->num_waiting_threads++;
            this->_condition_variable->wait(lock);
            this->num_waiting_threads--;
            lock.unlock();
            continue;
        }
        int task_ID = this->curret_ID;
        this->curret_ID++;
        int num_tot_tasks = this->num_total_tasks;
        lock.unlock();
        this->runnable->runTask(task_ID, num_tot_tasks);
        _task_mutex->lock();
        this->remaining_tasks--;
        if (this->remaining_tasks == 0) {
            // Wake up main thread
            _task_mutex->unlock();
            this->_finish_cond->notify_one();
            continue;
        }
        _task_mutex->unlock();
    }
    return;
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
