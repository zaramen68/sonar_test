#include <chrono>
#include <thread>
#include <atomic>

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <iostream>
#include <mutex>
#include <string>
#include <modbus.h>
#include <assert.h>
#include <fstream>
#include <vector>
#include <string>


#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>	    // for UART

#include <ryml_std.hpp>
#include <ryml.hpp>

#include "yaml-cpp/yaml.h"

#include "debout.h"
#include "Task.h"

#include "uart.hpp"
#include "sonar.hpp"


struct _modbus_get_data {
    /* Slave address */
    int slave;
    /* Socket or file descriptor */
    int s;
    int debug;
    int error_recovery;
    struct timeval response_timeout;
    struct timeval byte_timeout;
    struct timeval indication_timeout;
    const void *backend;
    void *backend_data;
};




std::string get_file_contents(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in) {
        std::cerr << "could not open " << filename << std::endl;
        exit(1);
    }
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

enum LineType {
    UART_LINE = 0,
    MODBUS_LINE
};

struct Device{
    int id;
    int measure_reg;
    int test_reg;
};

void operator >>  (const YAML::Node& node, Device& device){
    node["id"] >> device.id;
    node["measure_reg"] >>device.measure_reg;
    node["test_reg"] >> device.test_reg;
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
    int baud;
    uint32_t boot_time;
    int data_bit;
    int stop_bit;
    char parity_check;
};

void operator >> (const YAML::Node& node, ULine& line){
    node["port"] >> line.port;
    node["baud"] >> line.baud;
    node["boot_time"] >> line.boot_time;
    node["data_bit"] >> line.data_bit;
    node["stop_bit"] >> line.stop_bit;
    node["parity_check"] >> line.parity_check;

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



int main(int argc, char *argv[])
{

    std::vector<std::shared_ptr<ModbusCtx>> mb_lines;
    std::vector<std::shared_ptr<UartCtx>> u_lines;

    uint32_t time_out = 2'000'000;
    std::ofstream out;
    std::string config_name = "/home/tenderbook/sonars/";
    std::chrono::steady_clock::time_point begin;

    if(argc > 1){
        config_name += argv[1];
    }
    using namespace std::chrono_literals;

    std::ifstream fin(config_name.c_str());
    YAML::Parser parser(fin);
    // YAML::Node conf = YAML::LoadFile(config_name.c_str());
    YAML::Node doc;

    parser.GetNextDocument(doc);

    Modbus_list ml;
    if(const YAML::Node *mb = doc.FindValue("ModBus")){
        *mb >> ml;
    }

    Uart_list ul;
    if(const YAML::Node *ua = doc.FindValue("Uart")){
        *ua >> ul;
    }

    for(MbLine line_ : ml.lines){
        std::shared_ptr<ModbusCtx> mb(new ModbusCtx{line_.port, line_.baud, 'N', 8, 1, time_out, line_.boot_time});
        for(Device device_ : line_.devices){
            SonarDev dev{device_.id, device_.test_reg, device_.measure_reg};
            mb->add_device(dev);
        }
        mb_lines.push_back(mb);
    }

    for(ULine line_ : ul.lines){
        std::shared_ptr<UartCtx> uart(new UartCtx{line_.port, line_.parity_check, line_.baud, line_.data_bit, line_.stop_bit});
        u_lines.push_back(uart);
    }


    begin = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point w_begin;
    w_begin = std::chrono::steady_clock::now();
    double w_time = 0.;

    while (w_time <= 200.) {
        w_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - w_begin).count();

        for(auto line:mb_lines ){
            if(!line->get_valid()) {
                continue;
            }

            for(auto dev:line->get_devices()){

                std::this_thread::sleep_for(0.02s); // меньше - ошибки


                std::string lable ="line:  " + line->get_port() + "   sonar:  "+ std::to_string(dev.get_id());
                if(dev.measure()){
                    double delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
                    double work_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                    // if(out.is_open()) out << dev.get_id() << "," <<  dev.get_distance() << "," << delta_time << "," << work_time << std::endl;
                    begin = std::chrono::steady_clock::now();

                    debug_counter(lable + "   distance=  ", dev.get_distance());
                } else {
                    debug_counter(lable + "   error #  ", dev.get_error_counter());
                }

            }

        }

        for(auto line:u_lines ){
            if(!line->get_valid()) {
                continue;
            }

            std::this_thread::sleep_for(0.02s); // меньше - ошибки

            std::string lable ="uart line:  " + line->get_port();
            if(line->measure()){
                double delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
                double work_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                // if(out.is_open()) out << 1 << "," <<  line.get_distance() << "," << delta_time << "," << work_time << std::endl;
                begin = std::chrono::steady_clock::now();

                debug_counter(lable + "   distance=  ", line->get_distance());
            } else {
                debug_counter(lable + "   error #  ", line->get_error_counter());
            }

        }
            // std::cout << "id: "<< id_<< ",  " << "rc: " << rc << '\n';
            // for (int i=0; i < rc; i++) {
            //     printf("reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
            // }

            // std::this_thread::sleep_for(0.1s);
        debprint.reinit();
    }

    // modbus_free(ctx);
    // out.close();

    return 0;
}
