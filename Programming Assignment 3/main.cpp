#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "huffmanTree.h"
using namespace std;

struct DecompressData
{
    // struct contains a pointer to the root node, binary code, list of positions, empty message vector, binary semaphore, condition variable, counter for threads, and turn
    node *root;
    string code;
    vector<int> positions;
    vector<char>* message;
    pthread_mutex_t* bsem;
    pthread_cond_t* waitTurn;
    int numThread;
    int turn;

    DecompressData(){} //default constructor
};

//function passed to pthread create to decompress the info, print tree information, and get the original message
void *decompress(void *arg_ptr)
{
    //casting void ptr to struct ptr
    struct DecompressData *arg = (struct DecompressData *)arg_ptr;
    
    node *cu = arg->root;
    string code = arg->code;
    vector<int> positions = arg->positions;
    int numThread = arg->numThread;

    pthread_mutex_unlock(arg->bsem);

    pthread_mutex_lock(arg->bsem);

    while(arg->turn != numThread)
    {
        pthread_cond_wait(arg->waitTurn, arg->bsem);
    }

    pthread_mutex_unlock(arg->bsem);


    for (int i = 0; i < code.length(); i++)
    {
        //find the symbol from binary code
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
            //printing tree info
            cout << "Symbol: " << cu->data << ", Frequency: " << cu->freq << ", Code: " << code << endl;
            //after symbol is found add to its corresponding position
            for (int j = 0; j < positions.size(); j++)
            {
                (*arg->message)[positions[j]] = cu->data; //store the decompressed char
                
            }
            cu = arg->root;
        }
    }

    pthread_mutex_lock(arg->bsem);

    //increment turn and broadcast
    (arg->turn) = (arg->turn) + 1;
    pthread_cond_broadcast(arg->waitTurn);

    pthread_mutex_unlock(arg->bsem);

    return nullptr;
}

int main()
{
    //read input file from STDIN
    string inFile;
    getline(cin, inFile);
  
    vector<char> symbols;
    vector<int> frequencies;
    int messageSize = 0;
    string inStr;

    int size = stoi(inFile);

    //parsing of symbols, frequencies, and the resultant message size from input
    for(int i = 0; i < size; i++)
    {
        getline(cin, inStr);
        symbols.push_back(inStr[0]);
        frequencies.push_back(stoi(inStr.substr(2, inStr.size() - 1)));
        messageSize += inStr[2] - '0';
    }

    char symbolsArr[size];
    int frequenciesArr[size];

    //pushing vectors onto arrays to be passed to the huffmanCodes function
    for (int i = 0; i < size; i++)
    {
        symbolsArr[i] = symbols[i];
        frequenciesArr[i] = frequencies[i];
    }

    //generating and getting the root of the tree
    HCT Tree;
    node *root = Tree.HuffmanCodes(symbolsArr, frequenciesArr, size);

    struct DecompressData data;
    vector<char> message(messageSize);

    pthread_t tid[size];

    static pthread_mutex_t bsem;
    pthread_mutex_init(&bsem, nullptr);

    static pthread_cond_t waitTurn = PTHREAD_COND_INITIALIZER;

    data.bsem = &bsem;
    data.waitTurn = &waitTurn;
    data.turn = 0;

    for(int i = 0; i < size; i++)
    {
        getline(cin, inStr);
        stringstream ss(inStr);
        string code;
        ss >> code;
        vector<int> positions;
        int pos;

        while (ss >> pos)
        {
            positions.push_back(pos);
        }

        pthread_mutex_lock(&bsem);

        data.root = root;
        data.code = code;
        data.positions = positions;
        data.message = &message;
        data.numThread = i;

        //creating threads
        if(pthread_create(&tid[i], NULL, decompress, &data))
        {
            cerr << "Error creating thread" << endl;
            return 1; 
        }
    }

    //joining threads
    for (int i = 0; i < size; i++)
    {
        pthread_join(tid[i], nullptr);
    }

    // printing the original message
    cout << "Original message: ";
   for(int i = 0; i < messageSize; i++)
   {
    cout << message[i];
   }

    return 0;
}