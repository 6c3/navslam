cmake_minimum_required(VERSION 3.5)
project(serial2esp)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(serial REQUIRED)

include_directories(include)
include_directories(/usr/local/include)

# serial_odom_node
add_executable(serial_odom_node src/serialPort_odom_ros2.cpp)
ament_target_dependencies(serial_odom_node
  rclcpp
  geometry_msgs
  nav_msgs
  sensor_msgs
  tf2
  tf2_geometry_msgs
  serial
)

# pubcmd_node
add_executable(pubcmd_node src/pubcmd.cpp)
ament_target_dependencies(pubcmd_node
  rclcpp
  geometry_msgs
)

install(TARGETS serial_odom_node pubcmd_node
  DESTINATION lib/${PROJECT_NAME}
)
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)
ament_package()