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
    #include "math.h"
	
	#define pi 3.141593
 	#define rad (pi/180)
    serial::Serial ser; //聲明串口對象
    int cnt = 0,nav_status = 0;
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
		
    	//ROS_INFO(" vector_data.vector_linear =  %f , vector_data.vector_angular= %f \n",vector_data.vector_linear, vector_data.vector_angular);
		
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
		ros::Subscriber sub = n.subscribe("/cmd_vel", 100, PoseCallback);
		//發佈主題
		ros::Publisher odom_pub = n.advertise<nav_msgs::Odometry>("odom_data", 100);
		ros::Publisher imu_pub  = n.advertise<sensor_msgs::Imu>("imu_data",100);
		//tf::TransformBroadcaster odom_broadcaster;
		
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
		float last_v = 0.0 ,last_w = 0.0;
	   
	    int speed_cont = 0, cmd_val_pub = 0;
		while(ros::ok())
		{
            cmd_val_pub++;
            /*if(cmd_val_pub == 10)
            {
                cmd_val_pub = 0;
                //ROS_INFO(" vector_data.vector_linear =  %f , vector_data.vector_angular= %f \n",vector_data.vector_linear, vector_data.vector_angular);
                uint8_t tx_buf[(4*2)+2] ={0};
                tx_buf[0] = 0xA0;
                tx_buf[9] = 0x0A;
                //vector_data.vector_linear = -0.1f;
    	        //vector_data.vector_angular = 0.0f;
                memcpy(tx_buf+1,(uint8_t*)(&vector_data),4*2);
                ser.write(tx_buf,(4*2)+2);
                //ROS_INFO("%d\n",tx_buf[9]);
            }
            */

			if(ser.available())
			{
				
				//ROS_INFO("%d\n",ser.available());
				uint8_t check = 0;
				ser.read(&check,1);
				if(check == 0xA0)
				{
					ser.read(&check,1);
				if(check == 0xA0)				
				{
					uint8_t read_data[32] = {0};
					//float data_name[8] = {0};
					float *data_float = (float*)read_data;
					ser.read(read_data,32);
					current_time = ros::Time::now();
					//compute odometry in a typical way given the velocities of the robot
					double dt = (current_time - last_time).toSec();
					double delta_x = ((data_float[0]) * cos(th) - 0 * sin(th)) * dt;
					double delta_y = ((data_float[0]) * sin(th) + 0 * cos(th)) * dt;
					double delta_th = (data_float[1]) * dt;
				//	ROS_INFO("dt = %0.4f\n",dt);
					x += delta_x;
					y += delta_y;
					th += delta_th;
					
					speed_cont++;
                    if(speed_cont == 10)
                    {
                            ROS_INFO(" speed v:%f\t w:%f dt:%f\n",data_float[0],data_float[1],dt);
                            speed_cont = 0;
                    }

					//since all odometry is 6DOF we'll need a quaternion created from yaw
					geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(th);

					//next, we'll publish the odometry message over ROS
					nav_msgs::Odometry odom;
					odom.header.stamp = current_time;
					odom.header.frame_id = "odom";
                    odom.child_frame_id = "base_link";
                    
					//set the position
					odom.pose.pose.position.x = x;
					odom.pose.pose.position.y = y;
					odom.pose.pose.position.z = 0.0;
					odom.pose.pose.orientation = odom_quat;
					
					//set the velocity
					odom.twist.twist.linear.x = data_float[0];
					odom.twist.twist.linear.y = 0.0;
					odom.twist.twist.linear.z = 0.0;
					
					odom.twist.twist.angular.x = 0.0;
					odom.twist.twist.angular.y = 0.0;
					odom.twist.twist.angular.z = data_float[1];

                    
                    sensor_msgs::Imu imu;
                    imu.header.stamp = current_time;
                    imu.header.frame_id = "base_link";
                    
                    if (fabs(data_float[5]) <= 0.4)
                        data_float[5] = 0.0;
                    if (fabs(data_float[6]) <= 0.4)
                        data_float[6] = 0.0;
                    if (fabs(data_float[7]) <= 0.4)
                        data_float[7] = 0.0;
                        
                    imu.angular_velocity.x = data_float[5] * rad;
                    imu.angular_velocity.y = data_float[6] * rad;
                    imu.angular_velocity.z = data_float[7] * rad * -1.0f /2.0f;
                    
                    imu.linear_acceleration.x = data_float[2];
                    imu.linear_acceleration.y = data_float[3] * -1.0f;
                    imu.linear_acceleration.z = data_float[4] * -1.0f;
                    
					//publish the message
					odom_pub.publish(odom);
					imu_pub.publish(imu);
					last_time = current_time;	
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

