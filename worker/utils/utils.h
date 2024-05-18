#ifndef UTILS_H
#define UTILS_H

#include "scheduler.pb.h"
#include <vector>

// Function to calculate the total capacity of the worker based on system load
int calculateTotalCapacity();

// Function to calculate the current capacity of the worker based on system load
int calculateCurrentCapacity();

// Function to fetch the list of tasks currently in progress
std::vector<TaskStatus> getTasksInProgress();

// Function to send a heartbeat message to the scheduler
void sendHeartbeatToScheduler(const HeartbeatMessage& heartbeat);

// Function to add a task to the list of tasks in progress
void addTask(const TaskStatus& task);

// Function to remove a task from the list of tasks in progress
void completeTask(const std::string& task_id);

// Function to get the CPU load from the system
double getCPULoad();

// Function to check if the worker is overloaded
bool isOverloaded();

// Function to handle the graceful shutdown of the worker
void shutdownWorker();

#endif // UTILS_H
