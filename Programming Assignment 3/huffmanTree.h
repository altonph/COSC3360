// Code Sourced from GeeksForGeeks
// https://www.geeksforgeeks.org/huffman-coding-using-priority-queue/
// https://www.geeksforgeeks.org/huffman-coding-greedy-algo-3/
#include <iostream>
#include <queue>
#include <string>
using namespace std;

// node struct for each node of the huffman tree
struct node
{
	char data;
	int freq;
	string code;
	node *left;
	node *right;
	int nodeAge;

	node(char character, int frequency, int age)
	{
		data = character;
		freq = frequency;
		left = right = NULL;
		nodeAge = age; 
	}
};

// comparator class to arrange symbols based on frequencies or ASCII values
class Compare
{
public:
	bool operator()(node *left, node *right)
	{
		if (left->freq == right->freq)
		{
			if (left->data != right->data)
			{
				return left->data > right->data;
			}
			else
			{
				return left->nodeAge < right->nodeAge;
			}
		}
		else
		{
			return left->freq > right->freq;
		}
	}
};

// Class Huffman Code Tree
class HCT
{
private:
	// private member variable of the root node
	node *root;

public:
	// default constructor
	HCT()
	{
		root = nullptr;
	}

	// Implementation of the Huffman Algorithm using a priority queue
	node *HuffmanCodes(char data[], int freq[], int size)
	{
		// variable declarations
		node *left, *right, *top;
		priority_queue<node *, vector<node *>, Compare> pq;
		int age = 0;

		// populating priority queue
		for (int i = 0; i < size; i++)
		{
			node *newNode = new node(data[i], freq[i], age++);
			pq.push(newNode);
		}

		// executing algorithm
		while (pq.size() != 1)
		{
			left = pq.top();
			pq.pop();
			right = pq.top();
			pq.pop();

			node *parent = new node(' ', left->freq + right->freq, age++);
			parent->left = left;
			parent->right = right;

			pq.push(parent);
		}

		//returning root node
		return pq.top();
	}
};