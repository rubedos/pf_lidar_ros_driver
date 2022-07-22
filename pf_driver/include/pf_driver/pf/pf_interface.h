#ifndef PF_DRIVER_PF_INTERFACE_H
#define PF_DRIVER_PF_INTERFACE_H

#pragma once

#include <string>
#include <memory>
#include <future>
#include <dynamic_reconfigure/server.h>

#include "pf_driver/pf/pf_parser.h"
#include "pf_driver/pf/writer.h"
#include "pf_driver/pf/reader.h"
#include "pf_driver/communication.h"
#include "pf_driver/pf/r2000/pfsdp_2000.hpp"
#include "pf_driver/pf/r2300/pfsdp_2300.hpp"
#include "pf_driver/PFDriverR2000Config.h"
#include "pf_driver/PFDriverR2300Config.h"

class PFInterface
{
public:
  PFInterface() : state_(PFState::UNINIT)
  {
  }

  bool init(std::shared_ptr<HandleInfo> info, std::shared_ptr<ScanConfig> config,
            std::shared_ptr<ScanParameters> params, std::string topic, std::string frame_id, const uint16_t num_layers);

  bool start_transmission(std::shared_ptr<std::mutex> net_mtx, std::shared_ptr<std::condition_variable> net_cv,
                          bool& net_fail);
  void stop_transmission();
  void terminate();

private:
  using PipelinePtr = std::unique_ptr<Pipeline<PFPacket>>;

  ros::NodeHandle nh_;
  ros::Timer watchdog_timer_;
  std::unique_ptr<Transport> transport_;
  std::shared_ptr<PFSDPBase> protocol_interface_;
  PipelinePtr pipeline_;
  std::shared_ptr<Reader<PFPacket>> reader_;
  std::shared_ptr<std::mutex> config_mutex_;
  std::string topic_;
  std::string frame_id_;
  uint16_t num_layers_;
  std::string product_;

  std::shared_ptr<HandleInfo> info_;
  std::shared_ptr<ScanConfig> config_;
  std::shared_ptr<ScanParameters> params_;
  std::string prev_handle_;

  enum class PFState
  {
    UNINIT,
    INIT,
    RUNNING,
    SHUTDOWN,
    ERROR
  };
  PFState state_;

  bool init()
  {
    return init(info_, config_, params_, topic_, frame_id_, num_layers_);
  }

  void change_state(PFState state);
  bool can_change_state(PFState state);

  void start_watchdog_timer(float duration);
  void feed_watchdog(const ros::TimerEvent& e);  // timer based
  void on_shutdown();

  // factory functions
  bool handle_version(int major_version, int minor_version, int device_family, std::string topic, std::string frame_id,
                      const uint16_t num_layers);
  PipelinePtr get_pipeline(std::string packet_type, std::shared_ptr<std::mutex> net_mtx,
                           std::shared_ptr<std::condition_variable> net_cv, bool& net_fail);
  void connection_failure_cb();
};

#endif
