#ifndef TASK_EXECUTOR_H
#define TASK_EXECUTOR_H

#include <vector>
#include <string>

struct Task {
    std::string id;
    std::vector<std::string> commands;
};

void executeTask(const Task& task);

#endif // TASK_EXECUTOR_H
