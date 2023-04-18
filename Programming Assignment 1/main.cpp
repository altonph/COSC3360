#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <pthread.h>
#include "huffmanTree.h"
using namespace std;

struct DecompressData
{
    // struct contains a pointer to the root node, binary code, list of positions, an an empty vector for the resultant message
    node *root;
    string code;
    vector<int> positions;
    vector<char> message;

    DecompressData(node *r, string c, vector<int> pos, vector<char> mes)
    {
        root = r;
        code = c;
        positions = pos;
        message = mes;
    }
};

//function passed to pthread create to decompress the info from the compressed file
void *decompress(void *arg_ptr)
{
    //casting void ptr to struct ptr
    struct DecompressData *arg = (struct DecompressData *)arg_ptr;

    node *cu = arg->root;

    for (int i = 0; i < arg->code.length(); i++)
    {
        //find the symbol from binary code
        if (arg->code[i] == '0')
        {
            cu = cu->left;
        }
        else
        {
            cu = cu->right;
        }

         if (cu->left == nullptr && cu->right == nullptr)
         {
            //after symbol is found add to its corresponding position
            for (int j = 0; j < arg->positions.size(); j++)
            {
                arg->message[arg->positions[j]] = cu->data;
            }
            cu = arg->root;
        }

    }
    return nullptr;
}

int main()
{
    //read input file from STDIN
    string inFile;
    getline(cin, inFile);
    ifstream ifs(inFile);
  
    vector<char> symbols;
    vector<int> frequencies;
    int messageSize = 0;
    string inStr;

    //parsing of symbols, frequencies, and the resultant message size from input
    while (getline(ifs, inStr))
    {
        symbols.push_back(inStr[0]);
        frequencies.push_back(stoi(inStr.substr(2, inStr.size() - 1)));
        messageSize += inStr[2] - '0';
    }

    int size = symbols.size();

    char symbolsArr[size];
    int frequenciesArr[size];

    //pushing vectors onto arrays to be passed to the huffmanCodes function
    for (int i = 0; i < size; i++)
    {
        symbolsArr[i] = symbols[i];
        frequenciesArr[i] = frequencies[i];
    }

    //generating, printing, and getting the root of the tree
    HCT Tree;
    node *root = Tree.HuffmanCodes(symbolsArr, frequenciesArr, size);

    //read the compressed file from STDIN
    string comFile;
    getline(cin, comFile);
    ifstream com(comFile);
  
    static vector<DecompressData> data;
    string comStr;
    vector<char> message(messageSize);
    
    //parsing of the compressed file and pushing the data from each line onto the vector of structs
    while (getline(com, comStr))
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

        data.push_back(DecompressData(root, code, positions, message));
    }

    // creating threads
    const int NTHREADS = data.size();
    pthread_t tid[NTHREADS];

    for (int i = 0; i < NTHREADS; i++)
    {
        if (pthread_create(&tid[i], nullptr, decompress, &data[i]))
        {
            cerr << "Error creating thread" << endl;
            return 1;
        }
    }

    // joining threads
    for (int i = 0; i < NTHREADS; i++)
    {
        pthread_join(tid[i], nullptr);
    }

    // printing the original message
    cout << "Original message: ";

    vector<char> decoded(messageSize);
    
    //adding the symbol from each position to a final decoded vector
    for (int i = 0; i < NTHREADS; i++)
    {
        for (int j = 0; j < data[i].message.size(); j++)
        {
           decoded[j] += data[i].message[j];
        }
    }

    //printing out the final decoded vector that stores the original message
    for(int i = 0; i < decoded.size(); i++)
    {
        cout << decoded[i];
    }

    //closing files
    ifs.close();
    com.close();

    return 0;
}