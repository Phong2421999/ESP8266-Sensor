#include "osapi.h"
#include "tcp_server.h"

os_timer_t timer;
#define PORT         1500
void ICACHE_FLASH_ATTR user_check_wifi_connect() // kiểm tra wifi kết nối hay chưa
{
    struct ip_info ipConfig;
    os_timer_disarm(&timer); // bỏ timer đếm thời gian
    wifi_get_ip_info(STATION_IF,&ipConfig); // lấy ip
    if(wifi_station_get_connect_status() == STATION_GOT_IP && ipConfig.ip.addr !=0)
    {
        os_printf("\r\nwifi connectd!");
        user_tcp_init(PORT); // mở server
    }
    else
    {
        os_timer_setfn(&timer,(os_timer_func_t *)user_check_wifi_connect,NULL); //gán việc chạy lại hàm kiểm tra kết nối sau khi đếm xong
        os_timer_arm(&timer,300,0); //tạo timer
    }
}

//config ssid và password để  kết nối wifi
void ICACHE_FLASH_ATTR user_config_wifi(char ssid[32],char password[64])
{
    struct station_config stationConfig;

    stationConfig.bssid_set = 0; // không cần xác nhận địa chỉ mac


    // chạy hàm kiểm tra
    os_timer_disarm(&timer);
    os_timer_setfn(&timer,(os_timer_func_t *)user_check_wifi_connect,NULL);
    os_timer_arm(&timer,300,0);
}