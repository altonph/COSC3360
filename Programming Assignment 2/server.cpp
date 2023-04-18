#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "huffmanTree.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
using namespace std;

//handles zombie processes
void fireman(int)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[])
{
    vector<char> symbols;
    vector<int> frequencies;
    int messageSize = 0;
    string inStr;

    // parsing of symbols, frequencies, and the resultant message size from input
    while (getline(cin, inStr))
    {
        symbols.push_back(inStr[0]);
        frequencies.push_back(stoi(inStr.substr(2, inStr.size() - 1)));
        messageSize += inStr[2] - '0';
    }

    int size = symbols.size();

    char symbolsArr[size];
    int frequenciesArr[size];

    // pushing vectors onto arrays to be passed to the huffmanCodes function
    for (int i = 0; i < size; i++)
    {
        symbolsArr[i] = symbols[i];
        frequenciesArr[i] = frequencies[i];
    }

    // Print the Huffman codes from the generated tree. The symbols' information (leaf nodes) must be printed from left to right.
    HCT Tree;
    node *root = Tree.HuffmanCodes(symbolsArr, frequenciesArr, size);

    // code referenced from blackboard in class assignment
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    //handles zombie processes
    signal(SIGCHLD, fireman); 

    if (argc < 2)
    {
        std::cerr << "ERROR, no port provided\n";
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket";
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cerr << "ERROR on binding";
        exit(1);
    }

    //listening for incoming request from the client
    listen(sockfd, 10);

    clilen = sizeof(cli_addr);
    
    //this loops continuously to accept incoming request
    while (true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);

        if (fork() == 0)
        {
           
         if (newsockfd < 0)
            {
                cerr << "ERROR on accept";
                exit(1);
            }
        
            //reading in the size of the buffer
            int size;
            n = read(newsockfd, &size, sizeof(int));

            if (n < 0)
            {
                std::cerr << "ERROR reading from socket";
                exit(1);
            }

            // 1. First, receive the binary code from the client program.
            //reading in binary code from client into the buffer
            char* buffer = new char[size + 1];
            bzero(buffer, size + 1);
            n = read(newsockfd, buffer, size);

            if (n < 0)
            {
                std::cerr << "ERROR reading from socket";
                exit(1);
            }

            string code = buffer;

            //2. Next, use the generated Huffman tree to decode the binary code.

            node *cu = root;
            char decodedChar;

            for(int i = 0; i < code.size(); i++)
            {
                // find the symbol from binary code
                if (code[i] == '0')
                {
                    cu = cu->left;
                }
                else
                {
                    cu = cu->right;
                }

                if (cu->left == nullptr && cu->right == nullptr)
                {
                    decodedChar = cu->data;
                }
                
            }

            // 3. Finally, return the character to the client program using sockets.

            // first writing the final message size to the client to prepare for the resultant output
            n = write(newsockfd, &messageSize, sizeof(int));
            if (n < 0)
            {
                std::cerr << "ERROR writing to socket";
                exit(1);
            }

            //returning the character to the client program via the buffer
            bzero(buffer, strlen(buffer));
            strcpy(buffer, &decodedChar);

            n = write(newsockfd, buffer, strlen(buffer));
            if (n < 0)
            {
                std::cerr << "ERROR writing to socket";
                exit(1);
            }
        
            close(newsockfd);
            delete [] buffer;
            _exit(0);
        }
    }

close(sockfd);
return 0;
}