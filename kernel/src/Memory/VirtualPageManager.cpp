/*
Copyright (©) 2022-2024  Frosty515

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

#include "VirtualPageManager.hpp"


/* Extra Internal Functions */

// Set a node to reserved (sets highest bit of extraData)
void Internal_UsedToReserved(AVLTree::Node* node) {
    uint64_t data = node->extraData;
    data |= (UINT64_C(1) << 63);
    node->extraData = data;
}

// Set a node to used (clears highest bit of extraData)
void Internal_ReservedToUsed(AVLTree::Node* node) {
    uint64_t data = node->extraData;
    data &= ~(UINT64_C(1) << 63);
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
        LinkedList::insertNode(list, (uint64_t)mem + (sizeFound - count) * 4096);
        node->extraData = (uint64_t)list;
    }
    return mem;
}

// Split a Reserved/Used block. addr is original block starting address, count is the page count wanted for the first section.
void Internal_SplitRUBlock(AVLTree::Node* root, void* addr, uint64_t count) {
    AVLTree::Node* node = AVLTree::findNode(root, (uint64_t)addr);
    if (node == nullptr)
        return; // ignore invalid block address
    const bool isReserved = (node->extraData & (UINT64_C(1) << 63)) >> 63; // save state of block
    Internal_ReservedToUsed(node); // clear state of block to make the math easier
    if (count >= node->extraData)
        return; // ignore invalid addr/count combo
    uint64_t upperBlockSize = node->extraData - count;
    node->extraData = count | (isReserved ? (UINT64_C(1) << 63) : 0);
    AVLTree::insert(root, (uint64_t)addr + upperBlockSize * 4096, upperBlockSize | (isReserved ? (UINT64_C(1) << 63) : 0));
}

// Split a Free block. addr is original block starting address, original_count is the original size of the block, new_count is the page count wanted for the first section
AVLTree::Node* Internal_SplitFreeBlock(AVLTree::Node* root, void* addr, uint64_t original_count, uint64_t new_count) {
    if (original_count <= new_count)
        return root; // ignore invalid new_count
    AVLTree::Node* node = AVLTree::findNode(root, original_count);
    if (node == nullptr)
        return root; // ignore invalid size
    LinkedList::Node* list = (LinkedList::Node*)node->extraData;
    if (list == nullptr)
        return root; // this should **NEVER** happen, but is here just in case.
    LinkedList::Node* item = LinkedList::findNode(list, (uint64_t)addr);
    if (item == nullptr || item->data != (uint64_t)addr)
        return root; // ignore invalid addr/size combo
    LinkedList::deleteNode(list, item->data); // delete list entry
    if (list == nullptr)
        AVLTree::deleteNode(root, original_count); // delete old size entry as there are no list entries left
    else
        node->extraData = (uint64_t)list;
    // insert the first section of the block
    node = AVLTree::findNode(root, new_count);
    if (node == nullptr) {
        list = nullptr;
        AVLTree::insert(root, new_count, (uint64_t)nullptr);
        node = AVLTree::findNode(root, new_count);
    }
    else
        list = (LinkedList::Node*)node->extraData;
    LinkedList::insertNode(list, (uint64_t)addr);
    node->extraData = (uint64_t)list;
    // insert the second section of the block
    node = AVLTree::findNode(root, original_count - new_count);
    if (node == nullptr) {
        list = nullptr;
        AVLTree::insert(root, original_count - new_count, (uint64_t)nullptr);
        node = AVLTree::findNode(root, original_count - new_count);
    }
    else
        list = (LinkedList::Node*)node->extraData;
    LinkedList::insertNode(list, (uint64_t)addr + new_count * 4096);
    node->extraData = (uint64_t)list;
    return root;
}

void Internal_mergeRUBlocks(AVLTree::Node* root, AVLTree::Node* parent, AVLTree::Node* node, AVLTree::Node* sibling) {
    bool nodeReserved = (node->extraData & (UINT64_C(1) << 63)) >> 63;
    bool siblingReserved = (sibling->extraData & (UINT64_C(1) << 63)) >> 63;
    uint64_t nodeSize = node->extraData & ~(UINT64_C(1) << 63);
    uint64_t siblingSize = sibling->extraData & ~(UINT64_C(1) << 63);
    uint64_t totalSize = nodeSize + siblingSize;

    // check if both nodes are either reserved or used
    if (nodeReserved == siblingReserved) {
        // update parent node with new combined node
        node->extraData = totalSize | (nodeReserved ? (UINT64_C(1) << 63) : 0);
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
            if (nodeEnd == sibling->key) {
                // merge adjacent blocks
                Internal_mergeFreeBlocks(root, parent, node, sibling);
            }
        }
    }
}

void Internal_clearFreePagesSizeTree(AVLTree::Node* root) {
    while (root != nullptr) {// due to auto-balancing, this will clear the tree
        LinkedList::Node* list = (LinkedList::Node*)root->extraData;
        while (list != nullptr) {
            using namespace LinkedList;
            deleteNode(list, list); // will automatically assign list to the next node in line
        }
        AVLTree::deleteNode(root, root->key);
    }
}

void Internal_clearUsedReservedPagesTree(AVLTree::Node* root) {
    while (root != nullptr) // due to auto-balancing, this will clear the tree
        AVLTree::deleteNode(root, root->key);
}

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
    LinkedList::insertNode(list, root->key);
}


/* Virtual Page Manager Class */

/* Public Methods */

VirtualPageManager::VirtualPageManager() : m_FreePagesCount(0), m_FreePagesSizeTree(nullptr), m_ReservedANDUsedPages(nullptr), m_ReservedPagesCount(0), m_UsedPagesCount(0), m_region(), m_FreePagesSizeTreeLock(0), m_ReservedANDUsedPagesLock(0), m_GlobalLock(0) {
    // do nothing. just here so the compiler doesn't complain
}

VirtualPageManager::~VirtualPageManager() {
    spinlock_acquire(&m_FreePagesSizeTreeLock);
    Internal_clearFreePagesSizeTree(m_FreePagesSizeTree);
    spinlock_release(&m_FreePagesSizeTreeLock);
    spinlock_acquire(&m_ReservedANDUsedPagesLock);
    Internal_clearUsedReservedPagesTree(m_ReservedANDUsedPages);
    spinlock_release(&m_ReservedANDUsedPagesLock);
}

void VirtualPageManager::InitVPageMgr(MemoryMapEntry** MemoryMap, uint64_t MemoryMapEntryCount, void* kernel_virt_start, size_t kernel_size, void* fb_virt, uint64_t fb_size, const VirtualRegion& region) {
    if (!AVLTree::NodePool_HasBeenInitialised())
        AVLTree::NodePool_Init();
    if (!LinkedList::NodePool_HasBeenInitialised())
        LinkedList::NodePool_Init();
    m_region = region;
    FreePages(m_region.GetStart(), m_region.GetSize() >> 12);
    for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
        MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)MemoryMap + (i * MEMORY_MAP_ENTRY_SIZE));
        if ((entry->type == WORLDOS_MEMORY_ACPI_NVS || entry->type == WORLDOS_MEMORY_ACPI_RECLAIMABLE) && m_region.IsInside((void*)(entry->Address), entry->length)) {
            uint64_t page_length = (((entry->length % 4096) > 0) ? ((entry->length / 4096) + 1) : (entry->length / 4096));
            ReservePages((void*)entry->Address, page_length);
        }
    }
    if (m_region.IsInside(kernel_virt_start, kernel_size)) {
        kernel_size += 4095; // ensure none of the kernel isn't accounted for
        LockPages(kernel_virt_start, kernel_size / 4096);
    }
    if (m_region.IsInside(fb_virt, fb_size)) {
        fb_size += 4095;
        LockPages(fb_virt, fb_size / 4096);
    }
}

void VirtualPageManager::InitVPageMgr(const VirtualRegion& region) {
    if (!AVLTree::NodePool_HasBeenInitialised())
        AVLTree::NodePool_Init();
    if (!LinkedList::NodePool_HasBeenInitialised())
        LinkedList::NodePool_Init();
    m_region = region;
    FreePages(m_region.GetStart(), m_region.GetSize() >> 12);
}

void* VirtualPageManager::FindFreePage() {
    return FindFreePages(1);
}

void* VirtualPageManager::FindFreePages(uint64_t count) {
    spinlock_acquire(&m_FreePagesSizeTreeLock);
    AVLTree::Node* node = nullptr;
    node = AVLTree::findNodeOrHigher(m_FreePagesSizeTree, count);
    if (node == nullptr) {
        spinlock_release(&m_FreePagesSizeTreeLock);
        return nullptr;
    }
    LinkedList::Node* list = (LinkedList::Node*)node->extraData;
    if (list == nullptr) {
        spinlock_release(&m_FreePagesSizeTreeLock);
        return nullptr;
    }
    void* mem = (void*)list->data;
    spinlock_release(&m_FreePagesSizeTreeLock);
    return mem;
}

void VirtualPageManager::cleanupRUTree(AVLTree::Node* node, AVLTree::Node* parent) {
    if (node == nullptr) {
        return;
    }
    spinlock_acquire(&m_ReservedANDUsedPagesLock);
    do_cleanupRUTree(node, parent);
    spinlock_release(&m_ReservedANDUsedPagesLock);
}

void VirtualPageManager::do_cleanupRUTree(AVLTree::Node* node, AVLTree::Node* parent) {
    if (node == nullptr) {
        return;
    }
    cleanupRUTree(node->left, node);
    cleanupRUTree(node->right, node);
    if (parent != nullptr) {
        AVLTree::Node* sibling = (parent->left == node) ? parent->right : parent->left;
        if (sibling != nullptr) {
            uint64_t nodeEnd = Internal_getBlockEnd(node->key, node->extraData & ~(UINT64_C(1) << 63));
            bool nodeReserved = (node->extraData & (UINT64_C(1) << 63)) >> 63;
            bool siblingReserved = (sibling->extraData & (UINT64_C(1) << 63)) >> 63;
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
    spinlock_acquire(&m_FreePagesSizeTreeLock);
    // TODO: add disable interrupts call here
    AVLTree::Node* FreePagesAddressTree = nullptr;
    Internal_OrganiseFPSTIntoNewAVLTree(FreePagesAddressTree, m_FreePagesSizeTree);
    Internal_cleanFreePagesTree(FreePagesAddressTree, FreePagesAddressTree, nullptr);
    // wipe old free pages tree
    Internal_clearFreePagesSizeTree(m_FreePagesSizeTree);
    // rebuild tree
    Internal_OrganiseNewAVLTreeIntoFPST(m_FreePagesSizeTree, FreePagesAddressTree);
    spinlock_release(&m_FreePagesSizeTreeLock);
}

void VirtualPageManager::ReservePage(void* addr) {
    ReservePages(addr, 1);
}

void VirtualPageManager::ReservePages(void* addr, uint64_t count) {
    size_t length = count << 12;
    spinlock_acquire(&m_RegionLock);
    if (!m_region.EnsureIsInside(addr, length)) {
        spinlock_release(&m_RegionLock);
        return;
    }
    spinlock_release(&m_RegionLock);
    count = length >> 12;
    uint64_t addr_i = (uint64_t)addr;
    if (count == 0)
        return;
    spinlock_acquire(&m_ReservedANDUsedPagesLock);
    AVLTree::Node* node = nullptr;
    node = AVLTree::findNode(m_ReservedANDUsedPages, addr_i);
    if (node != nullptr) {
        spinlock_release(&m_ReservedANDUsedPagesLock);
        return; // ignore request if entry with same address already exists
    }
    AVLTree::insert(m_ReservedANDUsedPages, addr_i, (count | (UINT64_C(1) << 63)));
    spinlock_release(&m_ReservedANDUsedPagesLock);
    spinlock_acquire(&m_GlobalLock);
    m_ReservedPagesCount += count;
    spinlock_release(&m_GlobalLock);
    (void)UnfreePages(addr, count);
}

void VirtualPageManager::UnreservePage(void* addr) {
    UnreservePages(addr, 1);
}

void VirtualPageManager::UnreservePages(void* addr, uint64_t count) {
    size_t length = count << 12;
    spinlock_acquire(&m_RegionLock);
    if (!m_region.EnsureIsInside(addr, length)) {
        spinlock_release(&m_RegionLock);
        return;
    }
    spinlock_release(&m_RegionLock);
    if (count == 0)
        return;
    count = length >> 12;
    uint64_t addr_i = (uint64_t)addr;
    spinlock_acquire(&m_ReservedANDUsedPagesLock);
    AVLTree::Node* node = nullptr;
    node = AVLTree::findNode(m_ReservedANDUsedPages, addr_i);
    if (node == nullptr || node->extraData != (count | (UINT64_C(1) << 63))) {
        spinlock_release(&m_ReservedANDUsedPagesLock);
        return; // ignore request if entry with same address doesn't exist
    }
    AVLTree::deleteNode(m_ReservedANDUsedPages, addr_i);
    spinlock_release(&m_ReservedANDUsedPagesLock);
    spinlock_acquire(&m_GlobalLock);
    m_ReservedPagesCount -= count;
    spinlock_release(&m_GlobalLock);
    FreePages((void*)addr_i, count);
}

// Allocate 1 memory page
void* VirtualPageManager::AllocatePage() {
    return AllocatePages(1);
}

// Allocate requested amount of pages
void* VirtualPageManager::AllocatePages(uint64_t count) {
    if (count == 0)
        return nullptr;
    // START FindFreePages code
    spinlock_acquire(&m_FreePagesSizeTreeLock);
    AVLTree::Node* node = nullptr;
    node = AVLTree::findNodeOrHigher(m_FreePagesSizeTree, count);
    if (node == nullptr) {
        spinlock_release(&m_FreePagesSizeTreeLock);
        return nullptr;
    }
    LinkedList::Node* list = (LinkedList::Node*)node->extraData;
    if (list == nullptr) {
        spinlock_release(&m_FreePagesSizeTreeLock);
        return nullptr;
    }
    // END FindFreePages code
    void* mem = (void*)list->data;
    if (node->key > count)
        m_FreePagesSizeTree = Internal_SplitFreeBlock(m_FreePagesSizeTree, mem, node->key, count);
    spinlock_release(&m_FreePagesSizeTreeLock);
    LockPages(mem, count, true, false); // checks have already been performed and there should be an exact block match
    return mem;
}

// Allocate 1 memory page at requested address
void* VirtualPageManager::AllocatePage(void* addr) {
    return AllocatePages(addr, 1);
}

// Allocate requested amount of pages at requested address. Returns nullptr if address is invalid.
void* VirtualPageManager::AllocatePages(void* addr, uint64_t count) {
    spinlock_acquire(&m_RegionLock);
    if (count == 0 || !m_region.IsInside(addr, count << 12)) {
        spinlock_release(&m_RegionLock);
        return nullptr;
    }
    spinlock_release(&m_RegionLock);
    if (!UnfreePages(addr, count))
        return nullptr;
    LockPages(addr, count, false, false); // no need to check as everything is already safe
    return addr;
}

void VirtualPageManager::UnallocatePage(void* addr) {
    UnallocatePages(addr, 1);
}

void VirtualPageManager::UnallocatePages(void* addr, uint64_t count) {
    UnlockPages(addr, count);
}

const VirtualRegion& VirtualPageManager::GetVirtualRegion() const {
    return m_region;
}

bool VirtualPageManager::AttemptToExpandRight(size_t new_size) {
    spinlock_acquire(&m_RegionLock);
    if (new_size < m_region.GetSize()) {
        spinlock_release(&m_RegionLock);
        return false;
    }
    void* old_end = m_region.GetEnd();
    uint64_t extra_count = (new_size - m_region.GetSize()) >> 12;
    m_region.ExpandRight(new_size);
    spinlock_release(&m_RegionLock);
    FreePages(old_end, extra_count);
    return true;
}

/* Private Methods */

void VirtualPageManager::LockPage(void* addr, bool unfree, bool check) {
    LockPages(addr, 1, unfree, check);
}

void VirtualPageManager::LockPages(void* addr, uint64_t count, bool unfree, bool check) {
    if (check) {
        size_t length = count << 12;
        spinlock_acquire(&m_RegionLock);
        if (!m_region.EnsureIsInside(addr, length)) {
            spinlock_release(&m_RegionLock);
            return;
        }
        spinlock_release(&m_RegionLock);
        count = length >> 12;
    }
    uint64_t addr_i = (uint64_t)addr;
    if (count == 0)
        return;
    spinlock_acquire(&m_ReservedANDUsedPagesLock);
    AVLTree::Node* node = nullptr;
    node = AVLTree::findNode(m_ReservedANDUsedPages, addr_i);
    if (node != nullptr) {
        spinlock_release(&m_ReservedANDUsedPagesLock);
        return; // ignore request if entry with same address already exists
    }
    AVLTree::insert(m_ReservedANDUsedPages, addr_i, (count & ~(UINT64_C(1) << 63)));
    spinlock_release(&m_ReservedANDUsedPagesLock);
    spinlock_acquire(&m_GlobalLock);
    m_UsedPagesCount += count;
    spinlock_release(&m_GlobalLock);
    if (unfree)
        (void)UnfreePages((void*)addr_i, count, !check);
}

void VirtualPageManager::UnlockPage(void* addr) {
    UnlockPages(addr, 1);
}

void VirtualPageManager::UnlockPages(void* addr, uint64_t count) {
    size_t length = count << 12;
    spinlock_acquire(&m_RegionLock);
    if (!m_region.EnsureIsInside(addr, length)) {
        spinlock_release(&m_RegionLock);
        return;
    }
    spinlock_release(&m_RegionLock);
    count = length >> 12;
    uint64_t addr_i = (uint64_t)addr;
    if (count == 0)
        return;
    spinlock_acquire(&m_ReservedANDUsedPagesLock);
    AVLTree::Node* node = nullptr;
    node = AVLTree::findNode(m_ReservedANDUsedPages, addr_i);
    if (node == nullptr || node->extraData != (count & ~(UINT64_C(1) << 63))) {
        spinlock_release(&m_ReservedANDUsedPagesLock);
        return; // ignore request if entry with same address doesn't exist
    }
    AVLTree::deleteNode(m_ReservedANDUsedPages, addr_i);
    spinlock_release(&m_ReservedANDUsedPagesLock);
    spinlock_acquire(&m_GlobalLock);
    m_FreePagesCount -= count;
    spinlock_release(&m_GlobalLock);
    FreePages((void*)addr_i, count);
}

void VirtualPageManager::FreePage(void* addr) {
    FreePages(addr, 1);
}

// Mark pages as free
void VirtualPageManager::FreePages(void* addr, uint64_t count) {
    size_t length = count << 12;
    spinlock_acquire(&m_RegionLock);
    if (!m_region.EnsureIsInside(addr, length)) {
        spinlock_release(&m_RegionLock);
        return;
    }
    spinlock_release(&m_RegionLock);
    count = length >> 12;
    if (count == 0)
        return;
    uint64_t addr_i = (uint64_t)addr;
    spinlock_acquire(&m_FreePagesSizeTreeLock);
    AVLTree::Node* node = nullptr;
    node = AVLTree::findNode(m_FreePagesSizeTree, count);
    if (node == nullptr) {
        AVLTree::insert(m_FreePagesSizeTree, count, (uint64_t)nullptr);
        node = AVLTree::findNode(m_FreePagesSizeTree, count);
    }
    LinkedList::Node* list = (LinkedList::Node*)(node->extraData);
    LinkedList::insertNode(list, addr_i);
    node->extraData = (uint64_t)list;
    spinlock_release(&m_FreePagesSizeTreeLock);
    spinlock_acquire(&m_GlobalLock);
    m_FreePagesCount += count;
    spinlock_release(&m_GlobalLock);
}

bool VirtualPageManager::UnfreePage(void* addr, bool exact) {
    return UnfreePages(addr, 1, exact);
}

bool VirtualPageManager::UnfreePages(void* addr, uint64_t count, bool exact) {
    size_t length = count << 12;
    spinlock_acquire(&m_RegionLock);
    if (!m_region.EnsureIsInside(addr, length)) {
        spinlock_release(&m_RegionLock);
        return false;
    }
    spinlock_release(&m_RegionLock);
    count = length >> 12;
    uint64_t addr_i = (uint64_t)addr;
    if (count == 0)
        return false;
    if (exact) { // must exactly match. no splitting allowed
        spinlock_acquire(&m_FreePagesSizeTreeLock);
        AVLTree::Node* node = AVLTree::findNode(m_FreePagesSizeTree, count);
        if (node == nullptr) {
            spinlock_release(&m_FreePagesSizeTreeLock);
            return false;
        }
        LinkedList::Node* list = (LinkedList::Node*)(node->extraData);
        if (list == nullptr) {
            spinlock_release(&m_FreePagesSizeTreeLock);
            return false;
        }
        LinkedList::Node* sub_node = LinkedList::findNode(list, addr_i);
        if (sub_node == nullptr) {
            spinlock_release(&m_FreePagesSizeTreeLock);
            return false;
        }
        LinkedList::deleteNode(list, sub_node);
        if (list == nullptr)
            AVLTree::deleteNode(m_FreePagesSizeTree, node->key);
        else
            node->extraData = (uint64_t)list;
        spinlock_release(&m_FreePagesSizeTreeLock);
        spinlock_acquire(&m_GlobalLock);
        m_FreePagesCount -= count;
        spinlock_release(&m_GlobalLock);
        return true;
    }
    spinlock_acquire(&m_FreePagesSizeTreeLock);
    AVLTree::Node* node = AVLTree::findNodeOrHigher(m_FreePagesSizeTree, count);
    if (node == nullptr) {
        spinlock_release(&m_FreePagesSizeTreeLock);
        return false; // no free regions large enough
    }
    uint64_t i_count = count;
    bool found = false;
    bool correct = false;
    while (node != nullptr && !found) {
        LinkedList::Node* list = (LinkedList::Node*)node->extraData;
        LinkedList::Node* sub_node = LinkedList::findNode(list, addr_i);
        if (sub_node == nullptr) {
            VirtualRegion region;
            sub_node = list;
            while (sub_node != nullptr) {
                using namespace LinkedList;
                region = VirtualRegion((void*)(sub_node->data), node->key << 12);
                AVLTree::Node* new_node = node;
                if (region.IsInside(addr, length)) {
                    if (addr > region.GetStart()) {
                        m_FreePagesSizeTree = Internal_SplitFreeBlock(m_FreePagesSizeTree, region.GetStart(), node->key, (addr_i - sub_node->data) >> 12);
                        region.SetStart(addr);
                        new_node = AVLTree::findNodeOrHigher(m_FreePagesSizeTree, region.GetSize() >> 12);
                    }

                    if (length < region.GetSize()) {
                        
                        m_FreePagesSizeTree = Internal_SplitFreeBlock(m_FreePagesSizeTree, region.GetStart(), new_node->key, count);
                        region.ExpandRight(region.GetSize() - length);
                    }
                    found = true;
                    break;
                }
                sub_node = sub_node->next;
            }
        }
        else { // perfect address match
            found = true;
            if (node->key > count)
                m_FreePagesSizeTree = Internal_SplitFreeBlock(m_FreePagesSizeTree, addr, node->key, count);
            else { // perfect match
                LinkedList::deleteNode(list, sub_node);
                if (list == nullptr)
                    AVLTree::deleteNode(m_FreePagesSizeTree, node->key);
                else
                    node->extraData = (uint64_t)list;
                correct = true;
            }
            break;
        }
        i_count = node->key + 1;
        node = AVLTree::findNodeOrHigher(m_FreePagesSizeTree, i_count);
    }
    if (!correct && found) {
        node = AVLTree::findNode(m_FreePagesSizeTree, count);
        if (node == nullptr) {
            spinlock_release(&m_FreePagesSizeTreeLock);
            return false;
        }
        LinkedList::Node* list = (LinkedList::Node*)(node->extraData);
        if (list == nullptr) {
            spinlock_release(&m_FreePagesSizeTreeLock);
            return false;
        }
        LinkedList::Node* sub_node = LinkedList::findNode(list, addr_i);
        if (sub_node == nullptr) { // we should have found and split the block by now
            spinlock_release(&m_FreePagesSizeTreeLock);
            return false;
        }
        LinkedList::deleteNode(list, sub_node);
        if (list == nullptr)
            AVLTree::deleteNode(m_FreePagesSizeTree, node->key);
        else
            node->extraData = (uint64_t)list;
        correct = true;
    }
    spinlock_release(&m_FreePagesSizeTreeLock);
    if (found && correct) {
        spinlock_acquire(&m_GlobalLock);
        m_FreePagesCount -= count;
        spinlock_release(&m_GlobalLock);
    }
    return found && correct;
}

VirtualPageManager* g_VPM = nullptr;
VirtualPageManager* g_KVPM = nullptr;
