import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/agx/cthree/slam/test/ws2/install/fishbot_description'
