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
		
		/*
    if(cnt <=30)
    {
      uint8_t tx_buf[(cnt*4*2)+3] ={0};
      tx_buf[0] = 0xA0;
       tx_buf[1] = cnt;
      tx_buf[(cnt*4*2)+2] = 0x0A;
      for(int i = 0;i <(cnt*4*2)+3;i++)
        ROS_INFO("tx_buf[%d] =  %x",i,tx_buf[i]);
      
      memcpy(tx_buf+2,(uint8_t*)(&(pose_data[0])),cnt*4*2);
       ser.write(tx_buf,(cnt*4*2)+3);
    }
     cnt = 0;
     */
    // ROS_INFO("x0 [%f]",pose_data[0].position_x);
    // ROS_INFO("y0 [%f]",pose_data[0].position_y);
    // ROS_INFO("z0 [%f]",pose_data[0].position_z);
     //ROS_INFO("x [%f]",msg_p->poses[0].pose.position.x); //ROS_INFO("I heard: [%f]", msg->data.c_str());
    // ROS_INFO("recv data nav_msgs\n"); //ROS_INFO("I heard: [%f]", msg->data.c_str());
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
      //ros::Subscriber write_sub = nh.subscribe("write", 1000, write_callback);
      ros::Subscriber sub = n.subscribe("/cmd_vel", 1000, PoseCallback);
      //發佈主題
      //ros::Publisher read_pub = nh.advertise<sensor_msgs::Imu>("read", 1000);
	  ros::Publisher IMU_pub = n.advertise<sensor_msgs::Imu>("IMU_data", 1000);
	  ros::Publisher Euler_pub = nh.advertise<geometry_msgs::Vector3>("Euler_Angle", 1000);  
	 ros::Publisher ACC_pub = nh.advertise<geometry_msgs::Vector3>("ACC", 1000);  
	 ros::Publisher ang_pub = nh.advertise<geometry_msgs::Vector3>("ang", 1000);  
	  //ros::Publisher odom_pub = n.advertise<nav_msgs::Odometry>("Odometry_data", 1000);
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
      }
      else
      {
      return -1;
      }
      //指定循環的頻率
      ros::Rate loop_rate(100);
		  
	//int cont = 0;
      while(ros::ok())
      {
	//uint8_t a[11] = {'9','2','3','4','5','6','8','8','9'};
		 // ROS_INFO_STREAM("idie\n");
		  //ROS_INFO("%d\n",ser.available());

      if(ser.available())
	  {
		  ROS_INFO_STREAM("Reading from serial port\n");
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
				 //float odom_data[2] = {0};
				 ser.read(read_data,20);
				/*for(int i  = 0;i<20;i++)
				 {
					ROS_INFO("%x\t",read_data[i]); 
				 }*/
				 
				 //memcpy((uint8_t*)data_name,read_data,20);
				sensor_msgs::Imu imu_rx_data;		
				imu_rx_data.header.stamp = ros::Time::now();
				imu_rx_data.header.frame_id = "imu_link";
				 geometry_msgs::Vector3 rpy;
				 geometry_msgs::Vector3 acc_data;
				 geometry_msgs::Vector3 ang_data;
			/*	 
				 nav_msgs::Odometry  odom_rx_data;
				 odom_rx_data.header.stamp = ros::Time::now();
				 odom_rx_data.header.frame_id= "odom";
				 odom_rx_data.child_frame_id= "wheel";
				 */
				 for(int i = 0; i <10; i++)
				 {
				 	 data_name[i] = (int16_t)((read_data[i*2]<<8) | read_data[(i*2)+1]);
				 }
				 
				 /*odom_data[0] = *((float*)(read_data+12));//*((float*)(read_data+12))
				 odom_data[1] = *((float*)(read_data+16));//*((float*)(read_data+16))
				 
				 odom_rx_data.twist.twist.linear.x = (double)odom_data[0];
				 odom_rx_data.twist.twist.angular.z = (double)odom_data[1];
				 odom_rx_data.pose.pose.position.x = 0;
				 odom_rx_data.pose.pose.position.y = 0;
				 odom_rx_data.pose.pose.position.z = 0;
				 odom_rx_data.pose.pose.orientation.w = 0;
				*/ 
				  imu_rx_data.linear_acceleration.x = 0;//((double)data_name[1])/100;
				  imu_rx_data.linear_acceleration.y =0; //((double)data_name[0])/100*-1;
				 imu_rx_data.linear_acceleration.z = 9.8; //((double)data_name[2])/100;
				// imu_rx_data.angular_velocity.x =  ((double)data_name[4])/16;
				///  imu_rx_data.angular_velocity.y =  ((double)data_name[3])/16;
				 imu_rx_data.angular_velocity.z =  0;//(((double)data_name[5])/16*3.14159/180);
				 
				 //imu_rx_data.orientation.x = ((double)data_name[6])/10000;
				// imu_rx_data.orientation.y = ((double)data_name[7])/10000;
				// imu_rx_data.orientation.z = ((double)data_name[8])/10000;
				// imu_rx_data.orientation.w = ((double)data_name[9])/10000;
				 
				// acc_data.x = ((double)data_name[0])/100;
				// acc_data.y = ((double)data_name[1])/100;
				// acc_data.z = ((double)data_name[2])/100;
				 ang_data.x = ((double)data_name[3])/16;
				 ang_data.y = ((double)data_name[4])/16;
				 ang_data.z = ((double)data_name[5])/16;
				 
				 rpy.x=((double)data_name[6])/100;
				 rpy.y=((double)data_name[7])/100;
				 rpy.z=((double)data_name[8])/100;
				 		 
				ROS_INFO(" linear_acceleration_x_y_z\n:[%f] \t[%f] \t [%f]\n",((double)data_name[0])/100,((double)data_name[1])/100,((double)data_name[2])/100);
				ROS_INFO(" angular_velocity_x_y_z\n:%f\t%f\t%f\n",((double)data_name[3])/16,((double)data_name[4])/16,((double)data_name[5])/16);
				//ROS_INFO(" odom_v:%f\t odom_w: %f\n",odom_data[0],odom_data[1]);									
				IMU_pub.publish(imu_rx_data);
				Euler_pub.publish(rpy);
				 //ACC_pub.publish(acc_data);
				ang_pub.publish(ang_data);
				//odom_pub.publish(odom_rx_data);
				ser.flushInput ();
			 }
		}


		//  std_msgs::String result;
		 // result.data = ser.read(ser.available());
		  //ROS_INFO_STREAM("Read: " << result.data);
		  //read_pub.publish(result);

		  //int s = 5;
		  //int num = ser.write(&a[0],7);
		//ROS_INFO_STREAM("Write num : " << num <<"  cont:" << cont);
		//cont++;
  		 
      }

      //處理ROS的信息，比如訂閱消息,並調用回調函數
      ros::spinOnce();
      loop_rate.sleep();
      }
      }

