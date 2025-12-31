#include <uart.h>
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "App_Config.h"

// Server configuration (unchanged)
#define SERVER_IP       "192.168.60.130"
#define SERVER_PORT     9060
#define RECONNECT_DELAY 3000             // Reconnection interval (fixed 3s)
#define SEND_INTERVAL   2000             // Data send interval (2s)
#define RX_BUF_SIZE     1024             // Receive buffer size

// Global variables (unchanged)
static int sock_fd = -1;
static SemaphoreHandle_t conn_mutex;
static volatile uint8_t is_connected = 0;
struct timeval timeout = {0, 0};
/**
 * Establish TCP connection (with errno printing added)
 * @return 0-success, -1-failure
 */
static int tcp_client_connect(void) {
    struct sockaddr_in server_addr;
    int ret;

    // Close existing connection (logic unchanged)
    if (sock_fd != -1) {
        close(sock_fd);
        sock_fd = -1;
    }

    // Create TCP socket (logic unchanged)
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (sock_fd < 0) {
        Ifx_Console_print("Socket creation failed: errno=%d (1=Out of memory, 2=Protocol not supported)\r\n", errno);
        return -1;
    }

    // Configure server address (logic unchanged)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    // Connect to server (with errno printing added for failure cause)
    ret = connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret != 0) {
        // Print specific error: ECONNREFUSED=111 (Server rejected), ENETUNREACH=101 (Network unreachable), ETIMEDOUT=110 (Timeout)
        Ifx_Console_print("Connect failed: ret=%d, errno=%d\r\n", ret, errno);
        //closesocket(sock_fd);
        close(sock_fd);
        sock_fd = -1;
        return -1;
    }

    Ifx_Console_print("Successfully connected to %s:%d\r\n", SERVER_IP, SERVER_PORT);
    is_connected = 1;
    return 0;
}

// Data send task (logic unchanged, no modification needed)
static void tcp_send_task(void *arg) {
    char send_buf[128];
    int counter = 0;
    int send_len;

    while (1) {
        xSemaphoreTake(conn_mutex, portMAX_DELAY);
        if (is_connected) {
            sprintf(send_buf, "Client message %d: Hello Server\r\n", counter++);
            send_len = strlen(send_buf);
            if (send(sock_fd, send_buf, send_len, 0) != send_len) {
           //     Ifx_Console_print("Send failed, connection disconnected\r\n");
                is_connected = 0;
            } else {
               // Ifx_Console_print("Sent: %s", send_buf);
            }
        }
        xSemaphoreGive(conn_mutex);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Data receive task (logic unchanged, no modification needed)
static void tcp_recv_task(void *arg) {
    char rx_buf[RX_BUF_SIZE];
    int recv_len;

    while (1) {
        xSemaphoreTake(conn_mutex, portMAX_DELAY);
        if (is_connected) {
            recv_len = recv(sock_fd, rx_buf, RX_BUF_SIZE - 1, MSG_DONTWAIT);
            if (recv_len > 0) {
                rx_buf[recv_len] = '\0';
                //Ifx_Console_print("Received data: %s\r\n", rx_buf);
            } else if (recv_len == 0) {
                Ifx_Console_print("Server disconnected actively\r\n");
                is_connected = 0;
            } else if (recv_len < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
                //Ifx_Console_print("Receive error, connection disconnected: errno=%d\r\n", errno);
                is_connected = 0;
            }
        }
        xSemaphoreGive(conn_mutex);
        vTaskDelay(pdMS_TO_TICKS(20)); // Short delay for better responsiveness
    }
}

/**
 * Reconnection task (core fix: mutex release + unified reconnection interval)
 */
static void tcp_reconnect_task(void *arg) {
    while (1) {
        // 1. Acquire mutex (ensure thread safety)
        if (xSemaphoreTake(conn_mutex, portMAX_DELAY) == pdTRUE) {
            // 2. Attempt reconnection only when disconnected
            if (!is_connected) {
                Ifx_Console_print("Reconnecting to %s:%d...\r\n", SERVER_IP, SERVER_PORT);
                // Call connection function, release mutex regardless of success/failure
                 tcp_client_connect();
            }
            // 3. Core fix: Release mutex unconditionally (whether connect succeeds or fails)
            xSemaphoreGive(conn_mutex);
        }

        // 4. Unified reconnection interval: Wait 3s before next loop regardless of result
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// Initialization function (logic unchanged, no modification needed)
void tcp_client_init(void) {
    conn_mutex = xSemaphoreCreateMutex();
    if (conn_mutex == NULL) {
        Ifx_Console_print("Mutex creation failed (FreeRTOS resource insufficient)\r\n");
        return;
    }

   // xTaskCreate(tcp_send_task, "TCP_Send", 2048, NULL, 9, NULL);
   // xTaskCreate(tcp_recv_task, "TCP_Recv", 1024, NULL, 6, NULL);
   // xTaskCreate(tcp_reconnect_task, "TCP_Reconn", 512, NULL, 9, NULL);
   // Ifx_Console_print("TCP client initialization completed\r\n");
}
