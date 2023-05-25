#ifndef _LINKED_LIST_HPP
#define _LINKED_LIST_HPP

#include <stdint.h>


namespace LinkedList {

	static constexpr uint64_t POOL_SIZE = 128;

	struct Node {
		Node* previous;
		uint64_t data;
		Node* next;
	};

	void NodePool_Init();
	void NodePool_Destroy();
	Node* NodePool_AllocateNode();
	bool NodePool_FreeNode(Node*& node);
	bool NodePool_HasBeenInitialised();
	bool NodePool_IsInPool(Node* obj);

	// Get length of the Linked list
	uint64_t length(Node* head);


	// Helper function that allocates a new node with the given data and NULL previous and next pointers.
	Node* newNode(uint64_t data);

	// Recursive function to insert a node in the list with given data and returns the head node
	void insert(Node*& head, uint64_t data);

	// Get a pointer a node from its data
	Node* findNode(Node* head, uint64_t data);

	// Delete a node
	void deleteNode(Node*& head, uint64_t key);

	// print the Linked list
	void print(Node* head);

}

extern bool operator==(LinkedList::Node left, LinkedList::Node right);

#endif /* _LINKED_LIST_HPP */
