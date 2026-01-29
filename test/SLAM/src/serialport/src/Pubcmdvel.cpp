#include <ros/ros.h>
#include "geometry_msgs/Twist.h"

int main(int argc, char **argv)
{
    // ROS??��l��
    ros::init(argc, argv, "pubcmd");

    // ?��??�y�`
    ros::NodeHandle n;

    // ?�ؤ@?Publisher�A?���W?/person_info��topic�A����?��?learning_topic::Person�A?�C?��10
    ros::Publisher person_info_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

    // ?�m�`?��?�v
    ros::Rate loop_rate(100);

    int count = 0;
    while (ros::ok())
    {
        // ��l��learning_topic::Person?��������
    	geometry_msgs::Twist pose;
		pose.linear.x   = 0.1;
		pose.angular.z  = 0.5;
		

        // ?������
		person_info_pub.publish(pose);

       	ROS_INFO("Publish  Info: linear:%f  angular:%f  ", 
				  pose.linear.x, pose.angular.z);

        // ���Ӵ`??�v��?
        loop_rate.sleep();
    }

    return 0;
}