#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9000

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char input[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        printf("Socket creation error\n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed\n");
        return 1;
    }

    printf("Connected to DB Server on port 9000\n");

    while (1)
    {
        printf(">> ");
        fgets(input, sizeof(input), stdin);

        send(sock, input, strlen(input), 0);

        int valread = read(sock, buffer, 1024);

        if (valread > 0)
        {
            buffer[valread] = '\0';
            printf("%s\n", buffer);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);

    return 0;
}
