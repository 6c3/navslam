

# 開啟 Realsense 相機
gnome-terminal -- bash -c "
source ~/cthree/slam/cam_ws/install/setup.bash
   
ros2 launch realsense2_camera rs_launch.py
exec bash
"

sleep 8

# 開啟 OVD-SLAM10 RGB-D
gnome-terminal -- bash -c "
source ~/cthree/slam/OVD-SLAM10/Examples/ros2/install/setup.sh
cd ~/cthree/slam/OVD-SLAM10
ros2 run orbslam3 rgbd_loc ~/cthree/slam/OVD-SLAM10/Vocabulary/ORBvoc.bin ~/cthree/slam/OVD-SLAM10/Examples/RGB-D/RealSense_D455.yaml
exec bash
"
sleep 8
#ros2 launch octomap_server2 octomap_server_launch.py
gnome-terminal -- bash -c "
source ~/cthree/slam/ros2esp/install/setup.bash
ros2 launch pointcloud_to_grid dual_output_example.launch.py topic:=OVD10/pointcloud

exec bash
"
gnome-terminal -- bash -c "
source ~/cthree/slam/ros2esp/install/setup.bash
sudo chmod 777 /dev/ttyUSB0
ros2 launch sllidar_ros2 sllidar_a1_launch.py

exec bash
"
gnome-terminal -- bash -c "
source ~/cthree/slam/ros2esp/install/setup.bash
sudo chmod 777 /dev/ttyACM0
ros2 launch serial2esp use.launch.py

exec bash
"

gnome-terminal -- bash -c "
source ~/cthree/slam/ros2esp/install/setup.bash
ros2 run tf2_ros static_transform_publisher 0.15 0 0.05 0 0 0 base_link laser_link

exec bash
"
gnome-terminal -- bash -c "
source ~/cthree/slam/ros2esp/install/setup.bash
ros2 run tf2_ros static_transform_publisher 0 0 0 0 0 0 map odom

exec bash
"
gnome-terminal -- bash -c "
source /opt/ros/humble/setup.bash
ros2 launch nav2_bringup navigation_launch.py params_file:=/home/agx/cthree/slam/nav2_params.yaml

exec bash
"