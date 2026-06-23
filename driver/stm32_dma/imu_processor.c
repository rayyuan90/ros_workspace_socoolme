void Process_IMU_Data(uint8_t *data, uint16_t len) {
    // 假设数据包格式：[Header(0xAA)][AccX][AccY][AccZ][GyroX][GyroY][GyroZ][CRC]
    // 生产环境需添加 DMA 数据校验逻辑
    if (data[0] == 0xAA) {
        // 使用结构体映射直接解析，避免 memcpy
        typedef struct { int16_t ax, ay, az, gx, gy, gz; } IMU_Frame;
        IMU_Frame *frame = (IMU_Frame*)(data + 1);
        
        // 推送到 ROS 2 或控制循环
        Update_Kinematics(frame->ax, frame->ay, frame->az);
    }
}