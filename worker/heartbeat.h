#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "scheduler.pb.h"
#include <string>
#include <vector>


void sendHeartbeats(const std::string& worker_id, int total_capacity);

#endif // HEARTBEAT_H
