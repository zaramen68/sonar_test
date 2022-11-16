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


#include "mbsonar.hpp"
#include "uartsonar.hpp"

#include "debout.h"
#include "Task.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "yaml_str.hpp"
#include "yaml-cpp/yaml.h"
#pragma GCC diagnostic pop


template<typename T>
void work(std::shared_ptr< T > dev, const std::string &port){

    std::string const cloud_frame_id_{"cloud_frame"};

    auto now = std::chrono::steady_clock::now();
    const auto time_start = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();


    std::string lable = port;
    if(dev->measure()){
        // publish_laser_cloud(dev->get_publisher(), dev->get_obstacle(), cloud_frame_id_);

        debug_counter(lable + "   distance=  ", dev->get_distance()/1000.);
    } else {
        debug_counter(lable + "   no data  error_counter = ", dev->get_error_counter());
    }

    now = std::chrono::steady_clock::now();
    const auto time_stop = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();

    dev->get_debug(time_start, time_stop);
    debprint.reinit();
}

int main(int argc, char *argv[])
{

    std::vector<std::shared_ptr<ModbusCtx>> mb_lines;
    std::vector<std::shared_ptr<UartCtx>> u_lines;

    constexpr uint32_t time_out = 500'000;
    bool dump_flag = true;


    std::ofstream out;
    out.open(log_path+"time_log.data");

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

    //  ModBus devices list
    //

    for(MbLine line_ : ml.lines){
        std::shared_ptr<ModbusCtx> mb(new ModbusCtx{line_.port, line_.baud, 'N', 8, 1, time_out, line_.boot_time});
        for(Device device_ : line_.devices){

            std::shared_ptr< SonarDev > dev(new SonarDev{device_.id, device_.sonar_name, device_.test_reg, device_.measure_reg, device_.c_data, dump_flag});
            mb->add_device(dev);
        }
        mb_lines.push_back(mb);
    }

    for(ULine line_ : ul.lines){

        std::shared_ptr<UartCtx> uart(new UartCtx{line_.port, line_.sonar_name, line_.c_data, dump_flag});
        u_lines.push_back(uart);
    }


    begin = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point w_begin;
    w_begin = std::chrono::steady_clock::now();
    double w_time = 0.;

    while (w_time <= 20.) {
        w_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - w_begin).count();

        uint64_t delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - w_begin).count();
        uint64_t work_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        if(out.is_open()) { out  << delta_time << "," << work_time << std::endl; }

        std::vector<std::thread> mb_threads;
        for(auto line: mb_lines){
            if(!line->get_valid()){

                line->reconnect();
                continue;
            }
            std::this_thread::sleep_for(0.02s);                 // меньше - ошибки
            auto devices = line->get_devices();
            std::string lable = line->get_lable();

            for(auto dev_ : devices){
                if(!dev_->get_valid()){

                    dev_->is_alive();
                    if(!dev_->get_valid()) {
                        line->reconnect();
                    }
                    continue;
                }
                if(devices.size() > 1){
                        std::this_thread::sleep_for(0.02s);         // меньше - ошибки
                }

                mb_threads.push_back(std::thread(work<SonarDev>, dev_, lable));
            }
        }
        // UART loop
        //
        std::vector<std::thread> u_threads;

        for(auto line: u_lines){
            if(!line->get_valid()){

                line->uart_reconnect();
                continue;
            }
            std::this_thread::sleep_for(0.02s); // меньше - ошибки
            std::string lable = line->get_lable();

            u_threads.push_back(std::thread(work<UartCtx>, line, lable));
        }

        for(auto &mb_thread : mb_threads){
            mb_thread.join();
        }

        for(auto &u_thread : u_threads){
            u_thread.join();
        }

        debprint.reinit();
    }

    // modbus_free(ctx);
    out.close();

    return 0;
}