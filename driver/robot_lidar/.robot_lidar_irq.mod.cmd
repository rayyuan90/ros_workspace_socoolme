savedcmd_robot_lidar_irq.mod := printf '%s\n'   robot_lidar_irq.o | awk '!x[$$0]++ { print("./"$$0) }' > robot_lidar_irq.mod
