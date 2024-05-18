#ifndef SCHEDULER_SERVICE_H
#define SCHEDULER_SERVICE_H

#include "scheduler.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <map>
#include <string>
#include <vector>

class SchedulerServiceImpl final : public SchedulerService::Service {
public:
    grpc::Status RegisterWorker(grpc::ServerContext* context, const WorkerInfo* request, RegisterReply* reply) override;
    grpc::Status AssignTask(grpc::ServerContext* context, const TaskRequest* request, TaskResponse* response) override;
    grpc::Status Heartbeat(grpc::ServerContext* context, const HeartbeatMessage* request, HeartbeatReply* reply) override;

private:
    std::map<std::string, WorkerInfo> workers_;
    std::vector<TaskResponse> task_queue_;
};

#endif // SCHEDULER_SERVICE_H
