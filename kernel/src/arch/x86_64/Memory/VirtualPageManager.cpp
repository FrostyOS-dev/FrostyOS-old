#include "VirtualPageManager.hpp"


namespace x86_64_WorldOS {

    /* Extra Internal Functions */

    // Set a node to reserved (sets highest bit of extraData)
    void Internal_UsedToReserved(AVLTree::Node* node) {
        uint64_t data = node->extraData;
        data |= (1 << 63);
        node->extraData = data;
    }

    // Set a node to used (clears highest bit of extraData)
    void Internal_ReservedToUsed(AVLTree::Node* node) {
        uint64_t data = node->extraData;
        data &= ~(1 << 63);
        node->extraData = data;
    }

    // Find free pages and remove them from the free pages tree. Returns address of the free pages.
    void* Internal_FindANDMarkPagesNotFree(AVLTree::Node* root, uint64_t count) {
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNodeOrHigher(root, count);
        if (node == nullptr)
            return nullptr; // could happen if there is a massive request or we are running low on memory
        LinkedList::Node* list = (LinkedList::Node*)node->extraData;
        if (list == nullptr)
            return nullptr; // this should **NEVER** happen, but is here just in case.
        void* mem = (void*)list->data;
        if (mem == nullptr)
            return nullptr; // once again, this should **NEVER** happen, but is here just in case.
        LinkedList::deleteNode(list, list->data);
        node->extraData = (uint64_t)list;
        uint64_t sizeFound = node->key;
        if (sizeFound > count) {
            node = AVLTree::findNode(root, sizeFound - count);
            if (node == nullptr)
                list = nullptr;
            else
                list = (LinkedList::Node*)node->extraData;
            LinkedList::insert(list, (uint64_t)mem + (sizeFound - count) * 4096);
            node->extraData = (uint64_t)list;
        }
        return mem;
    }

    // Split a Reserved/Used block. addr is original block starting address, count is the page count wanted for the first section.
    void Internal_SplitRUBlock(AVLTree::Node* root, void* addr, uint64_t count) {
        AVLTree::Node* node = AVLTree::findNode(root, (uint64_t)addr);
        if (node == nullptr)
            return; // ignore invalid block address
        const bool isReserved = (node->extraData & (1 << 63)) >> 63; // save state of block
        Internal_ReservedToUsed(node); // clear state of block to make the math easier
        if (count >= node->extraData)
            return; // ignore invalid addr/count combo
        uint64_t upperBlockSize = node->extraData - count;
        node->extraData = count | (isReserved ? (1 << 63) : 0);
        AVLTree::insert(root, (uint64_t)addr + upperBlockSize * 4096, upperBlockSize | (isReserved ? (1 << 63) : 0));
    }

    // Split a Free block. addr is original block starting address, original_count is the original size of the block, new_count is the page count wanted for the first section
    void Internal_SplitFreeBlock(AVLTree::Node* root, void* addr, uint64_t original_count, uint64_t new_count) {
        if (original_count <= new_count)
            return; // ignore invalid new_count
        AVLTree::Node* node = AVLTree::findNode(root, original_count);
        if (node == nullptr)
            return; // ignore invalid size
        LinkedList::Node* list = (LinkedList::Node*)node->extraData;
        if (list == nullptr)
            return; // this should **NEVER** happen, but is here just in case.
        LinkedList::Node* item = LinkedList::findNode(list, (uint64_t)addr);
        if (item == nullptr || item->data != (uint64_t)addr)
            return; // ignore invalid addr/size combo
        LinkedList::deleteNode(list, item->data);
        node->extraData = (uint64_t)list;
        // insert the first section of the block
        node = AVLTree::findNode(root, new_count);
        if (node == nullptr) {
            list = nullptr;
            AVLTree::insert(root, new_count, (uint64_t)nullptr);
        }
        else
            list = (LinkedList::Node*)node->extraData;
        LinkedList::insert(list, (uint64_t)addr);
        node->extraData = (uint64_t)list;
        // insert the second section of the block
        node = AVLTree::findNode(root, original_count - new_count);
        if (node == nullptr) {
            list = nullptr;
            AVLTree::insert(root, new_count, (uint64_t)nullptr);
        }
        else
            list = (LinkedList::Node*)node->extraData;
        LinkedList::insert(list, (uint64_t)addr + (original_count - new_count) * 4096);
        node->extraData = (uint64_t)list;
    }

    void Internal_mergeRUBlocks(AVLTree::Node* root, AVLTree::Node* parent, AVLTree::Node* node, AVLTree::Node* sibling) {
        bool nodeReserved = (node->extraData & (1 << 63)) >> 63;
        bool siblingReserved = (sibling->extraData & (1 << 63)) >> 63;
        uint64_t nodeSize = node->extraData & ~(1 << 63);
        uint64_t siblingSize = sibling->extraData & ~(1 << 63);
        uint64_t totalSize = nodeSize + siblingSize;

        // check if both nodes are either reserved or used
        if (nodeReserved == siblingReserved) {
            // update parent node with new combined node
            node->extraData = totalSize | (nodeReserved ? (1 << 63) : 0);
            // delete the sibling node
            AVLTree::deleteNode(parent, sibling->key);
            // recursively check if the new node can be merged with its siblings
            if (parent != nullptr) {
                AVLTree::Node* grandparent = getParent(root, parent->key);
                AVLTree::Node* new_node = (parent->left == nullptr) ? parent->right : parent->left;
                AVLTree::Node* new_sibling = (parent->left == new_node) ? parent->right : parent->left;
                Internal_mergeRUBlocks(root, grandparent, parent, new_sibling);
            }
        }
    }

    uint64_t Internal_getBlockEnd(uint64_t key, uint64_t size) {
        return key + size * 4096;
    }

    // Organise the m_FreePagesSizeTree into an AVL tree sorted by address
    void Internal_OrganiseFPSTIntoNewAVLTree(AVLTree::Node* new_root, AVLTree::Node* root) {
        if (root == nullptr)
            return;
        if (root->left != nullptr)
            Internal_OrganiseFPSTIntoNewAVLTree(new_root, root->left);
        if (root->right != nullptr)
            Internal_OrganiseFPSTIntoNewAVLTree(new_root, root->right);
        LinkedList::Node* list = (LinkedList::Node*)root->extraData;
        while (list != nullptr) {
            AVLTree::Node* node = AVLTree::findNode(new_root, list->data);
            if (node != nullptr) {
                if (root->key > node->extraData)
                    node->extraData = node->key;
            }
            else
                AVLTree::insert(new_root, list->data, root->key);
            list = list->next;
        }
    }


    void Internal_mergeFreeBlocks(AVLTree::Node* root, AVLTree::Node* parent, AVLTree::Node* node, AVLTree::Node* sibling) {
        uint64_t nodeSize = node->extraData;
        uint64_t siblingSize = sibling->extraData;
        uint64_t totalSize = nodeSize + siblingSize;

        // update parent node with new combined node
        node->extraData = totalSize;
        // delete the sibling node
        AVLTree::deleteNode(parent, sibling->key);
        // recursively check if the new node can be merged with its siblings
        if (parent != nullptr) {
            AVLTree::Node* grandparent = getParent(root, parent->key);
            AVLTree::Node* new_node = (parent->left == nullptr) ? parent->right : parent->left;
            AVLTree::Node* new_sibling = (parent->left == new_node) ? parent->right : parent->left;
            Internal_mergeRUBlocks(root, grandparent, parent, new_sibling);
        }
    }

    void Internal_cleanFreePagesTree(AVLTree::Node* root, AVLTree::Node* node, AVLTree::Node* parent) {
        if (node == nullptr) {
            return;
        }
        Internal_cleanFreePagesTree(root, node->left, node);
        Internal_cleanFreePagesTree(root, node->right, node);
        if (parent != nullptr) {
            AVLTree::Node* sibling = (parent->left == node) ? parent->right : parent->left;
            if (sibling != nullptr) {
                uint64_t nodeEnd = Internal_getBlockEnd(node->key, node->extraData);
                uint64_t siblingEnd = Internal_getBlockEnd(sibling->key, sibling->extraData);
                if (nodeEnd == sibling->key) {
                    // merge adjacent blocks
                    Internal_mergeFreeBlocks(root, parent, node, sibling);
                }
            }
        }
    }

    void Internal_clearFreePagesTree(AVLTree::Node* root) {
        if (root == nullptr)
            return;
        if (root->left != nullptr)
            Internal_clearFreePagesTree(root->left);
        if (root->right != nullptr)
            Internal_clearFreePagesTree(root->right);
        LinkedList::Node* list = (LinkedList::Node*)root->extraData;
        while (list != nullptr) {
            using namespace LinkedList;
            deleteNode(list, list->data); // will automatically assign list to the next node in line
        }
        AVLTree::deleteNode(root, root->key);
    }

    // same as below, but you can choose the root
    void Internal_OrganiseNewAVLTreeIntoFPST(AVLTree::Node* new_root, AVLTree::Node* root) {
        if (root == nullptr)
            return;
        if (root->left != nullptr)
            Internal_OrganiseNewAVLTreeIntoFPST(new_root, root->left);
        if (root->right != nullptr)
            Internal_OrganiseNewAVLTreeIntoFPST(new_root, root->right);
        AVLTree::Node* node = AVLTree::findNode(new_root, root->extraData);
        if (node == nullptr) {
            AVLTree::insert(new_root, root->extraData, (uint64_t)nullptr);
            node = AVLTree::findNode(new_root, root->extraData);
        }
        LinkedList::Node* list = (LinkedList::Node*)new_root->extraData;
        LinkedList::insert(list, root->key);
    }


    /* Virtual Page Manager Class */

    /* Public Methods */

    void VirtualPageManager::InitVPageMgr(WorldOS::MemoryMapEntry* MemoryMap, uint64_t MemoryMapEntryCount, void* kernel_virt_start, size_t kernel_size, void* fb_virt, uint64_t fb_size) {
        FreePages((void*)0x1000, 0xFFFFFFFFF);
        for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
            WorldOS::MemoryMapEntry* entry = (WorldOS::MemoryMapEntry*)((uint64_t)MemoryMap + (i * MEMORY_MAP_ENTRY_SIZE));
            if (entry->type == WorldOS::WORLDOS_MEMORY_ACPI_NVS || entry->type == WorldOS::WORLDOS_MEMORY_ACPI_RECLAIMABLE) {
                uint64_t page_length = (((entry->length % 4096) > 0) ? ((entry->length / 4096) + 1) : (entry->length / 4096));
                ReservePages((void*)entry->Address, page_length);
            }
        }
        kernel_size += 4095; // ensure none of the kernel isn't accounted for
        LockPages(kernel_virt_start, kernel_size / 4096);
        ReservePage((void*)0); // reserve the first page
        fb_size += 4095;
        LockPages(fb_virt, fb_size / 4096);
    }

    void* VirtualPageManager::FindFreePage() {
        return FindFreePages(1);
    }

    void* VirtualPageManager::FindFreePages(uint64_t count) {
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNodeOrHigher(m_FreePagesSizeTree, count);
        if (node == nullptr)
            return nullptr;
        LinkedList::Node* list = (LinkedList::Node*)node->extraData;
        if (list == nullptr)
            return nullptr;
        return (void*)list->data;
    }

    void VirtualPageManager::cleanupRUTree(AVLTree::Node* node, AVLTree::Node* parent) {
        if (node == nullptr) {
            return;
        }
        cleanupRUTree(node->left, node);
        cleanupRUTree(node->right, node);
        if (parent != nullptr) {
            AVLTree::Node* sibling = (parent->left == node) ? parent->right : parent->left;
            if (sibling != nullptr) {
                uint64_t nodeEnd = Internal_getBlockEnd(node->key, node->extraData & ~(1 << 63));
                uint64_t siblingEnd = Internal_getBlockEnd(sibling->key, sibling->extraData & ~(1 << 63));
                bool nodeReserved = (node->extraData & (1 << 63)) >> 63;
                bool siblingReserved = (sibling->extraData & (1 << 63)) >> 63;
                if (nodeEnd == sibling->key && nodeReserved == siblingReserved) {
                    // merge adjacent blocks
                    Internal_mergeRUBlocks(m_ReservedANDUsedPages, parent, node, sibling);
                }
            }
        }
    }

    
    void VirtualPageManager::CleanupFreePagesTree() {
        /*
        Many things here need individual functions for them because they are recursive
        */

        // TODO: add disable interrupts call here
        AVLTree::Node* FreePagesAddressTree;
        Internal_OrganiseFPSTIntoNewAVLTree(FreePagesAddressTree, m_FreePagesSizeTree);
        Internal_cleanFreePagesTree(FreePagesAddressTree, FreePagesAddressTree, nullptr);
        // wipe old free pages tree
        Internal_clearFreePagesTree(m_FreePagesSizeTree);
        // rebuild tree
        Internal_OrganiseNewAVLTreeIntoFPST(m_FreePagesSizeTree, FreePagesAddressTree);
    }



    

    void VirtualPageManager::ReservePage(void* addr) {
        ReservePages(addr, 1);
    }

    void VirtualPageManager::ReservePages(void* addr, uint64_t count) {
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNode(m_ReservedANDUsedPages, (uint64_t)addr);
        if (node != nullptr)
            return; // ignore request if entry with same address already exists
        AVLTree::insert(m_ReservedANDUsedPages, (uint64_t)addr, (count | (1 << 63)));
        m_ReservedPagesCount += count;
        UnfreePages(addr, count);
    }

    void VirtualPageManager::UnreservePage(void* addr) {
        UnreservePages(addr, 1);
    }

    void VirtualPageManager::UnreservePages(void* addr, uint64_t count) {
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNode(m_ReservedANDUsedPages, (uint64_t)addr);
        if (node == nullptr || node->extraData != (count | (1 << 63)))
            return; // ignore request if entry with same address doesn't exist
        AVLTree::deleteNode(m_ReservedANDUsedPages, (uint64_t)addr);
        m_ReservedPagesCount -= count;
        FreePages(addr, count);
    }

    // Allocate 1 memory page
    void* VirtualPageManager::AllocatePage() {
        return AllocatePages(1);
    }

    // Allocate requested amount of pages
    void* VirtualPageManager::AllocatePages(uint64_t count) {
        // START FindFreePages code
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNodeOrHigher(m_FreePagesSizeTree, count);
        if (node == nullptr)
            return nullptr;
        LinkedList::Node* list = (LinkedList::Node*)node->extraData;
        if (list == nullptr)
            return nullptr;
        // END FindFreePages code
        void* mem = (void*)list->data;
        if (node->key > count) {
            Internal_SplitFreeBlock(m_FreePagesSizeTree, mem, node->key, count);
        }
        LockPages(mem, count);
    }

    void VirtualPageManager::UnallocatePage(void* addr) {
        UnallocatePages(addr, 1);
    }

    void VirtualPageManager::UnallocatePages(void* addr, uint64_t count) {
        UnlockPages(addr, count);
    }

    /* Private Methods */

    void VirtualPageManager::LockPage(void* addr) {
        LockPages(addr, 1);
    }

    void VirtualPageManager::LockPages(void* addr, uint64_t count) {
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNode(m_ReservedANDUsedPages, (uint64_t)addr);
        if (node != nullptr)
            return; // ignore request if entry with same address already exists
        AVLTree::insert(m_ReservedANDUsedPages, (uint64_t)addr, (count & ~(1 << 63)));
        m_FreePagesCount += count;
        UnfreePages(addr, count);
    }

    void VirtualPageManager::UnlockPage(void* addr) {
        UnlockPages(addr, 1);
    }

    void VirtualPageManager::UnlockPages(void* addr, uint64_t count) {
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNode(m_ReservedANDUsedPages, (uint64_t)addr);
        if (node == nullptr || node->extraData != (count & ~(1 << 63)))
            return; // ignore request if entry with same address doesn't exist
        AVLTree::deleteNode(m_ReservedANDUsedPages, (uint64_t)addr);
        m_FreePagesCount -= count;
        FreePages(addr, count);
    }

    void VirtualPageManager::FreePage(void* addr) {
        FreePages(addr, 1);
    }

    // Mark pages as free
    void VirtualPageManager::FreePages(void* addr, uint64_t count) {
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNode(m_FreePagesSizeTree, count);
        if (node == nullptr) {
            AVLTree::insert(m_FreePagesSizeTree, count, (uint64_t)nullptr);
            node = AVLTree::findNode(m_FreePagesSizeTree, count);
        }
        LinkedList::Node* list = (LinkedList::Node*)node->extraData;
        LinkedList::insert(list, (uint64_t)addr);
        m_FreePagesCount += count;
    }

    void VirtualPageManager::UnfreePage(void* addr) {
        UnfreePages(addr, 1);
    }

    void VirtualPageManager::UnfreePages(void* addr, uint64_t count) {
        AVLTree::Node* node = nullptr;
        node = AVLTree::findNode(m_FreePagesSizeTree, count);
        if (node == nullptr)
            return; // pages are already not free, so just return
        LinkedList::Node* list = (LinkedList::Node*)node->extraData;
        LinkedList::deleteNode(list, (uint64_t)addr);
        if (list == nullptr)
            AVLTree::deleteNode(m_FreePagesSizeTree, count);
        else
            node->extraData = (uint64_t)list;
        m_FreePagesCount -= count;
    }

    


}
