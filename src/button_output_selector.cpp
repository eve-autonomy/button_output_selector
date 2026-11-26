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


#include <chrono>
#include <memory>
#include <utility>
#include "button_output_selector/button_output_selector.hpp"

namespace button_output_selector
{

ButtonOutputSelector::ButtonOutputSelector(
  const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
: Node("button_output_selector", options)
{
  using std::placeholders::_1;

  operator_mode_ = Operator::DRIVER;

  sub_button_ = this->create_subscription<VehicleButton>(
    "input/button", rclcpp::QoS{1}.transient_local(),
    std::bind(&ButtonOutputSelector::onButton, this, _1));
  sub_get_operator_ = this->create_subscription<Operator>(
    "/api/external/get/operator", rclcpp::QoS{1},
    std::bind(&ButtonOutputSelector::onOperator, this, _1));

  pub_button_autonomous_ = this->create_publisher<VehicleButton>(
    "output/button_autonomous", rclcpp::QoS{1}.transient_local());
  pub_button_driver_ = this->create_publisher<VehicleButton>(
    "output/button_driver", rclcpp::QoS{1}.transient_local());

  use_double_button_ = this->declare_parameter<bool>("use_double_button");
  decision_grace_sec_ = this->declare_parameter<double>("decision_grace_sec");
  max_wait_sec_ = this->declare_parameter<double>("max_wait_sec");
  engage_released_time_ = this->now();
  last_accepted_time_ = this->now();
  pending_time_ = this->now();

  if (use_double_button_) {
    sub_engage_pressed_ = this->create_subscription<std_msgs::msg::Bool>(
      "input/engage_button_pressed", rclcpp::QoS{1}.transient_local(),
      [this](const std_msgs::msg::Bool::ConstSharedPtr msg) {
        if (engage_pressed_ && !msg->data) {
          engage_released_time_ = this->now();
        }
        engage_pressed_ = msg->data;
      });
    sub_double_engage_accepted_ = this->create_subscription<VehicleButton>(
      "input/double_engage_accepted", rclcpp::QoS{10},
      [this](const VehicleButton::ConstSharedPtr) {
        has_accepted_ = true;
        last_accepted_time_ = this->now();
      });
    timer_ = rclcpp::create_timer(
      this, get_clock(), std::chrono::milliseconds(50),
      std::bind(&ButtonOutputSelector::onTimer, this));
  }
}

void ButtonOutputSelector::publishButton(const VehicleButton & msg)
{
  if (operator_mode_ == Operator::AUTONOMOUS) {
    pub_button_autonomous_->publish(msg);
  } else {
    pub_button_driver_->publish(msg);
  }
}

void ButtonOutputSelector::onButton(const VehicleButton::ConstSharedPtr msg)
{
  // Conventional behavior. In the MANUAL mode (shutdown button) the button is always passed.
  if (!use_double_button_ || operator_mode_ != Operator::AUTONOMOUS) {
    publishButton(*msg);
    return;
  }
  // Double button & AUTONOMOUS mode: judge later in onTimer()
  pending_msg_ = *msg;
  pending_time_ = this->now();
  has_pending_ = true;
}

void ButtonOutputSelector::onTimer()
{
  if (!has_pending_) {
    return;
  }
  const auto now = this->now();
  // Wait for the engage button to be released (but not forever: it might be stuck)
  if (engage_pressed_ && (now - pending_time_).seconds() < max_wait_sec_) {
    return;
  }
  const auto base = (engage_released_time_ > pending_time_) ? engage_released_time_ : pending_time_;
  if ((now - base).seconds() < decision_grace_sec_) {
    return;
  }

  // The reservation button was pressed from about (stamp - 0.5 - hold_down_time).
  // If the double button engage was accepted after that, the press was a part of the engage.
  const double margin = 1.0;
  const auto press_start =
    pending_time_ - rclcpp::Duration::from_seconds(pending_msg_.hold_down_time + margin);
  const bool used_for_engage = has_accepted_ && (last_accepted_time_ >= press_start);

  has_pending_ = false;
  if (used_for_engage) {
    RCLCPP_INFO(
      this->get_logger(),
      "[button_output_selector] reservation button was used for the double button engage. Dropped.");
    return;
  }
  // Operator mode might have changed while waiting
  publishButton(pending_msg_);
}

void ButtonOutputSelector::onOperator(const Operator::ConstSharedPtr msg)
{
  operator_mode_ = msg->mode;
}

}  // namespace button_output_selector

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(button_output_selector::ButtonOutputSelector)
