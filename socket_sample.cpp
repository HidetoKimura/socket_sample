
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENT_NUM 5
#define BUFFER_SIZE 1024

void *handle_client(void *arg);

void *handle_server(void *arg) {
    int server_socket = *(int*)arg;
    int client_socket[MAX_CLIENT_NUM];
    struct sockaddr_in client_address;
    pthread_t thread_id[MAX_CLIENT_NUM];
    int i = 0 ;

    // Accept incoming connections
    std::cout << "Waiting for incoming connections..." << std::endl;
    int c = sizeof(struct sockaddr_in);
    while ((client_socket[i] = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t*)&c))) {
        std::cout << "Connection accepted" << std::endl;
        std::cout << "Client Socket fd: " << client_socket[i] << std::endl;
        if (client_socket[i] < 0) {
            std::cout << "Accept failed" << std::endl;
            return NULL;
        }
        if (pthread_create(&thread_id[i], NULL, handle_client, (void*)&client_socket[i]) < 0) {
            std::cout << "Could not create thread" << std::endl;
            return NULL;
        }
        if (i >= MAX_CLIENT_NUM - 1) {
            break;
        }
        i++;
    }

    pthread_exit(NULL);
}

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    int read_size;

    std::cout << "Server thread created:" << client_socket << std::endl;
    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        std::cout << client_socket << ":[Server] Client message: " << buffer << std::endl;
        snprintf(buffer, BUFFER_SIZE, "Hello from server");
        send(client_socket, buffer, BUFFER_SIZE, 0);
        memset(buffer, 0, BUFFER_SIZE);
        break;
    }

    if (read_size == 0) {
        std::cout << "Client disconnected" << std::endl;
    } else if (read_size == -1) {
        std::cout << "recv failed" << std::endl;
    }

    close(client_socket);
    pthread_exit(NULL);
}


void *test_client(void *arg) {

    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    int read_size;

    // Connect to server
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cout << "Could not create socket" << std::endl;
        pthread_exit(NULL);
    }

    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8888);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cout << "Connect failed" << std::endl;
        pthread_exit(NULL);
    }

    std::cout << (char*)arg << " connected" << std::endl;

    memset(buffer, 0, BUFFER_SIZE);
    snprintf(buffer, BUFFER_SIZE, "Hello from client");
    send(client_socket, buffer, BUFFER_SIZE, 0);

    memset(buffer, 0, BUFFER_SIZE);
    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        std::cout << (char*)arg << ":[Client] Server message: " << buffer << std::endl;
        memset(buffer, 0, BUFFER_SIZE);
        break;
    }

    close(client_socket);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    int server_socket;
    struct sockaddr_in server_address;
    pthread_t server_thread_id, client_thread_id[3];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cout << "Could not create socket" << std::endl;
        return 1;
    }

    // Prepare the sockaddr_in structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(8888);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cout << "Bind failed" << std::endl;
        return 1;
    }
    std::cout << "Bind done" << std::endl;
    // Listen
    listen(server_socket, MAX_CLIENT_NUM - 1);
    
    // Create server thread
    if (pthread_create(&server_thread_id, NULL, handle_server, (void*)&server_socket) < 0) {
        std::cout << "Could not create thread" << std::endl;
        return 1;
    }
    std::cout << "Server thread created" << std::endl;
    sleep(1);

    // Create client thread
    if (pthread_create(&client_thread_id[0], NULL, test_client, (void*)"Client A") < 0) {
        std::cout << "Could not create thread" << std::endl;
        return 1;
    }
    if (pthread_create(&client_thread_id[1], NULL, test_client, (void*)"Client B") < 0) {
        std::cout << "Could not create thread" << std::endl;
        return 1;
    }
    if (pthread_create(&client_thread_id[2], NULL, test_client, (void*)"Client C") < 0) {
        std::cout << "Could not create thread" << std::endl;
        return 1;
    }

    pthread_join(client_thread_id[0], NULL);
    pthread_join(client_thread_id[1], NULL);
    pthread_join(client_thread_id[2], NULL);
    std::cout << "Client thread finished" << std::endl;

    pthread_cancel(server_thread_id);
    std::cout << "Server thread finished" << std::endl;

    return 0;
}
