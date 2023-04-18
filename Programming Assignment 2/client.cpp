#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <pthread.h>
using namespace std;

struct DecompressData
{
    // struct contains binary code, list of positions, an an empty vector for the resultant message, size of final message, port number, and host
    string code;
    vector<int> positions;
    vector<char> message;
    int messageSize;
    char* portno;
    char* host;

    DecompressData(string c, vector<int> pos, vector<char> mes, char* pn, char* h)
    {
        code = c;
        positions = pos;
        message = mes;
        messageSize = 0;
        portno = pn;
        host = h;
    }
};

// read from STDIN and fills the struct with data
void FillDecompressData(vector<DecompressData> &data, char* portno, char* host)
{
    //1. Receives the information about the symbol to decompress (binary code and list of positions) from the main thread.
    vector<char> message;
    string comStr;
    while (getline(cin, comStr))
    {
        stringstream ss(comStr);
        string code;
        ss >> code;
        vector<int> positions;
        int pos;
        while (ss >> pos)
        {
            positions.push_back(pos);
        }
        data.push_back(DecompressData(code, positions, message, portno, host));
    }
}

//create socket to communicate with the server and send binary code
void* connectToServer(void* arg_ptr)
{
    //code referenced from blackboard in class assignment
    DecompressData *arg = (DecompressData*) arg_ptr;

    //2. Create a socket to communicate with the server program.
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = atoi(arg->portno);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket";
    }

    server = gethostbyname(arg->host);

    if (server == NULL)
    {
        std::cerr << "ERROR, no such host\n ";
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR connecting";
        exit(1);
    }
    
    // 3. Send the binary code to the server program using sockets.
    char* buffer = new char[arg->code.size() + 1];
    bzero(buffer, arg->code.size() + 1);
    strcpy(buffer, arg->code.c_str());
    int bufferSize = strlen(buffer);

    // writing buffer size to the server
    n = write(sockfd, &bufferSize, sizeof(int));
    if(n < 0)
    {
        cerr << "ERROR writing to socket";
        exit(1);
    }

    // writing binary code to the server
    n = write(sockfd, buffer, bufferSize);
    if(n < 0)
    {
        cerr << "ERROR writing to socket";
        exit(1);
    }

    int messageSize;

    //reading final message size from the server
    n = read(sockfd, &messageSize, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket";
        exit(1);
    }

    char decodedChar;

    // 4. Wait for the decoded representation of the binary code (character) from the server program.

    //reading the decoded representation of the binary code from server
    n = read(sockfd, &decodedChar, sizeof(char));
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket";
        exit(1);
    }

    // 5. Write the received information into a memory location accessible by the main thread.
    vector<char> message(messageSize);
    arg->messageSize = messageSize;
    arg->message = message;

    for(int i = 0; i < arg->code.size(); i++)
    {
        for(int j = 0; j < arg->positions.size(); j++)
        {
            arg->message[arg->positions[j]] = decodedChar;
        }
    }
     
    close(sockfd);
    delete [] buffer;
    return nullptr;
}

int main(int argc, char *argv[])
{

    static vector<DecompressData> data; //vector used to store binary code, list of positions, and final message

    if (argc < 3)
    {
        std::cerr << "usage " << argv[0] << "hostname port\n";
        exit(0);
    }
    
    //function call to add data to the struct with the port and host
    FillDecompressData(data, argv[2], argv[1]);

    //After reading the information from STDIN, this program creates m child threads (where m is the size of the alphabet). Each child thread executes the following tasks: 
    
    const int NTHREADS = data.size();
    pthread_t tid[NTHREADS];

    for(int i = 0; i < NTHREADS; i++)
    {
        if(pthread_create(&tid[i], NULL, connectToServer, &data[i]))
        {
            cerr << "Cannot create thread" << endl;
        } 
    }
    
    for(int i = 0; i < NTHREADS; i++)
    {
        pthread_join(tid[i], nullptr); //join threads
    }

    //Finally, after receiving the binary codes from the child threads, the main thread prints the original message.
    cout << "Original message: ";
    
    vector<char> decoded(data[0].messageSize);

    for(int i = 0; i < NTHREADS; i++)
    {
        for(int j = 0; j < data[i].message.size(); j++)
        {
         decoded[j] += data[i].message[j];
        }
    }

    for (int i = 0; i < decoded.size(); i++)
    {
        if(decoded[i] != '\0')
        {
        cout << decoded[i];
        }
    }

   return 0; 
}