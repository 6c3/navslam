#!/usr/bin/env python
# -*- coding: UTF-8 -*-

################################################
# Copyright(c): 2016-2018 www.corvin.cn
################################################
# Author: corvin
################################################
# Description:
# 在地圖上指定6個座標點作爲巡邏點,可以在這些點
# 之間進行不斷的巡邏,也可以指定巡邏的圈數,當到
# 達指定的圈數後就會停止運行.該patrol_nav_node,
# 是通過向MoveBaseGoal的target_pose中發佈目標
# 位姿來達到巡航的目的,根據move_base的action狀態
# 來判斷機器人是否到達了目標位置.當到達了目標位置
# 後取出下一個目標位姿進行導航.直到字典存儲的所有
# 目標位姿都到達後,就認爲巡航一圈結束了.
################################################
# History:
#   20180806: initial this file.
################################################
import rospy
import random
import actionlib
from actionlib_msgs.msg import *
from geometry_msgs.msg import Pose, Point, Quaternion
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal

class PatrolNav():

    def __init__(self):
        rospy.init_node('patrol_nav_node', anonymous=False)
        rospy.on_shutdown(self.shutdown)

        # From launch file get parameters
        self.rest_time     = rospy.get_param("~rest_time", 2)
        self.keep_patrol   = rospy.get_param("~keep_patrol",   True)
        self.random_patrol = rospy.get_param("~random_patrol", False)
        self.patrol_type   = rospy.get_param("~patrol_type", 0)
        self.patrol_loop   = rospy.get_param("~patrol_loop", 2)
        self.patrol_time   = rospy.get_param("~patrol_time", 5)

        #set all navigation target pose
        self.locations = dict()
        self.locations['one']   = Pose(Point(2.842,  -1.130, 0.000), Quaternion(0.000, 0.000, 0.006, 1.000))
        self.locations['two']   = Pose(Point(4.506,  -1.026, 0.000), Quaternion(0.000, 0.000, 0.646, 0.763))
        self.locations['three'] = Pose(Point(4.593,  2.040, 0.000), Quaternion(0.000, 0.000, 0.692, 0.722))
        self.locations['four']  = Pose(Point(4.434, 4.387, 0.000), Quaternion(0.000, 0.000, 0.989, -0.151 ))
        self.locations['five']  = Pose(Point(2.557, 4.009, 0.000), Quaternion(0.000, 0.000, -0.713, 0.702))
        self.locations['six']   = Pose(Point(0.031, 0.060, 0.000), Quaternion(0.000, 0.000, 0.145, 0.990))

        # Goal state return values
        goal_states = ['PENDING', 'ACTIVE', 'PREEMPTED', 'SUCCEEDED', 'ABORTED',
                       'REJECTED', 'PREEMPTING', 'RECALLING', 'RECALLED', 'LOST']

        # Subscribe to the move_base action server
        self.move_base = actionlib.SimpleActionClient("move_base", MoveBaseAction)
        self.move_base.wait_for_server(rospy.Duration(30))
        rospy.loginfo("Connected to move base server")

        # Variables to keep track of success rate, running time etc.
        loop_cnt = 0
        n_goals  = 0
        n_successes  = 0
        target_num   = 0
        running_time = 0
        location   = ""
        start_time = rospy.Time.now()
        locations_cnt = len(self.locations)
        sequeue = ['one', 'two', 'three', 'four', 'five', 'six']

        rospy.loginfo("Starting position navigation ")
        # Begin the main loop and run through a sequence of locations
        while not rospy.is_shutdown():
            #judge if set keep_patrol is true
            if self.keep_patrol == False:
                if self.patrol_type == 0: #use patrol_loop
                    if target_num == locations_cnt :
                      if loop_cnt < self.patrol_loop-1:
                        target_num = 0
                        loop_cnt  += 1
                        rospy.logwarn("Left patrol loop cnt: %d", self.patrol_loop-loop_cnt)
                      else:
                        rospy.logwarn("Now patrol loop over, exit...")
                        rospy.signal_shutdown('Quit')
                        break
            else:
                if self.random_patrol == False:
                    if target_num == locations_cnt:
                        target_num = 0
                else:
                    target_num = random.randint(0, locations_cnt-1)

            # Get the next location in the current sequence
            location = sequeue[target_num]
            rospy.loginfo("Going to: " + str(location))
            self.send_goal(location)

            # Increment the counters
            target_num += 1
            n_goals    += 1

            # Allow 5 minutes to get there
            finished_within_time = self.move_base.wait_for_result(rospy.Duration(300))
            # Check for success or failure
            if not finished_within_time:
                self.move_base.cancel_goal()
                rospy.logerr("ERROR:Timed out achieving goal")
            else:
                state = self.move_base.get_state()
                if state == GoalStatus.SUCCEEDED:
                    n_successes += 1
                    rospy.loginfo("Goal succeeded!")
                else:
                    rospy.logerr("Goal failed with error code:"+str(goal_states[state]))

            # How long have we been running?
            running_time = rospy.Time.now() - start_time
            running_time = running_time.secs/60.0

            # Print a summary success/failure and time elapsed
            rospy.loginfo("Success so far: " + str(n_successes) + "/" +
                          str(n_goals) + " = " +
                          str(100 * n_successes/n_goals) + "%")
            rospy.loginfo("Running time: " + str(self.trunc(running_time, 1)) + " min")
            rospy.sleep(self.rest_time)

            if self.keep_patrol == False and self.patrol_type == 1: #use patrol_time
                if running_time >= self.patrol_time:
                    rospy.logwarn("Now reach patrol_time, back to original position...")
                    self.send_goal('six')
                    rospy.signal_shutdown('Quit')

    def send_goal(self, locate):
        # Set up the next goal location
        self.goal = MoveBaseGoal()
        self.goal.target_pose.pose = self.locations[locate]
        self.goal.target_pose.header.frame_id = 'map'
        self.goal.target_pose.header.stamp = rospy.Time.now()
        self.move_base.send_goal(self.goal) #send goal to move_base

    def trunc(self, f, n):
        # Truncates/pads a float f to n decimal places without rounding
        slen = len('%.*f' % (n, f))
        return float(str(f)[:slen])

    def shutdown(self):
        self.move_base.cancel_goal()
        rospy.logwarn("Stopping the patrol...")

if __name__ == '__main__':
    try:
        PatrolNav()
        rospy.spin()
    except rospy.ROSInterruptException:
        rospy.logwarn("patrol navigation exception finished.")
