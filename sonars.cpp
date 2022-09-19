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

class UartCtx{
public:
    UartCtx(){}
    UartCtx(std::string port_, char parity_, int baud_, int data_bit_, int stop_bit_):
        s_port(port_), parity(parity_), baud(baud_), data_bit(data_bit_), stop_bit(stop_bit_){

            uart_connect();
            err_counter=0;
        }
        ~UartCtx(){ close(serial_port);}

        void uart_connect(){
            serial_port = open(s_port.c_str(), O_RDWR);
            valid = (tcgetattr(serial_port, &tty) != 0);
            switch (parity)
            {
            case 'N':
            case 'n':
                tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
                break;
            case 'O':
            case 'o':
                tty.c_cflag |= PARENB;
                tty.c_cflag |= PARODD; // odd
                break;
            case 'E':
            case 'e':
                tty.c_cflag |= PARENB;
                tty.c_cflag &= ~PARODD; // even
                break;
            default:
                tty.c_cflag &= ~PARENB;
                break;
            }

            switch (stop_bit)
            {
            case 1:
                tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
                break;
            default:
                tty.c_cflag |= CSTOPB;
                break;
            }

            tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size

            switch (data_bit)
            {
            case 8:
                tty.c_cflag |= CS8; // 8 bits per byte (most common)
                break;
            case 7:
                tty.c_cflag |= CS7; // 7 bits per byte
                break;
            case 6:
                tty.c_cflag |= CS6; // 6 bits per byte
                break;
            case 5:
                tty.c_cflag |= CS5; // 5 bits per byte
                break;

            default:
                tty.c_cflag |= CS8; // 8 bits per byte (most common)
                break;
            }

            tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
            tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

            tty.c_lflag &= ~ICANON;
            tty.c_lflag &= ~ECHO; // Disable echo
            tty.c_lflag &= ~ECHOE; // Disable erasure
            tty.c_lflag &= ~ECHONL; // Disable new-line echo
            tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
            tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
            tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

            tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
            tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
            // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
            // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

            tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
            tty.c_cc[VMIN] = 0;

            // Set in/out baud rate to be 9600
            // cfsetispeed(&tty, B9600);
            // cfsetospeed(&tty, B9600);
            tty.c_ispeed = baud;
            tty.c_ospeed = baud;

            // Save tty settings, also checking for error
            if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
                valid = false;
                debprint("Error %i from tcsetattr: %s\n", errno, strerror(errno));
            }
        }

        bool measure(){
            bool res = false;
            memset(&read_buf, '\0', sizeof(read_buf));
            num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
            for(int i = 3; i < num_bytes; ++i){
                uint8_t check_sum = read_buf[i];
                uint8_t head = read_buf[i-3];
                uint8_t byte_h = read_buf[i-2];
                uint8_t byte_l = read_buf[i-1];

                if(check_sum != ((head+byte_h+byte_l)&0xFF)){
                    ++err_counter;
                    continue;
                }
                res = true;
                dist = (byte_h << 8) | byte_l;
            }
            return res;
        }

        bool get_valid(){
            return valid;
        }

        uint get_distance(){
            return dist;
        }

        std::string get_port(){
            return s_port;
        }

        int get_error_counter(){
            return err_counter;
        }

private:
    bool valid;
    int serial_port;
    std::string s_port;
    char parity;
    uint8_t read_buf [256];
    int baud;
    int data_bit;
    int stop_bit;
    struct termios tty;
    int num_bytes;
    uint dist;
    int err_counter;

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

    std::vector<ModbusCtx> mb_lines;
    std::vector<UartCtx> u_lines;

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
        ModbusCtx mb{line_.port, line_.baud, 'N', 8, 1, time_out, line_.boot_time};
        for(Device device_ : line_.devices){
            SonarDev dev{device_.id, device_.test_reg, device_.measure_reg};
            mb.add_device(dev);
        }
        mb_lines.push_back(mb);
    }

    for(ULine line_ : ul.lines){
        UartCtx uart{line_.port, line_.parity_check, line_.baud, line_.data_bit, line_.stop_bit};
        u_lines.push_back(uart);
    }


/*
    std::string contents = get_file_contents(config_name.c_str());
    ryml::Tree tree = ryml::parse_in_place(ryml::to_substr(contents));

    std::string out_file;
    auto out_file_ = tree["File"];
    out_file_ >> out_file;
    out.open(out_file);


    ryml::NodeRef modbus_lines = tree["ModBus"];

    for (ryml::NodeRef const& child : modbus_lines.children()) {
        std::cout << "key: " << child.key() << std::endl;
    }

    for (ryml::NodeRef child : modbus_lines.children()) {
        // ryml::NodeRef devices  = child["devices"];
        auto port = child["port"];

        std::string port_str;
        port >> port_str;

        int i_baud;
        child["baud"] >> i_baud;

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

*/

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

    begin = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point w_begin;
    w_begin = std::chrono::steady_clock::now();
    double w_time = 0.;

    while (w_time <= 20.) {
        w_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - w_begin).count();

        for(auto line:mb_lines ){
            if(!line.get_valid()) {
                continue;
            }

            for(auto dev:line.get_devices()){

                std::this_thread::sleep_for(0.02s); // меньше - ошибки


                std::string lable ="line:  " + line.get_port() + "   sonar:  "+ std::to_string(dev.get_id());
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
            if(!line.get_valid()) {
                continue;
            }

            std::this_thread::sleep_for(0.02s); // меньше - ошибки

            std::string lable ="uart line:  " + line.get_port();
            if(line.measure()){
                double delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
                double work_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                // if(out.is_open()) out << 1 << "," <<  line.get_distance() << "," << delta_time << "," << work_time << std::endl;
                begin = std::chrono::steady_clock::now();

                debug_counter(lable + "   distance=  ", line.get_distance());
            } else {
                debug_counter(lable + "   error #  ", line.get_error_counter());
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
