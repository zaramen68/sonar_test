#if !defined( MBSONAR_H )
#define MBSONAR_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <iostream>
#include <mutex>
#include <string>
#include <modbus.h>
#include <assert.h>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>
#include <chrono>

#include "debout.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "yaml_str.hpp"
#include "common_f.hpp"
#pragma GCC diagnostic pop

using namespace std::literals::chrono_literals;


class SonarDev{
public:
    SonarDev(){}
    SonarDev(const int id_, const int sonar_name_, const int test_reg_, const int measure_reg_, const Convertion_data &cd_, const bool d_flag_):
            ctx(nullptr),
            id(id_),
            sonar_name(sonar_name_),
            test_reg(test_reg_),
            measure_reg(measure_reg_),
            error_counter(0),
            dump_flag(d_flag_),
            c_data(cd_)
            {
        valid = false;
    }


    void is_alive(){
        if(ctx == NULL) return;

        uint16_t tab_reg[1];
        modbus_flush(ctx);
        modbus_set_slave(ctx, id);
        const int rc = modbus_read_registers(ctx, test_reg, 1, tab_reg);
        debug_counter("is alive Modbus id " + std::to_string(id) + " sonar is: ", rc);
        valid = (rc != -1);
        if(valid) {error_counter = 0;}
    }

    bool measure(){
        if(ctx == nullptr) {
            ++error_counter;
            debug_counter("Modbus measure: ctx is NULL", id, error_counter);
            return false;
        }
        distance = -1;
        if(error_counter > 10){
            debug_counter("ModBus measure: id: " + std::to_string(id) + " is invalid", error_counter);
            valid = false;
            return false;
        }

        uint16_t tab_reg[1];
        modbus_set_slave(ctx, id);
        const int rc = modbus_read_registers(ctx, measure_reg, 1, tab_reg);
        if(rc != 1){
           ++error_counter;
           debug_counter("Modbus measure  id: " + std::to_string(id) + " error rc!=1 number is: ", error_counter);
           return false;
        }

        distance = tab_reg[0];
        if(distance == 0) { // полагаем, что 0 - пустое пространство
            obstacle.clear();
        }
        else{
            obstacle = make_obstacle(distance, c_data);
        }

        error_counter = 0;

        // if(dump_flag) {dump();}
        return true;
    }

    void dump(){
        if(debug_out.is_open()){
                const auto now = std::chrono::steady_clock::now();
                const auto timestamp = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
                debug_out << id << "," <<  distance << "," << 0 << "," << timestamp << std::endl;
        }
    }

    int get_id() const {
        return id;
    }

    int get_name() const {
        return sonar_name;
    }

    bool get_valid() const {
        return valid;
    }

    int get_error_counter() const {
        return error_counter;
    }

    uint16_t get_distance() const {
        return distance;
    }

    void put_ctx(modbus_t * const ctx_, const std::string &port_name){
        ctx = ctx_;
        out_file = log_path + port_name.substr(sonar_path_delim.size()) + "_id_" + std::to_string(id) + ".data";
        if ( dump_flag ){
            debug_out.open(out_file);
        }
    }

    std::vector<Vertex> get_obstacle() const {
        return obstacle;
    }


    void get_debug(uint64_t start_time,  uint64_t stop_time) {
        if(debug_out.is_open()){

                debug_out  <<  start_time <<  "," << stop_time << std::endl;
        }
    }

private:
    modbus_t *ctx = nullptr;
    int id;
    int sonar_name;
    int test_reg;
    int measure_reg;
    int error_counter;
    uint16_t distance;
    bool dump_flag;
    bool valid;
    double m_convert[3][3];
    double r_convert[3];
    std::vector<Vertex> obstacle;
    Convertion_data c_data;
    std::ofstream debug_out;
    std::string out_file;

};


class ModbusCtx{
public:

    ModbusCtx(){};

    ModbusCtx(const std::string& s_port_, const int baud_, const char parity_, const int data_bit_, const int stop_bit_, const uint32_t time_out_, const uint32_t boot_time_)
    : s_port(s_port_), parity(parity_), baud(baud_), data_bit(data_bit_), stop_bit(stop_bit_), time_out(time_out_)
    {
        port = s_port.c_str();
        lable = "modbus line:  " + s_port;
        boot_time = std::chrono::milliseconds(boot_time_);
        Modbus_connect();
    }

    ~ModbusCtx(){
        modbus_free(ctx);
    }

    bool get_valid() const {
        return (ctx != nullptr);
    }

    bool reconnect() {
        // троттлинг попыток подключения (для снижения нагрузки на ноду)
        if ( recon_counter == 0 ) {
            Modbus_connect();
            if (!get_valid())
                { recon_counter = 1; }
        } else {
            if (++recon_counter > 10)
                { recon_counter = 0; }
            return false;
        }
        for(auto& dev : devices){
            dev->put_ctx(ctx, s_port);
            dev->is_alive();
        }
        return true;
    }

    void add_device(std::shared_ptr< SonarDev >dev){
        //Изначальное добавление устройства в линию модбас
        // + инициализация
        //
        dev->put_ctx(ctx, s_port);
        dev->is_alive();
        devices.push_back(dev);
    }

    const std::vector<std::shared_ptr< SonarDev >> & get_devices() const {
        return devices;
    }

    const std::string& get_port() const {
        return s_port;
    }

    const std::string& get_lable() const {
        return lable;
    }

private:
    modbus_t *ctx = nullptr;
    std::vector<std::shared_ptr< SonarDev >> devices;
    std::chrono::milliseconds boot_time = 0ms;
    int recon_counter = 0;
    const char *port;
    std::string s_port;
    std::string lable;
    char parity;
    int baud;
    int data_bit;
    int stop_bit;
    uint32_t time_out;

    void Modbus_connect(){
        if (ctx != nullptr) {
            modbus_close(ctx);
            modbus_free(ctx);
            ctx = nullptr;
        }
        ctx = modbus_new_rtu(port, baud, parity, data_bit, stop_bit);
        if (ctx == nullptr) {
            debug_counter("Modbus error:  " + s_port, "Unable to create the libmodbus context ctx = NULL");
            return;
        }

        if (modbus_connect(ctx) == -1) {
            debug_counter("Modbus error: Connection failed:  " + s_port, stderr,  modbus_strerror(errno));
            modbus_free(ctx);
            ctx = nullptr;
            return;
        }

        std::this_thread::sleep_for(boot_time);
        modbus_set_response_timeout(ctx, 0, time_out);
        modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    }
};

#endif