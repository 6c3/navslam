import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/agx/cthree/slam/nav2fishbot/install/nav2_simple_commander'
