#include "udp.h"

static const char *TAG =        "udp";

#define MULTICAST_IPV4_ADDR     "175.155.58.189"
#define UDP_PORT                3333
#define MULTICAST_TTL 1

ip_info_t ip_info = {};

static int socket_add_ipv4_multicast_group(int sock, bool assign_source_if)
{
    int err = 0;
    struct ip_mreq imreq = { 0 };
    struct in_addr iaddr = { 0 };
    
    /* 获取本地ip地址 */ 
    esp_netif_ip_info_t ip_info = { 0 };
    esp_netif_t *sta_handle = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    err = esp_netif_get_ip_info(sta_handle, &ip_info);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get IP address info. Error 0x%x", err);
        return err;
    }
    inet_addr_from_ip4addr(&iaddr, &ip_info.ip);

    /* ip地址类型转换 */ 
    err = inet_aton(MULTICAST_IPV4_ADDR, &imreq.imr_multiaddr.s_addr);
    if (err != 1) {
        ESP_LOGE(TAG, "Configured IPV4 multicast address '%s' is invalid.", MULTICAST_IPV4_ADDR);
        return -1;
    }
    ESP_LOGI(TAG, "Configured IPV4 Multicast address %s", inet_ntoa(imreq.imr_multiaddr.s_addr));
    if (!IP_MULTICAST(ntohl(imreq.imr_multiaddr.s_addr))) {
        ESP_LOGW(TAG, "IPV4 multicast address '%s' is not a valid multicast address.", MULTICAST_IPV4_ADDR);
    }

    /* 是否为ipv4 */ 
    if (assign_source_if) {
        /* 设置套接字选项，指定多播组使用的本地接口 */ 
        err = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,
                         sizeof(struct in_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Failed to set IP_MULTICAST_IF. Error %d", errno);
            return err;
        }
    }

    /* 加入指定多播组 */ 
    err = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                         &imreq, sizeof(struct ip_mreq));
    if (err < 0) {
        ESP_LOGE(TAG, "Failed to set IP_ADD_MEMBERSHIP. Error %d", errno);
        return err;
    }

    return 0;
}

static int create_multicast_ipv4_socket(void)
{
    struct sockaddr_in saddr = { 0 };
    int sock = -1;
    int err = 0;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Failed to create socket. Error %d", errno);
        return -1;
    }

    // Bind the socket to any address
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(UDP_PORT);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (err < 0) {
        ESP_LOGE(TAG, "Failed to bind socket. Error %d", errno);
        goto err;
    }

    // Assign multicast TTL (set separately from normal interface TTL)
    uint8_t ttl = MULTICAST_TTL;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(uint8_t));
    if (err < 0) {
        ESP_LOGE(TAG, "Failed to set IP_MULTICAST_TTL. Error %d", errno);
        goto err;
    }

    // this is also a listening socket, so add it to the multicast
    // group for listening...
    err = socket_add_ipv4_multicast_group(sock, true);
    if (err < 0) {
        goto err;
    }

    // All set, socket is configured for sending and receiving
    return sock;

err:
    close(sock);
    return -1;
}

void udp_task(void *pvParameters) {
    int sock;
    while (1) {
        if(wifi_sta_isconnect() == 0){
            ESP_LOGI(TAG, "wait wifi connect");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        // //读取不到ip
        // if(nvs_read_ip_info(UDP_MULTICAST_KEY, &ip_info) == -1){
        //     ESP_LOGI(TAG, "read ip_info fail");
        //     vTaskDelay(5000 / portTICK_PERIOD_MS);
        //     continue;
        // }

        sock = create_multicast_ipv4_socket();
        if (sock < 0) {
            ESP_LOGE(TAG, "Failed to create IPv4 multicast socket");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }

        // set destination multicast addresses for sending from these sockets
        struct sockaddr_in sdestv4 = {
            .sin_family = PF_INET,
            .sin_port = htons(UDP_PORT),
        };
        // We know this inet_aton will pass because we did it above already
        inet_aton(MULTICAST_IPV4_ADDR, &sdestv4.sin_addr.s_addr);

        int err ;
        fd_set readfds;
        struct timeval tv = {
            .tv_sec = 2,
            .tv_usec = 0,
        };
        while (1) {
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);

            int s = select(sock + 1, &readfds, NULL, NULL, &tv);
            if (s < 0) {
                ESP_LOGE(TAG, "Select failed: errno %d", errno);
                break;
            }

            if (s > 0) {
                if (FD_ISSET(sock, &readfds)) {
                    // Incoming datagram received
                    char recvbuf[48];
                    char raddr_name[32] = { 0 };

                    struct sockaddr_storage raddr; // Large enough for both IPv4 or IPv6
                    socklen_t socklen = sizeof(raddr);
                    int len = recvfrom(sock, recvbuf, sizeof(recvbuf)-1, 0,
                                       (struct sockaddr *)&raddr, &socklen);
                    if (len < 0) {
                        ESP_LOGE(TAG, "multicast recvfrom failed: errno %d", errno);
                        break;
                    }

                    if (raddr.ss_family == PF_INET) {
                        inet_ntoa_r(((struct sockaddr_in *)&raddr)->sin_addr,
                                    raddr_name, sizeof(raddr_name)-1);
                    }
                    ESP_LOGI(TAG, "received %d bytes from %s:", len, raddr_name);

                    recvbuf[len] = 0; // Null-terminate whatever we received and treat like a string...
                    ESP_LOGI(TAG, "%s", recvbuf);
                }
            }else {
                ESP_LOGI(TAG, "Send to address %s:%d...",  MULTICAST_IPV4_ADDR, UDP_PORT);


                struct addrinfo hints = {
                    .ai_flags = AI_PASSIVE,
                    .ai_socktype = SOCK_DGRAM,
                };
                struct addrinfo *res;

                // Send an IPv4 multicast packet

                hints.ai_family = AF_INET; // For an IPv4 socket

                int err = getaddrinfo(MULTICAST_IPV4_ADDR,
                                      NULL,
                                      &hints,
                                      &res);
                if (err < 0) {
                    ESP_LOGE(TAG, "getaddrinfo() failed for IPV4 destination address. error: %d", err);
                    break;
                }
                if (res == 0) {
                    ESP_LOGE(TAG, "getaddrinfo() did not return any addresses");
                    break;
                }
                
                ((struct sockaddr_in *)res->ai_addr)->sin_port = htons(UDP_PORT);
                char addrbuf[32] = { 0 };
                inet_ntoa_r(((struct sockaddr_in *)res->ai_addr)->sin_addr, addrbuf, sizeof(addrbuf)-1);
                ESP_LOGI(TAG, "Sending to IPV4 multicast address %s:%d...",  addrbuf, UDP_PORT);

                char sendbuf[] = "multicast sent by ESP32\n";
                err = sendto(sock, sendbuf, sizeof(sendbuf), 0, res->ai_addr, res->ai_addrlen);
                
                if (err < 0) {
                    ESP_LOGE(TAG, "IPV4 sendto failed. errno: %d", errno);
                    break;
                }
            }
        }

        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
        vTaskDelay(5000 / portTICK_PERIOD_MS); // 每5秒发送一次消息
    }
}

void udp_client_task(void *pvParameters) {
    struct sockaddr_in server_addr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("116.169.4.84");
    server_addr.sin_port = htons(3333);

    while (1) {
        if(wifi_sta_isconnect() == 0){
            ESP_LOGI(TAG, "wait wifi connect");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }
        const char *message = "Hello, UDP Server!";
        char addrbuf[32] = { 0 };
        inet_ntoa_r(server_addr.sin_addr, addrbuf, sizeof(addrbuf)-1);
        ESP_LOGI(TAG, "Sending to address %s:%d...",  addrbuf, UDP_PORT);
        int err = sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);  // 每隔2秒发送一次消息
    }

    close(sock);
    vTaskDelete(NULL);
}