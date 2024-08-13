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

#include "Bitmap.hpp"
#include "AVLTree.hpp"

#include <math.h>
#include <stdio.h>
#include <util.h>

#include <Memory/newdelete.hpp> // required for creating and deleting nodes
#include <Memory/kmalloc.hpp>

namespace AVLTree {

	Node nodePool[POOL_SIZE];
	uint64_t nodePool_UsedCount;
	uint8_t nodePool_BitmapData[POOL_SIZE / 8];
	Bitmap nodePool_Bitmap;
	bool nodePoolHasBeenInitialised = false;

	void NodePool_Init() {
		nodePool_UsedCount = 0;
		memset(nodePool_BitmapData, 0, POOL_SIZE / 8);
		nodePool_Bitmap.SetSize(POOL_SIZE / 8);
		nodePool_Bitmap.SetBuffer(&(nodePool_BitmapData[0]));
		nodePoolHasBeenInitialised = true;
	}

	void NodePool_Destroy() {
		nodePool_Bitmap.~Bitmap();
		nodePoolHasBeenInitialised = false;
	}

	Node* NodePool_AllocateNode() {
		if (nodePool_UsedCount == POOL_SIZE)
			return nullptr;

		for (uint64_t i = 0; i < POOL_SIZE; i++) {
			if (nodePool_Bitmap[i] == 0) {
				nodePool_Bitmap.Set(i, true);
				nodePool_UsedCount++;
				memset(&(nodePool[i]), 0, sizeof(Node));
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

	bool NodePool_HasBeenInitialised() {
		return nodePoolHasBeenInitialised;
	}

	bool NodePool_IsInPool(Node* obj) {
        return (((uint64_t)obj >= (uint64_t)nodePool) && ((uint64_t)obj < ((uint64_t)nodePool + POOL_SIZE * sizeof(Node))));
    }


	uint64_t height(Node* root) {
		if (root == nullptr)
			return 0;
		return root->height;
	}

	Node* newNode(uint64_t key, uint64_t extraData, bool vmm) {
		Node* node = nullptr;
		if (vmm)
			node = (Node*)kcalloc_vmm(1, sizeof(Node));
		else if (NewDeleteInitialised())
			node = new Node();
		else
			node = NodePool_AllocateNode();

		if (node == nullptr)
			return nullptr; // protects against page faults

		node->key = key;
		node->extraData = extraData;
		node->left = nullptr;
		node->right = nullptr;
		node->height = 1;
		return node;
	}

	Node* rightRotate(Node* root) {
		Node* x = root->left;
		Node* T2 = x->right;

		// Perform rotation
		x->right = root;
		root->left = T2;

		// Update heights
		root->height = ulmax(height(root->left), height(root->right)) + 1;
		x->height = ulmax(height(x->left), height(x->right)) + 1;

		// Return new root
		return x;
	}

	Node* leftRotate(Node* root) {
		if (root == nullptr)
			return nullptr;
		Node* y = root->right;
		Node* T2 = y->left;

		// Perform rotation
		y->left = root;
		root->right = T2;

		// Update heights
		root->height = ulmax(height(root->left), height(root->right)) + 1;
		y->height = ulmax(height(y->left), height(y->right)) + 1;

		// Return new root
		return y;
	}

	int64_t getBalance(Node* N) {
		if (N == nullptr)
			return 0;
		return height(N->left) - height(N->right);
	}

	void insert(Node*& root, uint64_t key, uint64_t extraData, bool vmm) {
		/* 1. Perform the normal BST insertion */
		if (root == nullptr) {
			root = newNode(key, extraData, vmm);
			return;
		}

		if (key < root->key)
			insert(root->left, key, extraData, vmm);
		else if (key > root->key)
			insert(root->right, key, extraData, vmm);
		else // Equal keys are not allowed in BST
			return;

		/* 2. Update height of this ancestor node */
		root->height = 1 + ulmax(height(root->left), height(root->right));

		// STEP 3: GET THE BALANCE FACTOR OF THIS NODE
		int balance = getBalance(root);

		// STEP 4: BALANCE THE NODE
		// Left Left Case
		if (balance > 1 && key < root->left->key)
			root = rightRotate(root);

		// Right Right Case
		if (balance < -1 && key > root->right->key)
			root = leftRotate(root);

		// Left Right Case
		if (balance > 1 && key > root->left->key) {
			if (root->left->right != nullptr) {
				root->left = leftRotate(root->left);
				root = rightRotate(root);
			}
		}

		// Right Left Case
		if (balance < -1 && key < root->right->key) {
			if (root->right->left != nullptr) {
				root->right = rightRotate(root->right);
				root = leftRotate(root);
			}
		}
	}

	Node* findNode(Node* root, uint64_t key) {
		if (root == nullptr)
			return nullptr;
		if (root->key == key)
			return root;
		else if (root->right != nullptr && root->key < key)
			return findNode(root->right, key);
		else if (root->left != nullptr && root->key > key)
			return findNode(root->left, key);
		else
			return nullptr;
	}

	Node* findNodeOrHigher(Node* root, uint64_t key) {
		if (root == nullptr)
			return nullptr;
		Node* temp = findNode(root, key);
		if (temp != nullptr)
			return temp;
		else if (root->key >= key) {
			Node* node = findNodeOrHigher(root->left, key);
			return node == nullptr ? root : node;
		}
		else
			return findNodeOrHigher(root->right, key);
	}


	Node* minValueNode(Node* root) {
		Node* current = root;

		/* loop down to find the leftmost leaf */
		while (current->left != nullptr)
			current = current->left;

		return current;
	}

	void deleteNode(Node*& root, uint64_t key, bool vmm) {
		// Step 1: Perform standard BST delete
		if (root == nullptr) return;

		if (key < root->key) // If the key to be deleted is smaller than the root's key, then it lies in the left subtree
			deleteNode(root->left, key, vmm);
		else if (key > root->key) // If the key to be deleted is greater than the root's key, then it lies in the right subtree
			deleteNode(root->right, key, vmm);
		else { // If key is same as root's key, then this is the node to be deleted
			if (root->left == nullptr) { // Node with only one child or no child
				Node* temp = root->right;
				if (NodePool_IsInPool(root))
					NodePool_FreeNode(root);
				else if (vmm)
					kfree_vmm(root);
				else if (NewDeleteInitialised())
					delete root;
				root = temp;
			}
			else if (root->right == nullptr) {
				Node* temp = root->left;
				if (NodePool_IsInPool(root))
					NodePool_FreeNode(root);
				else if (vmm)
					kfree_vmm(root);
				else if (NewDeleteInitialised())
					delete root;
				root = temp;
			}
			else { // Node with two children: Get the in-order successor (smallest in the right subtree)
				Node* temp = minValueNode(root->right);
				root->key = temp->key;
				root->extraData = temp->extraData;
				deleteNode(root->right, temp->key, vmm);
			}
		}

		// If the tree had only one node then return
		if (root == nullptr) return;

		/* 2. Update height of this ancestor node */
		root->height = 1 + ulmax(height(root->left), height(root->right));

		// STEP 3: GET THE BALANCE FACTOR OF THIS NODE
		int balance = getBalance(root);

		// STEP 4: BALANCE THE NODE
		// Left Left Case
		if (balance > 1 && getBalance(root->left) >= 0)
			root = rightRotate(root);

		// Left Right Case
		if (balance > 1 && getBalance(root->left) < 0) {
			root->left = leftRotate(root->left);
			root = rightRotate(root);
		}

		// Right Right Case
		if (balance < -1 && getBalance(root->right) <= 0)
			root = leftRotate(root);

		// Right Left Case
		if (balance < -1 && getBalance(root->right) > 0) {
			root->right = rightRotate(root->right);
			root = leftRotate(root);
		}
	}

	void preOrder(Node* root) {
		if (root != nullptr) {
			if (root->left != nullptr) {
				dbgputs("left:");
				preOrder(root->left);
			}
			dbgprintf("centre:%lu(%lu) ", root->key, root->extraData);
			if (root->right != nullptr) {
				dbgputs("right:");
				preOrder(root->right);
			}
		}
		else
			dbgputc(' ');
	}

	Node* getParent(Node* root, uint64_t key) {
		if (root == nullptr)
			return nullptr;
		else if (root->left == nullptr || root->right == nullptr)
			return nullptr;
		else if (root->left->key == key || root->right->key == key)
			return root;
		else if (root->key < key)
			return getParent(root->right, key);
		else if (root->key > key)
			return getParent(root->left, key);
		else if (root->key == key)
			return nullptr;
		else
			return nullptr;
	}

}
