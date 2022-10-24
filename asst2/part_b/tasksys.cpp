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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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
    num_threads_ = num_threads;
    threads = new std::thread[num_threads_];
    current_task_group_id = 0;
    done_pool = false;
    run_task_mutex = new std::mutex();
    finished_task_groups_mutex = new std::mutex();
    run_task_cv = new std::condition_variable();
    finished_task_groups_cv = new std::condition_variable();

    for (int i = 0; i < num_threads_; i++) {
        threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::sleepingthread_func, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    done_pool = true;
    run_task_cv->notify_all();
    for (int i = 0; i < num_threads_; i++) {
        threads[i].join();
    }
    delete[] threads;
    delete run_task_mutex;
    delete finished_task_groups_mutex;
    delete run_task_cv;
    delete finished_task_groups_cv;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    std::vector<TaskID> noDeps;
    runAsyncWithDeps(runnable, num_total_tasks, noDeps);
    sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    // pushing a record corresponding to the bulk task launch, or perhaps records corresponding to 
    // each of the tasks in the bulk task launch onto a "work queue". Once the record to work to do 
    // is in the queue, runAsyncWithDeps() can return to the caller.
    task_group_id_to_task_group[current_task_group_id] = new WaitingTask(runnable, num_total_tasks,current_task_group_id);
    // std::cout<<"group id "<<current_task_group_id<<std::endl;
    if (deps.empty()){
        task_group_deps[current_task_group_id];
        //put into ready task group queue
        // appendReadyTaskGroup(current_task_group_id);
    }
    else{
        for(TaskID dep_task:deps){
            task_group_deps[current_task_group_id].insert(dep_task);
            // std::cout<<"group id :"<<current_task_group_id<<"'s dep_task :"<<dep_task<<std::endl;
        }
    }
    return current_task_group_id++;
}

void TaskSystemParallelThreadPoolSleeping::sleepingthread_func(){
    while(done_pool == false){
        std::unique_lock<std::mutex> worker_lock(*run_task_mutex);
        run_task_cv->wait(worker_lock, [this] {return !ready_tasks.empty() || done_pool;});
        // std::cout<<"lock released"<<std::endl;
        if (done_pool){
            return;
        }
        //run ready task 
        auto task = ready_tasks.front();
        ready_tasks.pop();
        worker_lock.unlock();
        // std::cout<<"run task "<<task->current_task_id<<" of task group"<<task->task_group_id<<std::endl;
        task->runnable->runTask(task->current_task_id, task->num_total_tasks);

        finished_task_groups_mutex->lock();
        // std::cout<<"task group"<<task->task_group_id<<" left task "<<left_tasks[task->task_group_id]<<std::endl;
        left_tasks[task->task_group_id]--;
        if (left_tasks[task->task_group_id] == 0){
            // std::cout<<"task group "<<task->task_group_id<<" finished."<<std::endl;
            finished_task_groups.push(task->task_group_id);
            finished_task_groups_mutex->unlock();
            finished_task_groups_cv->notify_one();
        }
        else{
            finished_task_groups_mutex->unlock();
        }
    }
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    searchReadyTaskGroup();
    bool done = false;
    while(done==false){
        std::unique_lock<std::mutex> lock(*finished_task_groups_mutex);
        finished_task_groups_cv->wait(lock, [this] { return !finished_task_groups.empty(); });

        // std::cout<<"successfully finished"<<std::endl;
        while(!finished_task_groups.empty()){
            // std::cout<<"finished task group "<<finished_task_groups.front()<<std::endl;
            int task_group_id = finished_task_groups.front();
            left_tasks.erase(left_tasks.find(task_group_id));
            finished_task_groups.pop();
            lock.unlock();
            //remove depenency on this task group
            for(auto it = task_group_deps.begin(); it != task_group_deps.end(); it++){
                it->second.erase(task_group_id);
            }
            lock.lock();
        }
        lock.unlock();
        searchReadyTaskGroup();
        lock.lock();
        for(auto it = left_tasks.begin(); it != left_tasks.end(); it++){
            if (it->second != 0){
                std::cout<<"left task group: "<<it->first<<" has "<<it->second<<" left tasks."<<std::endl;
            }
        }
        done = left_tasks.empty();//check if all task groups are finished
        lock.unlock();
    }
    return;
}

void TaskSystemParallelThreadPoolSleeping::appendReadyTaskGroup(TaskID task_group_id){
    WaitingTask* task_group = task_group_id_to_task_group[task_group_id];
    left_tasks[task_group_id] = task_group->num_total_tasks;
    run_task_mutex->lock();
    for(int i = 0; i < task_group->num_total_tasks; i++){
        ready_tasks.push(new ReadyTask(task_group->runnable, task_group->num_total_tasks, task_group_id, i));
        // std::cout<<"pushed ready task "<<i<<" of group "<<task_group_id<<std::endl;
    }
    run_task_mutex->unlock();
    run_task_cv->notify_all();
    return;
}

void TaskSystemParallelThreadPoolSleeping::searchReadyTaskGroup(){
    for(auto it = task_group_deps.begin(); it != task_group_deps.end();){
        if(it->second.empty()){
            appendReadyTaskGroup(it->first);
            it = task_group_deps.erase(it);
        }
        else{
            it++;
        }
    }
    return;
}
