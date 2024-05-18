#include "task_executor.h"
#include <omp.h>
#include <iostream>
#include <vector>

void executeTask(const Task& task) {
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < static_cast<int>(task.commands.size()); ++i) {
        // Execute each command
        system(task.commands[i].c_str());
    }
}
