欢创科技激光雷达解析程序使用说明

Version: 1.5

Tutorial:

1. 执行程序编译命令
    1) Windows
    mkdir build
    cd build
    cmake -G "Visual Studio 11 2012" ..
    (注意，这里使用的编译器为vs2012,应根据实际情况进行修改)
    
    2) Linux (注意，linux下rangeView暂时不可用，cmake中rangeView对应部分需注释掉)
    mkdir build
    cd build
    cmake ..
    make
    
2. 修改端口号
    应根据激光雷达所连接的USB端口号，修改src/sdk_demo.cpp对应代码，具体请看代码注释。
    在修改好端口号后，再执行make，重新编译生成exe文件。

3. sdk_demo运行过程中的打印语句说明
    printf("%d, dist: %d, angle: %f, is_invalid: %d\n", i, dataPack[i].dist, dataPack[i].angle, dataPack[i].flag);
    从左到右分别是：
    i           -> 数据包序号(0-7)
    dist        -> 测距结果(单位：mm)
    angle       -> 当前数据包对应的角度
    is_invalid  -> 当前数据包有效性(0：有效，1：无效)
    
4. rangeView主要用于实时展示激光雷达点云图
    配置文件：根目录\bin\lib\Release\config\all_config.json
    关键配置参数：
        "port": 串口号
        "baudRate": 波特率
    
5. SDK引用说明：
    源码路径: 根目录\src\base
    生成静态库路径：根目录\dep\lidar，其中：
        引用头文件：include\lidar.h
        链接静态库：lib\lidar.lib
    