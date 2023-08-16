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

#ifndef _AVL_TREE_HPP
#define _AVL_TREE_HPP

#include <stdint.h>

namespace AVLTree {

	static constexpr uint64_t POOL_SIZE = 128;

	struct Node {
		// actual data
		uint64_t key;
		uint64_t extraData;

		// Required extras
		Node* left;
		Node* right;
		uint64_t height;
	};

	void NodePool_Init();
	void NodePool_Destroy();
	Node* NodePool_AllocateNode();
	bool NodePool_FreeNode(Node* node);
	bool NodePool_HasBeenInitialised();
	bool NodePool_IsInPool(Node* obj);

	// Get height of the AVL tree
	uint64_t height(Node* root);

	// Helper function that allocates a new node with the given key, extra data and NULL left and right pointers.
	Node* newNode(uint64_t key, uint64_t extraData);

	// A utility function to right rotate subtree.
	Node* rightRotate(Node* root);

	// A utility function to left rotate subtree.
	Node* leftRotate(Node* root);

	// Get Balance factor of node N.
	int64_t getBalance(Node* N);

	// Recursive function to insert a key in the subtree and returns the new root of the subtree.
	void insert(Node*& root, uint64_t key, uint64_t extraData);

	// Get a pointer to a node from its key
	Node* findNode(Node* root, uint64_t key);

	// Get a pointer to a node from its key or the key with the next highest value
	Node* findNodeOrHigher(Node* root, uint64_t key);

	// Get the node with the lowest value in the tree
	Node* minValueNode(Node* root);

	// Delete a node
	void deleteNode(Node*& root, uint64_t key);


	// A utility function to print preorder traversal of the tree.
	void preOrder(Node* root);

	// find Parent node of a key
	Node* getParent(Node* root, uint64_t key);

}

#endif /* _AVL_TREE_HPP */
