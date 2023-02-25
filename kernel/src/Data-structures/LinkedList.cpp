#include "LinkedList.hpp"
#include "Bitmap.hpp"

#include <stdio.hpp>
#include <util.h>
#include <Memory/newdelete.hpp> // required for creating and deleting nodes

bool operator==(LinkedList::Node left, LinkedList::Node right) {
	return ((left.data == right.data) && (left.next = right.next) && (left.previous == right.previous));
}

namespace LinkedList {

	Node nodePool[POOL_SIZE];
	uint64_t nodePool_UsedCount;
	uint8_t nodePool_BitmapData[POOL_SIZE / 8];
	WorldOS::Bitmap nodePool_Bitmap;

	void NodePool_Init() {
		nodePool_UsedCount = 0;
		memset(nodePool_BitmapData, 0, POOL_SIZE / 8);
		nodePool_Bitmap.SetSize(POOL_SIZE);
		nodePool_Bitmap.SetBuffer(&(nodePool_BitmapData[0]));
	}

	void NodePool_Destroy() {
		nodePool_Bitmap.~Bitmap();
	}

	Node* NodePool_AllocateNode() {
		if (nodePool_UsedCount == POOL_SIZE - 1) return nullptr;
		for (uint64_t i = 0; i < POOL_SIZE; i++) {
			if (nodePool_Bitmap[i] == 0) {
				nodePool_Bitmap.Set(i, true);
				nodePool_UsedCount++;
				return &(nodePool[i]);
			}
		}
		return nullptr;
	}

	bool NodePool_FreeNode(Node* node) {
		for (uint64_t i = 0; i < POOL_SIZE; i++) {
			if ((uint64_t)node == (uint64_t)(&(nodePool[i]))) {
				nodePool_Bitmap.Set(i, false);
				nodePool_UsedCount--;
				return true;
			}
		}
		return false;
	}


	uint64_t length(Node* head) {
		if (head == nullptr)
			return 0;
		
		Node* current = head;
		uint64_t count = 0;
		while (true) {
			count++;
			if (current->next == nullptr) break;
			current = current->next;
		}

		current = nullptr; // protects the node that current points to from potential deletion

		return count;
	}

	Node* newNode(uint64_t data) {
		Node* node = nullptr;
		if (NewDeleteInitialised())
			node = new Node();
		else
			node = NodePool_AllocateNode();

		if (node == nullptr)
			return nullptr; // protects against page faults

		node->data = data;
		node->previous = nullptr;
		node->next = nullptr;
		return node;
	}

	void insert(Node*& head, uint64_t data) {
		// check if head is NULL
		if (head == nullptr) {
			head = newNode(data);
			return;
		}

		// move to last node
		Node* current = head;
		while (true) {
			if (current->next == nullptr) break;
			current = current->next;
		}

		// get new node and set last node's next to it
		current->next = newNode(data);

		// update newly created node's previous to the last node
		current->next->previous = current;

		// clear the value of current to protect the node it is pointing to from possible deletion
		current = nullptr;
	}

	Node* findNode(Node* head, uint64_t data) {
		Node* current = head;
		while (current != nullptr) {
			if (current->data == data) return current;
			current = current->next;
		}

		current = nullptr; // protects the node that current points to from potential deletion
		return nullptr;
	}

	void deleteNode(Node*& head, uint64_t data) {
		Node* temp = head;
		if (temp != nullptr && temp->data == data) {
			head = temp->next;
			head->previous = nullptr;
			if (NewDeleteInitialised())
				delete temp;
			else
				NodePool_FreeNode(temp);
			return;
		}
		while (temp != nullptr && temp->data != data) {
			temp = temp->next;
		}
		if (temp == nullptr) return;
		if (temp->next != nullptr)
			temp->next->previous= temp->previous;
		if (temp->previous!= nullptr)
			temp->previous->next = temp->next;
		if (NewDeleteInitialised())
			delete temp;
		else
			NodePool_FreeNode(temp);
	}

	void print(Node* head) {
		printf("Linked list order: ");

		Node* current = head;
		while (current != nullptr) {
			printf(" %lu ", current->data);
			current = current->next;
		}

		printf("\n");

		// clear the value of current to protect the node it is pointing to from possible deletion
		current = nullptr;
	}

}
