    #include <ros/ros.h>
    #include <serial/serial.h>  //ROS已經內置了的串口包
    #include <std_msgs/String.h>
    #include <std_msgs/Empty.h>
    #include "geometry_msgs/Pose.h"
    #include "nav_msgs/Path.h"
    #include "geometry_msgs/Twist.h"
	#include "nav_msgs/Odometry.h"
    #include <sensor_msgs/Imu.h> 
    #include "geometry_msgs/Vector3.h"
    #include <tf/transform_broadcaster.h>
	
    serial::Serial ser; //聲明串口對象
    int cnt = 0;
	struct var
	{
		float vector_linear;
		float vector_angular;
	};
	struct var vector_data;
      //回調函數
	void PoseCallback(const geometry_msgs::Twist::ConstPtr& msg_p) //void chatterCallback(const std_msgs::String::ConstPtr& msg)
	{
		vector_data.vector_linear = (float)(msg_p->linear.x);
    	vector_data.vector_angular = (float)(msg_p->angular.z);
		
    	ROS_INFO(" vector_data.vector_linear =  %f , vector_data.vector_angular= %f \n",vector_data.vector_linear, vector_data.vector_angular);
		uint8_t tx_buf[(4*2)+2] ={0};
		tx_buf[0] = 0xA0;
		tx_buf[(4*2)+2-1] = 0x0A;
		memcpy(tx_buf+1,(uint8_t*)(&vector_data),4*2);
		ser.write(tx_buf,(4*2)+2);
	}   
	int main (int argc, char** argv)
	{
      	//初始化節點
		ros::init(argc, argv, "Read_IMU_serial_node");
      	//聲明節點句柄
		ros::NodeHandle nh;
      	ros::NodeHandle n;
		std::string device;
    	ros::NodeHandle pn("~");
   	 	pn.param<std::string>("device",device,"/dev/ttyACM0");		  
   		std::cout<<"input="<<device<<std::endl;
		//訂閱主題，並配置回調函數
		ros::Subscriber sub = n.subscribe("/cmd_vel", 1000, PoseCallback);
		//發佈主題
		ros::Publisher odom_pub = n.advertise<nav_msgs::Odometry>("odom", 200);
		 
		tf::TransformBroadcaster odom_broadcaster;
		
		double x = 0.0, y = 0.0, th = 0.0;
		double vx = 0.0, vy = 0.0, vth = 0.0;
		ros::Time current_time, last_time;
		current_time = ros::Time::now();
		last_time = ros::Time::now();
		
		try
		{
			//設置串口屬性，並打開串口
			ser.setPort(device);
			ser.setBaudrate(115200);
			serial::Timeout to = serial::Timeout::simpleTimeout(1000);
			ser.setTimeout(to);
			ser.open();
		}
		catch (serial::IOException& e)
		{
			ROS_ERROR_STREAM("Unable to open port ");
			std::cout<<device<<std::endl;
			return -1;
		}
      //檢測串口是否已經打開，並給出提示信息
		if(ser.isOpen())
		{
			ROS_INFO_STREAM("Serial Port initialized");
			ROS_INFO_STREAM("Reading from serial port\n");
		}
		else
		{
			return -1;
		}
		//指定循環的頻率
		ros::Rate loop_rate(100);
		while(ros::ok())
		{
			if(ser.available())
			{
				
				ROS_INFO("%d\n",ser.available());
				uint8_t check = 0;
				ser.read(&check,1);
				if(check == 0xA0)
				{
					ser.read(&check,1);
				if(check == 0xA0)				
				{
					uint8_t read_data[20] = {0};
					int16_t data_name[10] {0};
					ser.read(read_data,20);
		
					for(int i = 0; i <10; i++)
					{
						data_name[i] = (int16_t)((read_data[i*2]<<8) | read_data[(i*2)+1]);
					}				
					//ROS_INFO(" odom_v:%f\t odom_w: %f\n",odom_data[0],odom_data[1]);									
					//odom_pub.publish(odom_rx_data);
					
					
					ros::spinOnce();               // check for incoming messages

					current_time = ros::Time::now();

					//compute odometry in a typical way given the velocities of the robot
					double dt = (current_time - last_time).toSec();
					double delta_x = (vx * cos(th) - vy * sin(th)) * dt;
					double delta_y = (vx * sin(th) + vy * cos(th)) * dt;
					double delta_th = vth * dt;

					x += delta_x;
					y += delta_y;
					th += delta_th;

					//since all odometry is 6DOF we'll need a quaternion created from yaw
					geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(th);

					//first, we'll publish the transform over tf
					geometry_msgs::TransformStamped odom_trans;
					odom_trans.header.stamp = current_time;
					odom_trans.header.frame_id = "odom";
					odom_trans.child_frame_id = "base_link";
					odom_trans.transform.translation.x = x;
					odom_trans.transform.translation.y = y;
					odom_trans.transform.translation.z = 0.0;
					odom_trans.transform.rotation = odom_quat;

					//send the transform
					odom_broadcaster.sendTransform(odom_trans);

					//next, we'll publish the odometry message over ROS
					nav_msgs::Odometry odom;
					odom.header.stamp = current_time;
					odom.header.frame_id = "odom";

					//set the position
					odom.pose.pose.position.x = x;
					odom.pose.pose.position.y = y;
					odom.pose.pose.position.z = 0.0;
					odom.pose.pose.orientation = odom_quat;

					//set the velocity
					odom.child_frame_id = "base_link";
					odom.twist.twist.linear.x = vx;
					odom.twist.twist.linear.y = vy;
					odom.twist.twist.angular.z = vth;

					//publish the message
					odom_pub.publish(odom);
					
					//clear serial buffer
					ser.flushInput ();
				}
			}
		}

      //處理ROS的信息，比如訂閱消息,並調用回調函數
      ros::spinOnce();
      loop_rate.sleep();
	}
}

