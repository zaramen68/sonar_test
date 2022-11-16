#if !defined( YAML_STR_H )
#define YAML_STR_H

#include <vector>
#include "yaml-cpp/yaml.h"
#include "common_f.hpp"


void operator >> (const YAML::Node& node, Convertion_data& data){
    node["m00"] >> data.m00;
    node["m01"] >> data.m01;
    node["m02"] >> data.m02;
    node["m10"] >> data.m10;
    node["m11"] >> data.m11;
    node["m12"] >> data.m12;
    node["m20"] >> data.m20;
    node["m21"] >> data.m21;
    node["m22"] >> data.m22;
    node["x_off"] >> data.x_off;
    node["y_off"] >> data.y_off;
    node["z_off"] >> data.z_off;
}


struct Device{
    int id;
    int measure_reg;
    int test_reg;
    int sonar_name;
    std::string topic_name;
    Convertion_data c_data;
};

void operator >>  (const YAML::Node& node, Device& device){
    node["id"] >> device.id;
    node["sonar_name"] >> device.sonar_name;
    node["measure_reg"] >>device.measure_reg;
    node["test_reg"] >> device.test_reg;
    node["topic_name"] >> device.topic_name;
    node["convertion_data"] >> device.c_data;
}

struct MbLine{
    std::string port;
    int baud;
    uint32_t boot_time;
    std::vector<Device> devices;
};

void operator >> (const YAML::Node& node, MbLine& line){
    node["port"] >> line.port;
    node["baud"] >> line.baud;
    node["boot_time"] >> line.boot_time;
    const YAML::Node& devices  = node["devices"];
    for(unsigned i = 0; i < devices.size(); i++){
        Device device;
        devices[i] >> device;
        line.devices.push_back(device);
    }
}

struct Modbus_list{
    std::vector<MbLine> lines;
};

void operator >> (const YAML::Node& node, Modbus_list& ml){
    for(unsigned i = 0; i < node.size(); i++){
        MbLine line;
        node[i] >> line;
        ml.lines.push_back(line);
    }
}

struct ULine{
    std::string port;
    std::string topic_name;
    int sonar_name;
    int baud;
    uint32_t boot_time;
    int data_bit;
    int stop_bit;
    char parity_check;
    Convertion_data c_data;
};

void operator >> (const YAML::Node& node, ULine& line){
    node["port"] >> line.port;
    node["topic_name"] >> line.topic_name;
    node["sonar_name"] >> line.sonar_name;
    node["baud"] >> line.baud;
    node["boot_time"] >> line.boot_time;
    node["data_bit"] >> line.data_bit;
    node["stop_bit"] >> line.stop_bit;
    node["parity_check"] >> line.parity_check;
    node["convertion_data"] >> line.c_data;

}

struct Uart_list{
    std::vector<ULine> lines;
};

void operator >> (const YAML::Node& node, Uart_list& ul){
    for(unsigned i = 0; i < node.size(); i++){
        ULine line;
        node[i] >> line;
        ul.lines.push_back(line);
    }
}

#endif