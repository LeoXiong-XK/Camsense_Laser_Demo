#include "lidar\include\lidar.h"
#include <stdio.h>

int main()
{
    const int data_num = 8;
    rangedata dataPack[data_num];
    Dev device;
    int fps = 0, rtn = 0;

    // ##### 1. Open serial port using valid COM id #####
    //rtn = device.openSerial("/dev/ttyUSB0", 115200);       // For Linux OS (replace '0' with the correct COM id)
    rtn = device.openSerial("com3", 115200);                       // For windows OS (replace '3' with the correct COM id)

    if (rtn < 0)
    {
        printf("Error: Unable to open serial port!\n");
        getchar();
        exit(0);
    }

    while (true)
    {
        // ##### 2. Read data from serial port #####
        rtn = device.ReadData_serial();
        if (rtn <= 0)
        {
            printf("Warning: Unable to read serial data!\n");
            continue;
        }

        // ##### 3. Parse data #####
        device.ParseData_serial(dataPack, fps);
        for (int i = 0; i < data_num; ++i)
        {
            printf("%d, dist: %d, angle: %f, is_invalid: %d\n",
                i, dataPack[i].dist, dataPack[i].angle, dataPack[i].flag);
        }
        printf("\n");
    }

    // ##### 4. Close serial #####
    device.closeSerial();
    return 0;

}

