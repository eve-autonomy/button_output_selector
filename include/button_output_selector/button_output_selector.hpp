// Copyright 2022 eve autonomy inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#ifndef BUTTON_OUTPUT_SELECTOR__BUTTON_OUTPUT_SELECTOR_HPP_
#define BUTTON_OUTPUT_SELECTOR__BUTTON_OUTPUT_SELECTOR_HPP_

#include <string>
#include <list>
#include <vector>
#include <climits>

#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/bool.hpp"
#include "autoware_state_machine_msgs/msg/vehicle_button.hpp"
#include "tier4_external_api_msgs/msg/operator.hpp"

namespace button_output_selector
{
using autoware_state_machine_msgs::msg::VehicleButton;
using tier4_external_api_msgs::msg::Operator;

class ButtonOutputSelector : public rclcpp::Node
{
public:
  explicit ButtonOutputSelector(const rclcpp::NodeOptions & options);

private:
  uint8_t operator_mode_;

  // ---- double button (use_double_button:=true) ----
  // While the engage button and the reservation button are pressed together to depart,
  // the reservation button must not trigger the "delivery reservation" function.
  // So in the AUTONOMOUS mode the reservation button output is held until the engage button
  // is released, and dropped if the double button engage was accepted meanwhile.
  bool use_double_button_{false};
  double decision_grace_sec_;
  double max_wait_sec_;
  bool engage_pressed_{false};
  rclcpp::Time engage_released_time_;
  bool has_accepted_{false};
  rclcpp::Time last_accepted_time_;
  bool has_pending_{false};
  VehicleButton pending_msg_;
  rclcpp::Time pending_time_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_engage_pressed_;
  rclcpp::Subscription<VehicleButton>::SharedPtr sub_double_engage_accepted_;
  void onTimer();

  // Publisher
  rclcpp::Publisher<VehicleButton>::SharedPtr pub_button_autonomous_;
  rclcpp::Publisher<VehicleButton>::SharedPtr pub_button_driver_;

  // Subscription
  rclcpp::Subscription<Operator>::SharedPtr sub_get_operator_;
  rclcpp::Subscription<VehicleButton>::SharedPtr sub_button_;

  void publishButton(const VehicleButton & msg);
  void onButton(const VehicleButton::ConstSharedPtr msg);
  void onOperator(const Operator::ConstSharedPtr msg);
};

}  // namespace button_output_selector
#endif  // BUTTON_OUTPUT_SELECTOR__BUTTON_OUTPUT_SELECTOR_HPP_
