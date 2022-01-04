// MCA.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "MCA.h"
int main()
{
    //Global Variables
    SOCKET s;
    struct sockaddr_in server, si_other;
    int slen, recv_len;
    char buf[BUFLEN];
    WSADATA wsa;

    slen = sizeof(si_other);
    float MotionData[BUFLEN / 4];


    
    InitializeWinsock(wsa);//Initialise winsock   
    CreateAndPrepareSocket(s, server);//Create a socket

    //Bind
    BindSocket(s, server);

    //keep listening for data
    while (1)
    {
        ZeroMemory(buf, BUFLEN);
        //printf("Waiting for data...");
        fflush(stdout);

        //clear the buffer by filling null, it might have previously received data
        memset(buf, '\0', BUFLEN);

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen)) == SOCKET_ERROR)
        {
            printf("recvfrom() failed with error code : %d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        //print details of the client/peer and the data received
        char clientIp[BUFLEN];
        ZeroMemory(clientIp, BUFLEN);
        inet_ntop(AF_INET, &si_other.sin_addr, clientIp, BUFLEN);
       //printf("Received packet from %s:%d\n", clientIp, ntohs(si_other.sin_port));
        
        DeserializeRecvData(MotionData, buf);

        //Print Motion Data
        printf("\n");
        for (float a:MotionData)
        {
            printf("%f, ", a);
        }
       


       // printf("Data: %s\n", buf);

        ////now reply the client with the same data
        //if (sendto(s, buf, recv_len, 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
        //{
        //    printf("sendto() failed with error code : %d", WSAGetLastError());
        //    exit(EXIT_FAILURE);
        //}
    }

    closesocket(s); //close socket
    WSACleanup(); //shutdown socket

    return 0;
}

void BindSocket(const SOCKET& s, sockaddr_in& server)
{
    if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("Bind done");
}

void CreateAndPrepareSocket(SOCKET& s, sockaddr_in& server)
{
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }
    printf("Socket created.\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
}

void InitializeWinsock(WSADATA& wsa)
{
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Initialised.\n");
}

void DeserializeRecvData(float*  floatArray, char* byteArray)
{
    int itr = 0;
    for (int i = 0; i < BUFLEN; i += 4)
    {
        if (itr< BUFLEN / 4)
        {
            floatArray[itr] = BytesToFloat(byteArray[i], byteArray[i + 1], byteArray[i + 2], byteArray[i + 3]);
            itr++;
        }
    }

    /*printf("\n");
    for (int k = 0; k < BUFLEN / 4; k++)
    {
        printf("%f, ", floatArray[k]);
    }*/
}


float BytesToFloat(char b0, char b1, char b2, char b3)
{
   // char byte_array[] = { b3, b2, b1, b0 };
    char byte_array[] = { b0, b1, b2, b3 };
    float result;
    std::copy(reinterpret_cast<const char*>(&byte_array[0]),
        reinterpret_cast<const char*>(&byte_array[4]),
        reinterpret_cast<char*>(&result));
    return result;
}


