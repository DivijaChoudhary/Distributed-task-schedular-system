#include "scheduler_service.h"
#include <iostream>

grpc::Status SchedulerServiceImpl::RegisterWorker(grpc::ServerContext* context, const WorkerInfo* request, RegisterReply* reply) {
    std::string worker_id = request->id();
    workers_[worker_id] = *request;

    reply->set_message("Worker registered successfully");
    reply->set_success(true);

    std::cout << "Worker registered: " << worker_id << std::endl;
    return grpc::Status::OK;
}

grpc::Status SchedulerServiceImpl::AssignTask(grpc::ServerContext* context, const TaskRequest* request, TaskResponse* response) {
    if (!task_queue_.empty()) {
        *response = task_queue_.front();
        task_queue_.erase(task_queue_.begin());
        response->set_has_task(true);
    } else {
        response->set_has_task(false);
    }
    return grpc::Status::OK;
}

grpc::Status SchedulerServiceImpl::Heartbeat(grpc::ServerContext* context, const HeartbeatMessage* request, HeartbeatReply* reply) {
    std::string worker_id = request->worker_id();
    if (workers_.find(worker_id) != workers_.end()) {
        workers_[worker_id].set_current_capacity(request->current_capacity());
        reply->set_message("Heartbeat received");
        reply->set_success(true);

        std::cout << "Heartbeat received from worker: " << worker_id << std::endl;
    } else {
        reply->set_message("Worker not registered");
        reply->set_success(false);
    }
    return grpc::Status::OK;
}
