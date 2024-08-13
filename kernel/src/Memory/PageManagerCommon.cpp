#include "PageManager.hpp"

#include "newdelete.hpp"
#include "kmalloc.hpp"


bool PageManager::isReadable(void* addr, size_t size) const {
    spinlock_acquire(&m_lock);
    bool result = isReadableImpl(addr, size);
    spinlock_release(&m_lock);
    return result;
}

bool PageManager::isWritable(void* addr, size_t size) const {
    spinlock_acquire(&m_lock);
    bool result = isWritableImpl(addr, size);
    spinlock_release(&m_lock);
    return result;
}

bool PageManager::isValidAllocation(void* addr, size_t size) const {
    PageObject* obj = FindObject(addr);
    if (obj == nullptr)
        return false;
    return (obj->page_count * PAGE_SIZE) == size;
}

PagePermissions PageManager::GetPermissions(void* addr) const {
    PageObject* obj = FindObject(addr);
    if (obj == nullptr)
        return PagePermissions::READ;
    return obj->perms;
}

PageObject* PageManager::CreateObject(const PageObject& obj) {
    if (NewDeleteInitialised())
        return new PageObject(obj);
    else {
        m_page_object_pool_used = true;
        PageObject* po = PageObjectPool_Allocate();
        *po = obj;
        return po;
    }
}

PageObject* PageManager::CreateObject() {
    return (PageObject*)kcalloc_vmm(1, sizeof(PageObject));
}

void PageManager::DeleteObject(const PageObject* obj) {
    kfree_vmm((void*)obj);
}

bool PageManager::InsertObject(PageObject* obj) { // it is assumed that the lock is already acquired, as this is a private function
    if (obj == nullptr)
        return false;
    m_allocated_objects.insert(obj->virtual_address, obj);
    m_allocated_object_count++;
    return true;
}

bool PageManager::RemoveObject(PageObject* obj) { // it is assumed that the lock is already acquired, as this is a private function
    if (obj == nullptr)
        return false;
    m_allocated_objects.remove(obj->virtual_address);
    m_allocated_object_count--;
    return true;
}

void PageManager::EnumerateObjects(bool (*callback)(PageObject* obj, void* data), void* data) const {
    AVLTree::Node* current = m_allocated_objects.GetRoot();
    AVLTree::Node* pre = nullptr;

    while (current != nullptr) {
        if (current->left == nullptr) {
            if (!callback((PageObject*)current->extraData, data))
                break; // Stop if callback returns false
            current = current->right; // Move to next right node
        }
        else {
            // Find the inorder predecessor of current
            pre = current->left;
            while (pre->right != nullptr && pre->right != current)
                pre = pre->right;

            // Make current as the right child of its inorder predecessor
            if (pre->right == nullptr) {
                pre->right = current;
                current = current->left;
            }
            else {
                // Revert the changes made in the 'if' part to restore the original tree
                // i.e., fix the right child of predecessor
                pre->right = nullptr;
                if (!callback((PageObject*)current->extraData, data))
                    break; // Stop if callback returns false
                current = current->right;
            }
        }
    }
}

PageObject* PageManager::FindObject(void* addr) const {
    return m_allocated_objects.find(addr);
}

PageObject* PageManager::FindObject(const VirtualRegion& region) const {
    return FindObjectImpl(m_allocated_objects.GetRoot(), region.GetStart(), region.GetSize());
}

PageObject* PageManager::FindObjectImpl(AVLTree::Node const* root, void* base, size_t size) const {
    // can't do a simple find because we are working with a region inside an object, not the starting address
    uint64_t key = (uint64_t)base;

    if (root == nullptr)
        return nullptr;

    PageObject* obj = (PageObject*)root->extraData;
    if ((uint64_t)obj->virtual_address <= key && ((uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE) >= (key + size))
        return obj;

    if (key < (uint64_t)root->key) {
        void* ret = FindObjectImpl(root->left, base, size);
        if (ret != nullptr)
            return (PageObject*)ret;
        if ((uint64_t)root->key > (key + size))
            return nullptr;
        // could be in the right subtree
        return FindObjectImpl(root->right, base, size);
    }
    else {
        void* ret = FindObjectImpl(root->right, base, size);
        if (ret != nullptr)
            return (PageObject*)ret;
        if ((uint64_t)root->key < key)
            return nullptr;
        // could be in the left subtree
        return FindObjectImpl(root->left, base, size);
    }

    return nullptr;
}

bool PageManager::isReadableImpl(void* addr, size_t size) const {
    PageObject* obj = FindObject(VirtualRegion(addr, PAGE_SIZE));
    if (obj == nullptr)
        return false; // not allocated
    if (obj->perms == PagePermissions::READ || obj->perms == PagePermissions::READ_WRITE || obj->perms == PagePermissions::READ_EXECUTE) {
        uint64_t new_size = size - obj->page_count * PAGE_SIZE - ((uint64_t)addr - (uint64_t)obj->virtual_address);
        if (new_size == 0)
            return true;
        return isReadableImpl((void*)((uint64_t)addr + obj->page_count * PAGE_SIZE), new_size);
    }
    else
        return false;
}

bool PageManager::isWritableImpl(void* addr, size_t size) const {
    PageObject* obj = FindObject(VirtualRegion(addr, PAGE_SIZE));
    if (obj == nullptr)
        return false; // not allocated
    if (obj->perms == PagePermissions::WRITE || obj->perms == PagePermissions::READ_WRITE) {
        uint64_t new_size = size - obj->page_count * PAGE_SIZE - ((uint64_t)addr - (uint64_t)obj->virtual_address);
        if (new_size == 0)
            return true;
        return isWritableImpl((void*)((uint64_t)addr + obj->page_count * PAGE_SIZE), new_size);
    }
    else
        return false;
}

void PageManager::PrintRegions(fd_t fd) const {
    //spinlock_acquire(&m_lock);
    EnumerateObjects([](PageObject* po, void* data) {
        fd_t* temp = (fd_t*)data;
        fd_t fd = *temp;
        if (po->flags & PO_ALLOCATED) {
            if (po->flags & PO_USER)
                fputs(fd, "User ");
            else
                fputs(fd, "Supervisor ");
            if (po->flags & PO_INUSE)
                fputs(fd, "In use ");
            else if (po->flags & PO_STANDBY)
                fputs(fd, "Standby ");
            if (po->perms == PagePermissions::READ)
                fputs(fd, "Read ");
            else if (po->perms == PagePermissions::WRITE)
                fputs(fd, "Write ");
            else if (po->perms == PagePermissions::EXECUTE)
                fputs(fd, "Execute ");
            else if (po->perms == PagePermissions::READ_WRITE)
                fputs(fd, "Read/Write ");
            else if (po->perms == PagePermissions::READ_EXECUTE)
                fputs(fd, "Read/Execute ");
            fprintf(fd, "0x%016llX - 0x%016llX\n", (uint64_t)po->virtual_address, (uint64_t)po->virtual_address + po->page_count * PAGE_SIZE);
        }
        return true;
    }, &fd);
    //spinlock_release(&m_lock);
}