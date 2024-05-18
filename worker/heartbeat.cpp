#include "heartbeat.h"
#include <thread>
#include <chrono>
#include <iostream>
#include "utils/utils.h"
#include "scheduler.grpc.pb.h"

void sendHeartbeats(const std::string& worker_id, int total_capacity) {
    while (true) {
        int current_capacity = calculateCurrentCapacity();

        HeartbeatMessage heartbeat;
        heartbeat.set_worker_id(worker_id);
        heartbeat.set_current_capacity(current_capacity);

        auto tasks_in_progress = getTasksInProgress();
        for (const auto& task_status : tasks_in_progress) {
            TaskStatus* task = heartbeat.add_tasks();
            task->set_task_id(task_status.task_id());
            task->set_status(task_status.status());
            
        }

        sendHeartbeatToScheduler(heartbeat);

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
