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
    this->num_threads_ = num_threads;
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::threadRun_fn(IRunnable* runnable, int num_total_tasks, int* done_tasks, std::mutex* mutex) {
    int cnt = 0;
    while (cnt < num_total_tasks) {
        mutex->lock();
        cnt = *done_tasks;
        *done_tasks += 1;
        mutex->unlock();
        if (cnt >= num_total_tasks) {
            break;
        }
        else{
            runnable->runTask(cnt, num_total_tasks);
        }
    }
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::thread* threads = new std::thread[this->num_threads_];
    int tmp = 0;
    int* done_tasks = &tmp;
    std::mutex* mutex = new std::mutex();
    for (int i = 0; i < this->num_threads_; i++) {
        threads[i] = std::thread(&TaskSystemParallelSpawn::threadRun_fn, this, runnable, num_total_tasks, done_tasks, mutex);
    }
    //Wait for spawn threads to complete
    for (int i = 0; i < this->num_threads_; i++) {
        threads[i].join();
    }
    delete[] threads;
    delete mutex;
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

TaskState::TaskState() {
    this->num_total_tasks = 0;
    this->done_tasks = 0;
    this->mutex = new std::mutex();
    this->finish_mutex = new std::mutex();
    this->finish_cond = new std::condition_variable();
    this->runnable = nullptr;
    this->num_total_tasks = 0;
}

TaskState::~TaskState() {
    delete this->mutex;
    delete this->finish_mutex;
    delete this->finish_cond;
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads_ = num_threads;
    this->threads = new std::thread[num_threads];
    this->mutex = new std::mutex();
    this->done_pool = false;
    this->done_tasks = 0;
    this->num_total_tasks = 0;
    this->left_tasks = 0;
    // this->task_state = new TaskState();
    for(int i=0; i < this->num_threads_; i++){
        this->threads[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::wait_fn, this);
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    this->done_pool = true;
    for (int i = 0; i < this->num_threads_; i++) {
        this->threads[i].join();
    }
    delete[] this->threads;
    delete this->mutex;
    // delete this->task_state;
}

void TaskSystemParallelThreadPoolSpinning::wait_fn() {
    int id;
    while (this->done_pool == false) {
        this->mutex->lock();
        if (this->done_tasks >= this->num_total_tasks) {
            this->mutex->unlock();
            continue;
        }
        id = this->num_total_tasks-this->left_tasks;
        if (id < this->num_total_tasks) {
            this->left_tasks--;
        }
        this->mutex->unlock();
        if (id<this->num_total_tasks){
            this->runnable->runTask(id, this->num_total_tasks);
            this->mutex->lock();
            this->done_tasks++;
            this->mutex->unlock();
        }

    }
    // while (this->done_pool == false) {
    //     this->task_state->mutex->lock();
    //     if (this->task_state->done_tasks >= this->task_state->num_total_tasks) {
    //         this->task_state->mutex->unlock();
    //         continue;
    //     }
    //     this->task_state->runnable->runTask(this->task_state->done_tasks, this->task_state->num_total_tasks);
    //     this->task_state->done_tasks++;
    //     this->task_state->mutex->unlock();
    // }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    // A lock must be held in order to wait on a condition variable.
    // This lock is atomically released before the thread goes to sleep
    // when `wait()` is called. The lock is atomically re-acquired when
    // the thread is woken up using `notify_all()`.
    // std::unique_lock<std::mutex> lk(*this->task_state->finish_mutex);
    // this->task_state->mutex->lock();
    // this->task_state->runnable = runnable;
    // this->task_state->num_total_tasks = num_total_tasks;
    // this->task_state->done_tasks = 0;
    // this->task_state->mutex->unlock();
    // this->task_state->finish_cond->wait(lk);
    // lk.unlock();
    
    this->mutex->lock();
    this->runnable = runnable;
    this->num_total_tasks = num_total_tasks;
    this->left_tasks = num_total_tasks;
    this->done_tasks = 0;
    this->mutex->unlock();
    bool done = false;
    while(done == false){
        this->mutex->lock();
        if(this->done_tasks >= this->num_total_tasks){
            done = true;
        }
        this->mutex->unlock();
    }
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

}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
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
