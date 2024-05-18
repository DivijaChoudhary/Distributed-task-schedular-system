#include "utils.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>

#define NOMINMAX
#include <windows.h>
#include <algorithm>  

std::mutex task_mutex;  // Mutex to protect access to tasks_in_progress vector
std::vector<TaskStatus> tasks_in_progress;  // Vector to store the current tasks being processed
std::atomic<bool> is_running(true);  // Atomic flag to control the running state of the worker
int total_capacity = std::thread::hardware_concurrency();  // Initial total capacity based on system's hardware concurrency
int current_capacity = total_capacity;  // Current available capacity, initially set to total_capacity

// Function to get the CPU load from the system using Windows API
double getCPULoad() {
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime) != 0) {
        static ULONGLONG last_idle_time = 0, last_kernel_time = 0, last_user_time = 0;

        ULONGLONG idle_time = *((ULONGLONG*)&idleTime);
        ULONGLONG kernel_time = *((ULONGLONG*)&kernelTime);
        ULONGLONG user_time = *((ULONGLONG*)&userTime);

        ULONGLONG total_time = (kernel_time - last_kernel_time) + (user_time - last_user_time);
        ULONGLONG idle_diff = idle_time - last_idle_time;

        last_idle_time = idle_time;
        last_kernel_time = kernel_time;
        last_user_time = user_time;

        if (total_time == 0) return 0.0;
        return (double)(total_time - idle_diff) * 100.0 / total_time;
    }
    return 0.0;
}

// Function to calculate the total capacity of the worker based on current system load
int calculateTotalCapacity() {
    double cpuLoad = getCPULoad();
    double highLoadThreshold = 0.75;  // Threshold for high system load

    if (cpuLoad > highLoadThreshold) {
        // Reduce capacity under high load conditions
        return std::max(1, total_capacity / 2);  // Ensure at least one task can run
    } else {
        return total_capacity;
    }
}

// Function to calculate the current capacity of the worker by checking tasks in progress
int calculateCurrentCapacity() {
    std::lock_guard<std::mutex> lock(task_mutex);
    int used_capacity = std::count_if(tasks_in_progress.begin(), tasks_in_progress.end(), [](const TaskStatus& task) {
        return task.status() == "in-progress";
    });
    return calculateTotalCapacity() - used_capacity;
}

// Function to fetch the list of tasks currently in progress
std::vector<TaskStatus> getTasksInProgress() {
    std::lock_guard<std::mutex> lock(task_mutex);
    return tasks_in_progress;
}

// Function to send a heartbeat message to the scheduler
void sendHeartbeatToScheduler(const HeartbeatMessage& heartbeat) {
    std::cout << "Sending heartbeat..." << std::endl;
    std::cout << "Worker ID: " << heartbeat.worker_id() << std::endl;
    std::cout << "Current Capacity: " << calculateCurrentCapacity() << std::endl;
    for (const auto& task : heartbeat.tasks()) {
        std::cout << "Task ID: " << task.task_id() << ", Status: " << task.status() << std::endl;
    }
}

// Function to add a task to the list of tasks in progress
void addTask(const TaskStatus& task) {
    std::lock_guard<std::mutex> lock(task_mutex);
    if (is_running) {
        tasks_in_progress.push_back(task);
    }
}

// Function to remove a task from the list of tasks in progress and mark it as completed
void completeTask(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(task_mutex);
    auto it = std::find_if(tasks_in_progress.begin(), tasks_in_progress.end(), [&task_id](const TaskStatus& task) {
        return task.task_id() == task_id;
    });
    if (it != tasks_in_progress.end()) {
        it->set_status("completed");
    }
}

// Function to check if the worker is currently overloaded
bool isOverloaded() {
    return calculateCurrentCapacity() <= 0;
}

// Function to handle the graceful shutdown of the worker, stopping all tasks and clearing resources
void shutdownWorker() {
    std::lock_guard<std::mutex> lock(task_mutex);
    std::cout << "Shutting down worker..." << std::endl;
    is_running = false; // Stop accepting new tasks
    // Optionally wait for all tasks to complete or handle them accordingly
    for (auto& task : tasks_in_progress) {
        task.set_status("stopped");
    }
    tasks_in_progress.clear();  // Clear all tasks if not needed to resume after restart
}
