/*
Copyright (©) 2022-2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _LINKED_LIST_HPP
#define _LINKED_LIST_HPP

#include <stdint.h>
#include <stdio.h>
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
	Node* newNode(uint64_t data, bool eternal = false);

	// Recursive function to insert a node in the list with given data and returns the head node
	void insertNode(Node*& head, uint64_t data, bool eternal = false);

	// Get a pointer to a node from its data
	Node* findNode(Node* head, uint64_t data);

	// Delete a node
	void deleteNode(Node*& head, uint64_t key);

	// Delete a specific node
	void deleteNode(Node*& head, Node* node);

	// print the Linked list
	void fprint(fd_t file, Node* head);

	void panic(const char* str); // a tiny function which just expands the PANIC macro. This is so PANIC can be called from the template class below.

	template <typename T> class SimpleLinkedList {
	public:
		SimpleLinkedList(bool eternal = false) : m_count(0), m_start(nullptr), m_eternal(eternal) {}
		~SimpleLinkedList() {
			if (m_eternal) {
				panic("Eternal SimpleLinkedList was deleted!");
			}
			for (uint64_t i = 0; i < m_count; i++)
				remove(i);
		}

		void insert(const T* obj) {
			if (findNode(m_start, (uint64_t)&obj) != nullptr) {
				dbgprintf("[%s] WARN: object already exists. Not inserting.\n", __extension__ __PRETTY_FUNCTION__);
				return; // object already exists
			}
			insertNode(m_start, (uint64_t)obj, m_eternal);
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
		uint64_t getIndex(const T* obj) const {
			Node* temp = m_start;
			for (uint64_t i = 0; i < m_count; i++) {
				if (temp == nullptr)
					return UINT64_MAX;
				if (temp->data == (uint64_t)obj)
					return i;
				temp = temp->next;
			}
			return UINT64_MAX;
		}
		void remove(uint64_t index) {
			if (m_eternal) {
				panic("Eternal SimpleLinkedList node was deleted!");
			}
			deleteNode(m_start, (uint64_t)get(index));
			m_count--;
		}
		void remove(const T* obj) {
			if (m_eternal) {
				panic("Eternal SimpleLinkedList node was deleted!");
			}
			deleteNode(m_start, (uint64_t)obj);
			m_count--;
		}
		void rotateLeft() {
			if (m_count < 2) {
				//dbgprintf("[%s] WARN: not enough nodes to rotate.\n", __extension__ __PRETTY_FUNCTION__);
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
		void fprint(const fd_t file) {
			fprintf(file, "LinkedList order: ");
			Node* node = m_start;
			for (uint64_t i = 0; i < m_count; i++) {
				if (node == nullptr)
					break;
				fprintf(file, "%lx ", node->data);
				node = node->next;
			}
			fprintf(file, "\n");
		}

		uint64_t getCount() const {
			return m_count;
		}

	private:
		uint64_t m_count;
		Node* m_start;
		bool m_eternal; // for a linked list that will never have nodes deleted
	};

}

extern bool operator==(LinkedList::Node left, LinkedList::Node right);

#endif /* _LINKED_LIST_HPP */
