#include "espconn.h"
#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
struct espconn espConn;
esp_tcp espTcp;

int data;

// hàm trả về  packet cho nguời dùng
void ICACHE_FLASH_ATTR user_http_response(struct espconn *conn,int error, char *html)
{
    char *buffer = NULL; //tạo biến để  lưu trữ dữ liệu trong IRam vì html có kích thước lớn
    int html_length = 0;
    buffer = (char *)os_malloc(256 *sizeof(char)); //tạo vùng nhờ để  lưu dữ liệu
    if(buffer !=NULL)
    {
        if(html !=NULL)
        {
            html_length = os_strlen(html);
        }
        else
        {
            html_length = 0;
        }
        os_sprintf(buffer, "HTTP/1.1 %d OK\r\n"
                            "Content-Length: %d\r\n"
                            "Content-Type: text/html\r\n"
                            "Connection: Closed\r\n"
                            "\r\n"
        ,error, html_length); //Tạo header cho packet với error code đưa vào
        if(html_length>0)
        {
            buffer = (char *)os_realloc(buffer,(256 + strlen(html))*sizeof(char));
            os_strcat(buffer,html); //nối html vào buffer
        }
        espconn_send(conn,buffer,strlen(buffer)); //gửi packet
        os_free(buffer); // gỉai phòng vùng nhớ
    }
}
int data=0;

void user_tcp_recv_cb(void *arg, char *pdata, unsigned short length)//arg: là espconn khi kết nối, pdata: dữ liệu đầu vào,length: độ dài dữ liệu đầu vào
{
    char *ptr = 0;
    struct espconn *pespconn = arg;
    ptr = (char *)os_strstr(pdata, "\r\n");
    ptr[0] = '\0';
    
    int temp = data/1000;
    int hum = data%1000;
    char *hello_html =(char *)os_malloc(256 *sizeof(char));;
    char *temperature=(char *)os_malloc(256 *sizeof(char));;
    char *humidity=(char *)os_malloc(256 *sizeof(char));;

    //so sánh các request đưa vào và response về
    if (os_strcmp(pdata, "GET / HTTP/1.1") == 0)
    {
        hello_html = "<h1>Temperature: ";
        os_sprintf(temperature, "%d",temp);
        os_sprintf(humidity, "%d",hum);
        os_strcat(hello_html,temperature);
        os_strcat(hello_html,"*C</h1><br><h1>Humidity: ");
        os_strcat(hello_html,humidity);
        os_strcat(hello_html,"%</h1><br><h1 style='color: red;'>Author: Phong-Duy</h1>");
        user_http_response(pespconn,200,hello_html);
    }
        os_free(temperature);
        os_free(humidity);
        os_free(hello_html);      
}

void ICACHE_FLASH_ATTR user_tcp_disconnect(void *arg) //cb khi người dùng ngắt kết nối
{
   
    os_printf("Client disconected\r\n");
}

void ICACHE_FLASH_ATTR user_tcp_listen(void *arg) //cb để  bắt đầu kết nôi tcp
{
    struct espconn *pesp_conn = arg;
   
    espconn_regist_recvcb(pesp_conn, user_tcp_recv_cb); // đăng kí recive cb
    espconn_regist_disconcb(pesp_conn, user_tcp_disconnect); // đăng kí disconnect cb
    os_printf("\r\nTCP server is listening\r\n");
}

// hàm cài đặt cho tcp server
void ICACHE_FLASH_ATTR user_tcp_init(uint32 port,int tempHum)
{
    data = tempHum;
    espConn.type = ESPCONN_TCP; // kiểu kết nối tcp
    espConn.state = ESPCONN_NONE; // trạng thái bắt đầu là chưa kết nối
    espConn.proto.tcp = &espTcp; // giao thức đưa vào biến kiểu esp_tcp
    espConn.proto.tcp->local_port = port; // port

    espconn_regist_connectcb(&espConn,user_tcp_listen); // đăng kí bắt đầu kết nối gọi cb listen
    espconn_accept(&espConn); // tạo kết nối
    os_printf("\r\nServer is online");
}


