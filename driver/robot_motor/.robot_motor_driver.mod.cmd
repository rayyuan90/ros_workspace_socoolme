savedcmd_robot_motor_driver.mod := printf '%s\n'   robot_motor_driver.o | awk '!x[$$0]++ { print("./"$$0) }' > robot_motor_driver.mod
