# Button output selector

## Overview

This node assigns button actions to topics to be published according to operator status that Autoware has.

1. If operator is `AUTONOMOUS`, it functions as delivery reservation button.
1. If other than 1, it functions as shutdown button.

## Input and Output

- Input
  - from [autoware.universe](https://github.com/autowarefoundation/autoware.universe)
    - `/api/external/get/operator` \[[tier4_external_api_msgs/msg/operator][Operator]\]:<br>Operator Status.
  - from delivery_reservation_/[button_manager](https://github.com/eve-autonomy/button_manager/)
    - `delivery_reservation_button_manager/output/delivery_reservation_button` \[[autoware_state_machine_msgs/msg/VehicleButton][ReservationButton]\]:<br>Delivery reservation button input. (this topic is remapped from `input/button`.)

- Output
  - to [autoware_state_machine](https://github.com/eve-autonomy/autoware_state_machine/)
    - `/delivery_reservation_button` \[[autoware_state_machine_msgs/msg/VehicleButton][ReservationButton]\]:<br>Serves as a reservation for delivery. (this topic is remapped from `output/button_autonomous`.)
  - to [shutdown_manager](https://github.com/eve-autonomy/shutdown_manager/)
    - `/shutdown_button` \[[autoware_state_machine_msgs/msg/VehicleButton][ReservationButton]\]:<br>Serves as a shutdown operation. (this topic is remapped from `output/button_driver`.)

[Operator]: https://github.com/tier4/tier4_autoware_msgs/blob/tier4/universe/tier4_external_api_msgs/msg/Operator.msg
[ReservationButton]: https://github.com/eve-autonomy/autoware_state_machine_msgs/blob/main/msg/VehicleButton.msg

## Node Graph

![node_graph](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/eve-autonomy/button_output_selector/main/docs/node_graph.pu)

## Parameter description

This node has no parameters.
