#if !defined( SONAR_H )
#define SONAR_H

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

#include "debout.h"

class SonarDev{
public:
    SonarDev(){}
    SonarDev(int id_, int test_reg_, int measure_reg_):id(id_), test_reg(test_reg_), measure_reg(measure_reg_), error_counter(0){
        // valid = is_alive();
    }

    bool is_alive(){
        bool res = false;
        int rc;
        uint16_t tab_reg[2];
        modbus_flush(ctx);
        modbus_set_slave(ctx, id);
        rc = modbus_read_registers(ctx, test_reg, 1, tab_reg);
        res = (rc != -1);
        return res;
    }

    bool measure(){
        bool res = false;
        distance = -1;
        int rc;
        uint16_t tab_reg[2];
        modbus_set_slave(ctx, id);
        rc = modbus_read_registers(ctx, measure_reg, 1, tab_reg);

        if(rc == 1){
            res = true;
            error_counter = 0;
            distance = tab_reg[0];
        }else{
             ++error_counter;
        }
        return res;
    }

    int get_id(){
        return id;
    }

    int get_error_counter(){
        return error_counter;
    }

    uint16_t get_distance(){
        return distance;
    }

    void put_ctx(modbus_t *ctx_){
        ctx = ctx_;
    }

private:
    modbus_t *ctx;
    int id;
    int test_reg;
    int measure_reg;
    int error_counter;
    int16_t distance;
    bool valid;
};


class ModbusCtx{
public:

    ModbusCtx(){};
    ModbusCtx(std::string s_port_, int baud_, char parity_, int data_bit_, int stop_bit_, uint32_t time_out_, uint32_t boot_time_):
        s_port(s_port_), parity(parity_), baud(baud_), data_bit(data_bit_), stop_bit(stop_bit_), time_out(time_out_){

        port = s_port.c_str();
        boot_time = std::chrono::milliseconds(boot_time_);
        valid = (Modbus_connect() == 1);
    }

    // ~ModbusCtx(){
    //     modbus_free(ctx);
    // }

    int Modbus_connect(){
        ctx = modbus_new_rtu(port, baud, parity, data_bit, stop_bit);
        if (ctx == NULL) {
            debug_counter("sonar error:", "Unable to create the libmodbus context");
            valid = false;
            return -1;
        }

        if (modbus_connect(ctx) == -1) {
            debug_counter("sonar error: Connection failed:", stderr,  modbus_strerror(errno));
            modbus_free(ctx);
            valid = false;
            return -1;
        }

        std::this_thread::sleep_for(boot_time);
        modbus_set_response_timeout(ctx, 0, time_out);
        modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
        return 1;
    }

    bool get_valid(){
        return valid;
    }

    void add_device(SonarDev dev){
        dev.put_ctx(ctx);
        devices.push_back(dev);
    }

    std::vector<SonarDev> get_devices(){
        return devices;
    }

    std::string get_port(){

        return s_port;
    }

private:
    modbus_t *ctx;
    std::vector<SonarDev> devices;
    std::chrono::milliseconds boot_time = 0ms;
    bool valid;
    const char *port;
    std::string s_port;
    char parity;
    int baud;
    int data_bit;
    int stop_bit;
    uint32_t time_out;
};



#endif