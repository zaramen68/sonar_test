#if !defined( COMMON_F_H )
#define COMMON_F_H

#include <vector>
#include <string>
#include <math.h>
#include <chrono>
#include "custom_literals.h"


const std::string log_path("/home/tenderbook/logs/sonar/");
const std::string config_name = "/home/tenderbook/settings/sonars.yaml";
const std::string sonar_path_delim("/dev/");


struct Vertex{
    float x, y, z;
    Vertex(){}
    Vertex(float x_, float y_, float z_):x(x_), y(y_), z(z_){}
    Vertex(float x_, float z_): x(x_), z(z_){y = 0.;}
};

struct Convertion_data{
    double m00;
    double m01;
    double m02;

    double m10;
    double m11;
    double m12;

    double m20;
    double m21;
    double m22;

    double x_off;
    double y_off;
    double z_off;
};

Vertex vertex_convert(const Vertex &v, const Convertion_data &c_data) {
    Vertex res;
    res.x = c_data.m00 * v.x + c_data.m01 * v.y + c_data.m02 * v.z + c_data.x_off;
    res.y = c_data.m10 * v.x + c_data.m11 * v.y + c_data.m12 * v.z + c_data.y_off;
    res.z = c_data.m20 * v.x + c_data.m21 * v.y + c_data.m22 * v.z + c_data.z_off;
    return res;
}

std::vector<Vertex> make_obstacle(const uint16_t distance, const Convertion_data &c_data){
    std::vector<Vertex> obstacle;
    if(distance==0.) {return obstacle;}

    constexpr uint16_t dist_r_limit = 150; // мм
    constexpr double sonar_view_angle = 40_deg; // Автоперевод в радианы
    constexpr double k = __builtin_tan(sonar_view_angle) / 1000.0 ; // Коэффициент преобразования угла в радиус
    constexpr double r_max = dist_r_limit * k; // мм

    constexpr double step=0.01;     // значение шага выбрано произвольно, что бы только набросать точек в отрезок, изображающий препятствие
                                    //  их может быть произвольное число и зраници отрезка выбраны произвольно.
    double r = distance < dist_r_limit ? distance*k : r_max;   // при расстоянии больше 0.15 м, радиус облака не увеличивается.
    for(double x = -r; x <= r; x+=step){
        double y1 = sqrt(r*r - x*x);
        double y2 = -y1;
        obstacle.push_back(vertex_convert(Vertex(x, y1, distance/1000.), c_data));
        obstacle.push_back(vertex_convert(Vertex(x, y2, distance/1000.), c_data));
    }
    return obstacle;
}

std::chrono::milliseconds current_time_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}


#endif