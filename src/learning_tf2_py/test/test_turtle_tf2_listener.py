# Copyright 2021 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import rclpy


def test_frame_listener_node_init():
    """
    Import and instantiate the FrameListener node to validate it builds.

    This test ensures the node can be constructed without spinning or
    connecting to external ROS entities.
    """
    # Import here to ensure package import works
    from learning_tf2_py.turtle_tf2_listener import FrameListener

    rclpy.init()
    try:
        node = FrameListener()
        # basic sanity checks
        assert node.get_name() == 'turtle_tf2_frame_listener'
        assert hasattr(node, 'publisher')
        assert hasattr(node, 'tf_buffer')
        # clean up
        node.destroy_node()
    finally:
        rclpy.shutdown()
