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

#include <ryml_std.hpp>
#include <ryml.hpp>

#include "debout.h"
#include "Task.h"


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
            fprintf(stderr, "sonar error: Connection failed: %s \n",  modbus_strerror(errno));
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


int main(int argc, char *argv[])
{
    int slave_id = 1;
    std::vector<int> id{1,2,3};
    std::vector<ModbusCtx> lines;
    uint32_t time_out = 2'000'000;

    if(argc > 1){
        std::stringstream convert1(argv[1]);
        convert1 >> slave_id;
    }
    using namespace std::chrono_literals;

    std::string contents = get_file_contents("/home/tenderbook/sonars/config.yaml");
    ryml::Tree tree = ryml::parse_in_place(ryml::to_substr(contents));

    ryml::NodeRef modbus_lines = tree["ModBus"];

    for (ryml::NodeRef const& child : modbus_lines.children()) {
        std::cout << "key: " << child.key() << std::endl;
    }

    for (ryml::NodeRef child : modbus_lines.children()) {
        // ryml::NodeRef devices  = child["devices"];
        auto port = child["port"];

        std::string port_str;
        port >> port_str;

        auto baud  = child["baud"];
        int i_baud;
        baud >> i_baud;

        auto boot_time  = child["boot_time"];
        uint32_t i_bt;
        boot_time >> i_bt;
        // const char* port_n = port_str.c_str();

        ModbusCtx line{port_str, i_baud, 'N', 8, 1, time_out, i_bt};

        std::cout << "key: " << port.key() << " val: " << port_str << std::endl;

        auto devices = child["devices"];

        for(ryml::NodeRef device: devices.children()){

            int id_i, mreg, treg;
            auto id_ = device["id"];
            id_ >> id_i;

            auto measure_reg = device["measure_reg"];
            measure_reg >> mreg;

            auto test_reg = device["test_reg"];
            test_reg >> treg;

            SonarDev dev{id_i, treg, mreg};
            line.add_device(dev);

            std::cout << "key: " << device.key() << "  id:  " << id_i << "  mreg  " << mreg << "treg" << treg <<std::endl;


        }

        lines.push_back(line);

    }

    // modbusCtl.init(1, std::string("/dev/ttyUSB0").c_str(), 9600, 50000, 1100, "Sonar"); // 950 - doesn't work, 1000 - work, using 1100 ms
    // modbus_t *ctx;

    // ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);
    // if (ctx == NULL) {
    //     std::cout << "Unable to create the libmodbus context" << std::endl;
    //     return -1;
    // }



    // volatile _modbus_get_data *data = reinterpret_cast<_modbus_get_data *>(ctx);
    // if (modbus_connect(ctx) == -1) {
    //     std::cout << "Connection failed:"  <<  modbus_strerror(errno) << std::endl;
    //     modbus_free(ctx);
    //     return -1;
    // }

    // assert(data->slave != -1);

    // uint8_t tab_bytes[MODBUS_MAX_PDU_LENGTH];


    // int rc_s = modbus_report_slave_id(ctx, MODBUS_MAX_PDU_LENGTH, tab_bytes);
    // if (rc_s > 1) {
    //     printf("Run Status Indicator: %s\n", tab_bytes[1] ? "ON" : "OFF");
    // }

    Task request{500};

    while (true) {
        // request([ctx, id]{

        for(auto line:lines ){
            if(!line.get_valid()) {
                continue;
            }
            // modbus_flush(ctx);
            // modbus_set_slave(ctx, id_);
            // modbus_set_response_timeout(ctx, 0, 500'000);
            // uint16_t tab_reg[2];
            // int rc;

            // rc = modbus_read_registers(ctx, 257, 1, tab_reg);
            // // if(!r) {return;}
            // std::string lable = "sonar id " + std::to_string(id_);
            // if(rc==1){
            //     debug_counter(lable, tab_reg[0]);
            // } else {
            //     debug_counter(lable + "no data", id_);

            for(auto dev:line.get_devices()){

                std::this_thread::sleep_for(0.1s);

                std::string lable ="line:  " + line.get_port() + "   sonar:  "+ std::to_string(dev.get_id());
                if(dev.measure()){
                    debug_counter(lable + "   distance=  ", dev.get_distance());
                } else {
                    debug_counter(lable + "   error #  ", dev.get_error_counter());
                }

            }

        }
            // std::cout << "id: "<< id_<< ",  " << "rc: " << rc << '\n';
            // for (int i=0; i < rc; i++) {
            //     printf("reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
            // }

            // std::this_thread::sleep_for(0.1s);
            debprint.reinit();
    }

            // std::this_thread::sleep_for(0.2s);
        // });
    // modbus_free(ctx);

    return 0;
}
