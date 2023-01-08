#include "AVLTree.hpp"

#include <stdio.hpp>

#include <math.h>

#include <Memory/newdelete.h> // required for creating and deleting nodes

namespace AVLTree {

	uint64_t height(Node* root) {
		if (root == nullptr)
			return 0;
		return root->height;
	}

	Node* newNode(uint64_t key, void* extraData) {
		Node* node = new Node(); /* TODO: fix this function */
		node->key = key;
		node->extraData = extraData;
		node->left = nullptr;
		node->right = nullptr;
		node->height = 1;
		return(node);
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

	void insert(Node*& root, uint64_t key, void* extraData) {
		/* 1. Perform the normal BST insertion */
		if (root == nullptr) {
			root = newNode(key, extraData);
			return;
		}

		if (key < root->key)
			insert(root->left, key, extraData);
		else if (key > root->key)
			insert(root->right, key, extraData);
		else // Equal keys are not allowed in BST
			return;

		/* 2. Update height of this ancestor node */
		root->height = 1 + ulmax(height(root->left), height(root->right));

		/* 3. Get the balance factor of this ancestor node to check whether this node became unbalanced */
		int64_t balance = getBalance(root);

		// If this node becomes unbalanced, then
		// there are 4 cases

		// Left Left Case
		if (balance > 1 && key < root->left->key) {
			root = rightRotate(root);
			return;
		}

		// Right Right Case
		if (balance < -1 && key > root->right->key) {
			root = leftRotate(root);
			return;
		}

		// Left Right Case
		if (balance > 1 && key > root->left->key) {
			root->left = leftRotate(root->left);
			root = rightRotate(root);
			return;
		}

		// Right Left Case
		if (balance < -1 && key < root->right->key) {
			root = leftRotate(root);
			return;
		}
	}

	Node* findNode(Node* root, uint64_t key) {
		if (root->left == nullptr || root->right == nullptr)
			return nullptr;
		else if (root->key < key)
			return findNode(root->right, key);
		else if (root->key > key)
			return findNode(root->left, key);
		else if (root->key == key)
			return root;
		else
			return nullptr;
	}

	Node* minValueNode(Node* root) {
		Node* current = root;

		/* loop down to find the leftmost leaf */
		while (current->left != nullptr)
			current = current->left;

		return current;
	}

	void deleteNode(Node*& root, uint64_t key) {
		// Step 1: Perform standard BST delete
		if (root == nullptr) return;

		// If the key to be deleted is smaller than the root's key,
		// then it lies in the left subtree
		if (key < root->key)
			deleteNode(root->left, key);

		// If the key to be deleted is greater than the root's key,
		// then it lies in the right subtree
		else if (key > root->key)
			deleteNode(root->right, key);

		// If key is same as root's key, then this is the node
		// to be deleted
		else {
			// Node with only one child or no child
			if (root->left == nullptr) {
				Node* temp = root->right;
				delete root;
				root = temp;
			}
			else if (root->right == nullptr) {
				Node* temp = root->left;
				delete root;
				root = temp;
			}

			// Node with two children: Get the in-order successor (smallest
			// in the right subtree)
			else {
				Node* temp = minValueNode(root->right);
				root->key = temp->key;
				deleteNode(root->right, temp->key);
			}
		}

		// If the tree had only one node then return
		if (root == nullptr) return;

		// Step 2: Update the height of the current node
		root->height = 1 + ulmax(height(root->left), height(root->right));

		// Step 3: Get the balance factor of this ancestor node to check whether
		// this node became unbalanced
		int64_t balance = getBalance(root);

		// If this node becomes unbalanced, then there are 4 cases

		// Left Left Case
		if (balance > 1 && getBalance(root->left) >= 0)
			rightRotate(root);

		// Left Right Case
		if (balance > 1 && getBalance(root->left) < 0) {
			leftRotate(root->left);
			rightRotate(root);
		}

		// Right Right Case
		if (balance < -1 && getBalance(root->right) <= 0)
			leftRotate(root);

		// Right Left Case
		if (balance < -1 && getBalance(root->right) > 0) {
			rightRotate(root->right);
			leftRotate(root);
		}
	}

	void preOrder(Node* root) {
		if (root != nullptr) {
			printf("%uld ", root->key);
			preOrder(root->left);
			preOrder(root->right);
		}
	}

}
