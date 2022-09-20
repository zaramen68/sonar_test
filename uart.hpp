#if !defined( UART_H )
#define UART_H

// C library headers
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

class UartCtx{
public:
    UartCtx(){}
    UartCtx(std::string port_, char parity_, int baud_, int data_bit_, int stop_bit_):
        s_port(port_), parity(parity_), baud(baud_), data_bit(data_bit_), stop_bit(stop_bit_){
            uart_connect();
            err_counter=0;
            i_msg[0] = 1;
        }

    void uart_connect(){
        serial_port = open(s_port.c_str(), O_RDWR);
        valid = (tcgetattr(serial_port, &tty) == 0);

        if(!valid) {
            debug_counter("Error ", strerror(errno), errno);
        }

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

        // tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
        // tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
        // tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
        // tty.c_cflag |= CS8; // 8 bits per byte (most common)

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

        tty.c_cc[VTIME] = 20;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        tty.c_cc[VMIN] = 0;

        // Set in/out baud rate to be 9600
        cfsetispeed(&tty, B9600);
        cfsetospeed(&tty, B9600);
        // cfsetispeed(&tty, baud);
        // cfsetospeed(&tty, baud);

        // Save tty settings, also checking for error
        if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
            valid = false;
            debug_counter("Error from tcsetattr: ", errno, strerror(errno));
        }
    }

    bool measure(){
        bool res = false;
        write(serial_port, i_msg, sizeof(i_msg));
        memset(&read_buf, '\0', sizeof(read_buf));
        num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
        if (num_bytes < 0) {
            debug_counter("Error reading:   ", strerror(errno));
        }
        if(num_bytes <= 0) return res;

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
    uint8_t i_msg [1];
    int baud;
    int data_bit;
    int stop_bit;
    struct termios tty;
    int num_bytes;
    uint dist;
    int err_counter;

};

#endif
