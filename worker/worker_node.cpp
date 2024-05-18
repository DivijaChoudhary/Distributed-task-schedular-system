#include <grpcpp/grpcpp.h>
#include "scheduler.grpc.pb.h"
#include "task_executor.h"
#include "heartbeat.h"
#include "utils/utils.h"
#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// Global flag to control running state of the application
std::atomic<bool> running(true);

// Class definition for WorkerClient to handle communication with a scheduler server
class WorkerClient {
public:
    // Constructor: Initializes gRPC stub for communication with scheduler
    WorkerClient(std::shared_ptr<Channel> channel)
        : stub_(SchedulerService::NewStub(channel)) {}

    // Function to register the worker with the scheduler, providing IDs and capacities
    bool RegisterWorker(const std::string& id, int total_capacity, int current_capacity) {
        WorkerInfo request;
        request.set_id(id);
        request.set_total_capacity(total_capacity);
        request.set_current_capacity(current_capacity);

        RegisterReply reply;
        ClientContext context;

        Status status = stub_->RegisterWorker(&context, request, &reply);
        if (status.ok() && reply.success()) {
            std::cout << "Worker registered successfully." << std::endl;
            return true;
        } else {
            std::cerr << "Worker registration failed." << std::endl;
            return false;
        }
    }

    // Function to request a new task from the scheduler
    TaskResponse RequestTask(const std::string& worker_id) {
        TaskRequest request;
        request.set_worker_id(worker_id);

        TaskResponse response;
        ClientContext context;

        Status status = stub_->AssignTask(&context, request, &response);
        if (status.ok()) {
            return response;
        } else {
            std::cerr << "Task request failed." << std::endl;
            return TaskResponse();
        }
    }

    // Function to send a heartbeat message to the scheduler indicating current state
    void SendHeartbeat(const HeartbeatMessage& heartbeat) {
        HeartbeatReply reply;
        ClientContext context;

        Status status = stub_->Heartbeat(&context, heartbeat, &reply);
        if (!status.ok() || !reply.success()) {
            std::cerr << "Heartbeat failed." << std::endl;
        }
    }

private:
    std::unique_ptr<SchedulerService::Stub> stub_; // gRPC stub for communication
};

// Signal handler function to handle termination signals (e.g., SIGINT, SIGTERM)
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << ", shutting down." << std::endl;
    running = false;  // Signal all threads to shut down
}

// Main function of the worker node
int main(int argc, char** argv) {
    // Setup signal handling
    signal(SIGINT, signalHandler);  // Handle Ctrl+C
    signal(SIGTERM, signalHandler); // Handle software termination request

    std::string worker_id = "192.168.1.1"; // Example worker IP
    WorkerClient worker_client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    // Calculate capacity based on system resources
    int total_capacity = calculateTotalCapacity();
    int current_capacity = calculateCurrentCapacity();

    // Attempt to register worker with the scheduler
    if (!worker_client.RegisterWorker(worker_id, total_capacity, current_capacity)) {
        return -1;  // Exit if registration fails
    }

    // Start a separate thread to handle periodic heartbeat messages
    std::thread heartbeat_thread([&]() {
        while (running) {
            HeartbeatMessage heartbeat;
            heartbeat.set_worker_id(worker_id);
            heartbeat.set_current_capacity(calculateCurrentCapacity());
            auto tasks_in_progress = getTasksInProgress();
            for (const auto& task_status : tasks_in_progress) {
                TaskStatus* task = heartbeat.add_tasks();
                task->set_task_id(task_status.task_id());
                task->set_status(task_status.status());
            }

            worker_client.SendHeartbeat(heartbeat);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    // Main loop to process tasks
    while (running) {
        auto task_response = worker_client.RequestTask(worker_id);
        if (task_response.has_task()) {
            Task task;
            task.id = task_response.task_id();
            for (const auto& command : task_response.commands()) {
                task.commands.push_back(command);
            }

            TaskStatus task_status;
            task_status.set_task_id(task.id);
            task_status.set_status("in-progress");
            addTask(task_status);

            executeTask(task);

            completeTask(task.id);
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    // Wait for the heartbeat thread to finish before exiting
    heartbeat_thread.join();  
    return 0;
}
