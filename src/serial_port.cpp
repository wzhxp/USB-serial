/*
参考：https://www.cxybb.com/article/liuxiaodong400/90370927
http://wjwwood.io/serial/doc/1.1.0/classserial_1_1_serial.html
*/
#include <ros/ros.h>
#include <serial/serial.h>
#include <iostream>
#include <vector>
#include <geometry_msgs/PointStamped.h>

int main(int argc, char **argv)
{
    ros::init(argc, argv, "serial_port");
    ros::NodeHandle n;

    serial::Serial sp;
    // 创建timeout
    serial::Timeout to = serial::Timeout::simpleTimeout(100);
    // 设置要打开的串口名称
    sp.setPort("/dev/ttyUSB0");
    // 设置串口通信的波特率
    sp.setBaudrate(115200);
    // 串口设置timeout
    sp.setTimeout(to);

    // 打开串口
    try
    {
        sp.open();
    }
    catch (serial::IOException &e)
    {
        ROS_ERROR_STREAM("Unable to open port.");
        return -1;
    }
    // 判断串口是否打开成功
    if (sp.isOpen())
    {
        ROS_INFO_STREAM("/dev/ttyUSB0 is opened.");
    }
    else
    {
        return -1;
    }

    // 创建发布对象
    ros::Publisher pub_uwbDate = n.advertise<geometry_msgs::PointStamped>("/uwb", 10);
    geometry_msgs::PointStamped uwbDate;

    while (ros::ok())
    {
        std::string buffer;
        // 读取一行数据，然后进行后续步骤，读取结束符/n
        size_t n = sp.readline(buffer);
        if (n != 0 && buffer.substr(0,2) == "MP")
        {
            std::stringstream ss(buffer);
            std::string str;
            std::vector<std::string> results;
            while (getline(ss, str, ','))
            {
                results.push_back(str);
            }
            int tag_id = std::stoi(results[1]);
            std::cout << "tag_id:" << tag_id << " ";
            double range = std::stod(results[4]);
            std::cout << "range:" << range << " ";
            double angle = std::stod(results[7]);
            std::cout << "angle:" << angle<< " "<< std::endl;

            // 把数据发送出去
            uwbDate.header.stamp = ros::Time::now();
            uwbDate.point.x = tag_id;
            uwbDate.point.y = range;
            uwbDate.point.z = angle;
            pub_uwbDate.publish(uwbDate);
        }
    }
    sp.close();
    return 0;
}
