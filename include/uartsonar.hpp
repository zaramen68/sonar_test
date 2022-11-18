#if !defined( UARTSONAR_H )
#define UARTSONAR_H

// C library headers
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function

#include <sys/ioctl.h>
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <linux/serial.h>

#include <unistd.h> // write(), read(), close()

#include "debout.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "yaml_str.hpp"
#include "common_f.hpp"
#pragma GCC diagnostic pop


class UartCtx{
public:
    UartCtx(){}

    UartCtx(const std::string &port_, const int sonar_name_, const char parity_, const int baud_, const int data_bit_, const int stop_bit_, const Convertion_data &cd_, const bool d_flag_):
        s_port(port_),
        sonar_name(sonar_name_),
        parity(parity_),
        baud(baud_),
        data_bit(data_bit_),
        stop_bit(stop_bit_),
        c_data(cd_),
        dump_flag(d_flag_)

        {
            is_answer = false;
            valid = (uart_connect() != -1);
            err_counter=0;
            lable = "uart line:  " + s_port;
            out_file = log_path + s_port.substr(sonar_path_delim.size()) + "_u_a_r_t.data";

            if ( dump_flag ){
                debug_out.open(out_file);
            }
        }

    UartCtx(const std::string &port_, const int sonar_name_, const Convertion_data &cd_,  const bool d_flag_):
        s_port(port_),
        sonar_name(sonar_name_),
        parity('N'),
        baud(9600),
        data_bit(8),
        stop_bit(1),
        c_data(cd_),
        dump_flag(d_flag_)

        {
            is_answer = false;
            valid = (uart_connect() != -1);
            err_counter=0;
            lable = "uart line:  " + s_port;
            out_file = log_path + s_port.substr(sonar_path_delim.size()) + "_u_a_r_t.data";

            if ( dump_flag ){
                debug_out.open(out_file);
            }
        }

    ~UartCtx(){
        if(serial_port_fd != -1) {close(serial_port_fd);}
    }

    int uart_connect(){
        serial_port_fd = open(s_port.c_str(), O_RDWR);
        // int get_res = ioctl(serial_port_fd, TCGETS2, &tty);

        if(auto get_res = ioctl(serial_port_fd, TCGETS2, &tty); get_res == -1){
            switch (errno){
                case EBADF: debug_counter("uart connect error " + s_port, "Некорректный файловый дескриптор" ); break;
                case EFAULT: debug_counter("uart connect error " + s_port, "Некорректный указатель на аргумент"); break;
                case EINVAL: debug_counter("uart connect error " + s_port,"Некорректные параметры в аргументе для TCGETS2"); break;
                //case ENOIOCTLCMD: process_error("команды TCGETS2 не существует", f); break;
                case ENOTTY: debug_counter("uart connect error " + s_port, "Файловый дескриптор не терминального устройства или команда TCGETS2 неприменима для данного устройства"); break;
                case EPERM: debug_counter("uart connect error " + s_port, "Нет разрешений на команду TCGETS2"); break;
                default: debug_counter("uart connect error " + s_port, std::string("Неизвестная ошибка, errno ") + std::to_string(errno)); break;
            }
            close(serial_port_fd);
            serial_port_fd = -1;
            return serial_port_fd;
        }

        debug_counter("get tty res Uart connect:  " + s_port, serial_port_fd);

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
        // tty.c_cflag |= CRTSCTS;
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

        // Для нас  время тайм-аута 200 мс.
        tty.c_cc[VTIME] = 2;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        tty.c_cc[VMIN] = 0;     // minimum bites to receive

        // Установка скорости обмена.
        tty.c_cflag &= ~(CBAUD | CBAUDEX);
        tty.c_cflag |= BOTHER;

        tty.c_ispeed = baud;
        tty.c_ospeed = baud;

        int set_res = ioctl(serial_port_fd, TCSETS2, &tty);
        debug_counter("set tty res from Uart connect  " + s_port, set_res);

        if (set_res != 0) {
            debug_counter("Error from Uart connect " + s_port + "setattr: ", errno, strerror(errno));
            close(serial_port_fd);
            serial_port_fd = -1;
        }
        return serial_port_fd;
    }

    void uart_reconnect(){
        debug_counter("Uart "+s_port+"  RECONNECT ", serial_port_fd);

        if (serial_port_fd != -1){close(serial_port_fd);}
        valid = (uart_connect() != -1);
        if(valid) {
            err_counter = 0;
        }
    }

    bool measure(){
        static constexpr uint8_t dist_request[1] = {0x01};
        static constexpr int64_t time_out = 200;
        is_answer = false;
        if(err_counter > 10){
            valid = false;
            return false;
        }

        // uint8_t read_buf [1];

        ioctl(serial_port_fd, TCFLSH, 2); // Очистка буферов чтения-записи


        [[maybe_unused]] auto const dummy_ = write(serial_port_fd, dist_request, sizeof(dist_request));
        // memset(&read_buf, '\0', sizeof(read_buf));

        const auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() < time_out)
        {
            uint8_t read_buf [1];
            memset(&read_buf, '\0', sizeof(read_buf));
            num_bytes = read(serial_port_fd, &read_buf, sizeof(read_buf));
            debug_counter("Uart "+s_port+" read num_bytes = ", num_bytes);

            if(num_bytes <= 0) {
                ++err_counter;
                debug_counter("measure Uart Error reading: " + s_port, strerror(errno), num_bytes);
                return false;
            }

            data_buffer.push_back(read_buf[0]);

            if(data_buffer.size() > 3 ){

                for(auto iter = data_buffer.rbegin(); iter < data_buffer.rend()-3; ++iter){
                    uint8_t check_sum = *(iter);
                    uint8_t head = *(iter+3);
                    uint8_t byte_h = *(iter+2);
                    uint8_t byte_l = *(iter+1);

                    if(check_sum != ((head+byte_h+byte_l)&0xFF)){
                        continue;
                    }

                    // data_buffer.erase(data_buffer.begin(), iter.base()); // Удаление хороших данных из буфера

                    err_counter = 0;
                    distance = (byte_h << 8) | byte_l;
                    make_obstacle(distance, c_data);
                    // if(dump_flag){ dump(); }
                    is_answer = true;
                    return true;
                }
            }
        }

        // num_bytes = read(serial_port_fd, &read_buf, sizeof(read_buf));

        // debug_counter("Uart "+s_port+" read num_bytes = ", num_bytes);

        // if(num_bytes <= 0) {
        //     ++err_counter;
        //     debug_counter("measure Uart Error reading: " + s_port, strerror(errno), num_bytes);
        //     return false;
        // }
        // Зачистка буфера
        //
        //data_buffer.clear();

        // Запись в буфер  долгосрочного хранения
        //

        // for (int i = 0; i<num_bytes; ++i){
        //     data_buffer.push_back(read_buf[i]);
        // }

        // if(data_buffer.size() > 3 ){

        //     for(auto iter = data_buffer.rbegin(); iter < data_buffer.rend()-3; ++iter){
        //         uint8_t check_sum = *(iter);
        //         uint8_t head = *(iter+3);
        //         uint8_t byte_h = *(iter+2);
        //         uint8_t byte_l = *(iter+1);

        //         if(check_sum != ((head+byte_h+byte_l)&0xFF)){
        //             continue;
        //         }

        //         data_buffer.erase(data_buffer.begin(), iter.base()); // Удаление хороших данных из буфера

        //         err_counter = 0;
        //         distance = (byte_h << 8) | byte_l;
        //         make_obstacle(distance, c_data);
        //         // if(dump_flag){ dump(); }
        //         is_answer = true;
        //         return true;
        //     }
        // }
        err_counter++;

        return false;
    }

    void dump(){
        if(debug_out.is_open()){
                const auto now = std::chrono::steady_clock::now();
                const auto timestamp = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
                debug_out << lable << "," <<  distance << "," << 0 << "," << timestamp << std::endl;
        }
    }

    bool get_valid() const {
        return valid;
    }

    bool get_answer() const {
        return is_answer;
    }

    int get_name() const {
        return sonar_name;
    }

    uint16_t get_distance() const {
        return distance;
    }

    const std::string get_port() const {
        return s_port;
    }

    const std::string& get_lable() const {
        return lable;
    }

    int get_error_counter() const {
        return err_counter;
    }

    std::vector<Vertex> get_obstacle(){
        return obstacle;
    }


    void get_debug(uint64_t start_time,  uint64_t stop_time, bool res) {
        if(debug_out.is_open()){

                debug_out  <<  start_time <<  "," << stop_time << "," << res << "," << valid << std::endl;
        }
    }

private:
    bool valid;
    int serial_port_fd;
    std::string s_port;
    std::string lable;
    int sonar_name;
    char parity;
    int baud;
    int data_bit;
    int stop_bit;
    struct termios2 tty;
    int num_bytes;
    uint16_t distance;
    int err_counter;
    bool is_answer;
    std::vector<Vertex> obstacle;
    Convertion_data c_data;
    std::ofstream debug_out;
    std::string out_file;
    bool dump_flag;
    std::vector<uint8_t> data_buffer;

};

#endif
