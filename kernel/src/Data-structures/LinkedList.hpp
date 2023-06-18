#ifndef _LINKED_LIST_HPP
#define _LINKED_LIST_HPP

#include <stdint.h>
#include <stdio.hpp>
#include <assert.h>

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
	void insertNode(Node*& head, uint64_t data);

	// Get a pointer to a node from its data
	Node* findNode(Node* head, uint64_t data);

	// Delete a node
	void deleteNode(Node*& head, uint64_t key);

	// print the Linked list
	void print(Node* head);

	template <typename T> class SimpleLinkedList {
	public:
		SimpleLinkedList() : m_count(0), m_start(nullptr) {}
		~SimpleLinkedList() {
			for (uint64_t i = 0; i < m_count; i++)
				remove(i);
		}

		void insert(const T& obj) {
			/*if (findNode(m_start, (uint64_t)&obj) != nullptr) {
				fprintf(VFS_DEBUG, "[%s] WARN: object already exists. Not inserting.\n", __extension__ __PRETTY_FUNCTION__);
				return; // object already exists
			}*/
			insertNode(m_start, (uint64_t)&obj);
			m_count++;
		}
		T* get(uint64_t index) const {
			if (index >= m_count)
				return nullptr;
			Node* temp = m_start;
			for (uint64_t i = 0; i < index; i++) {
				if (temp == nullptr)
					return nullptr;
				temp = temp->next;
			}
			if (temp == nullptr)
				return nullptr;
			return (T*)(temp->data);
		}
		uint64_t getIndex(const T& obj) const {
			Node* temp = m_start;
			for (uint64_t i = 0; i < m_count; i++) {
				if (temp == nullptr)
					return UINT64_MAX;
				if (temp->data == (uint64_t)&obj)
					return i;
				temp = temp->next;
			}
			return UINT64_MAX;
		}
		void remove(uint64_t index) {
			deleteNode(m_start, (uint64_t)get(index));
			m_count--;
		}
		void remove(const T& obj) {
			deleteNode(m_start, (uint64_t)&obj);
			m_count--;
		}
		void rotateLeft() {
			if (m_count < 2) {
				fprintf(VFS_DEBUG, "[%s] WARN: not enough nodes to rotate.\n", __extension__ __PRETTY_FUNCTION__);
				return; // not enough nodes to rotate
			}
			Node* end = m_start;
			assert(end != nullptr);
			while (end->next != nullptr)
				end = end->next;
			assert(end != nullptr);
			end->next = m_start;
			m_start->previous = end;
			m_start = m_start->next;
			m_start->previous = nullptr;
			end->next->next = nullptr;
		}
		void rotateRight() {
			if (m_count < 2)
				return; // not enough nodes to rotate
			Node* end = m_start;
			assert(end != nullptr);
			while (end->next != nullptr)
				end = end->next;
			assert(end != nullptr);
			m_start->previous = end;
			end->next = m_start;
			end->previous = nullptr;
			m_start = end;
		}
		T* getHead() {
			if (m_start == nullptr)
				return nullptr;
			return (T*)(m_start->data);
		}
		void fprint(fd_t file) {
			fprintf(file, "Linked list order: ");

			Node* current = m_start;
			while (current != nullptr) {
				fprintf(file, " %lu ", current->data);
				current = current->next;
			}

			fprintf(file, "\n");
		}

		uint64_t getCount() const {
			return m_count;
		}

	private:
		uint64_t m_count;
		Node* m_start;
	};

}

extern bool operator==(LinkedList::Node left, LinkedList::Node right);

#endif /* _LINKED_LIST_HPP */
