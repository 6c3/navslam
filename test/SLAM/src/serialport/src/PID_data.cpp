#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/Float32.h>
 #include <std_msgs/String.h>
#include <serial/serial.h> 
serial::Serial ser; //聲明串口對象
int main(int argc, char** argv){
  ros::init(argc, argv, "PID_publisher");
  ros::NodeHandle n;
  ros::Publisher PID_pub = n.advertise<std_msgs::Float32>("PID_data", 50);
  ros::Publisher ctl_pub = n.advertise<std_msgs::Float32>("ctl_data", 50);
	std::string device;
    ros::NodeHandle pn("~");
    pn.param<std::string>("device",device,"/dev/ttyACM0");		  
    std::cout<<"input="<<device<<std::endl;
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
      ros::Rate loop_rate(400);
		  
	int cont = 0;
      while(ros::ok())
      {
	//uint8_t a[11] = {'9','2','3','4','5','6','8','8','9'};
		 // ROS_INFO_STREAM("idie\n");
		  //ROS_INFO("%d\n",ser.available());

      if(ser.available())
	  {
		 
		 // ROS_INFO("%d\n",ser.available());
		  uint8_t check = 0;
		  ser.read(&check,1);
		  if(check == 0xA0)
		  {
			  ser.read(&check,1);
			 if(check == 0xA0)				
			 {
				  //ROS_INFO_STREAM("Reading from serial port\n");
				 uint8_t read_data[8] = {0};
				 //int16_t data_name[10] {0};
				 //float odom_data[2] = {0};
				ser.read(read_data,8);
				std_msgs::Float32  PID_rx_data;
				std_msgs::Float32  ctl_rx_data;
				//ROS_INFO(" linear_acceleration_x_y_z\n:[%f] \t[%f] \t [%f]\n",((double)data_name[0])/100,((double)data_name[1])/100,((double)data_name[2])/100);
				//ROS_INFO(" angular_velocity_x_y_z\n:%f\t%f\t%f\n",((double)data_name[3])/16,((double)data_name[4])/16,((double)data_name[5])/16);
				
				PID_rx_data.data = * (float*)read_data;
				ctl_rx_data.data = * (((float*)read_data)+1);
				 //ACC_pub.publish(acc_data);
				 cont++;
				 if(cont >= 10)
				 {
					 ROS_INFO(" PID_data %.3f",PID_rx_data.data);	
					 cont = 0;
				 }
					
				PID_pub.publish(PID_rx_data);
				 ctl_pub.publish(ctl_rx_data);
				//odom_pub.publish(odom_rx_data);
				ser.flushInput ();
			 }
		}


		//  std_msgs::String result;
		 // result.data = ser.read(ser.available());
		  //ROS_INFO_STREAM("Read: " << result.data);
		  //read_pub.publish(result);

  		 
      }

      //處理ROS的信息，比如訂閱消息,並調用回調函數
      ros::spinOnce();
      loop_rate.sleep();
      }
      }
