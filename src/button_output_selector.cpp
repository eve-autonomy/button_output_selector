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
}

void ButtonOutputSelector::onButton(const VehicleButton::ConstSharedPtr msg)
{
  if (operator_mode_ == Operator::AUTONOMOUS) {
    pub_button_autonomous_->publish(*msg);
  } else {
    pub_button_driver_->publish(*msg);
  }
}

void ButtonOutputSelector::onOperator(const Operator::ConstSharedPtr msg)
{
  operator_mode_ = msg->mode;
}

}  // namespace button_output_selector

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(button_output_selector::ButtonOutputSelector)
