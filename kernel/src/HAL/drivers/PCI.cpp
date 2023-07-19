#include "PCI.hpp"

#include <stdio.hpp>

#include <Data-structures/LinkedList.hpp>

#include <Memory/PagingUtil.hpp>

namespace PCI {

    void EnumerateFunctions(void* device_addr) {
        Header0* device_header = (Header0*)device_addr;
        if (device_header->ch.DeviceID == 0 || device_header->ch.DeviceID == 0xFFFF) return;

        for (int i = 0; i < 8; i++) {
            Header0* function_header = (Header0*)((uint64_t)device_addr + (i << 12));
            if (function_header->ch.DeviceID == 0 || function_header->ch.DeviceID == 0xFFFF) continue;
            PCIDeviceList::AddPCIDevice(function_header);
        }
    }

    void EnumerateDevices(void* bus_addr) {
        Header0* bus_header = (Header0*)bus_addr;
        if (bus_header->ch.DeviceID == 0 || bus_header->ch.DeviceID == 0xFFFF) return;

        for (int i = 0; i < 32; i++)
            EnumerateFunctions((void*)((uint64_t)bus_addr + (i << 15)));
    }

    void EnumerateBuses(void* segment_addr) {
        for (int i = 0; i < 256; i++)
            EnumerateDevices((void*)((uint64_t)segment_addr + (i << 20)));
    }

    namespace PCIDeviceList {
        LinkedList::Node* g_deviceList;

        Header0* GetPCIDevice(uint64_t index) {
            LinkedList::Node* node = g_deviceList;
            for (uint64_t i = 0; i <= index; i++) {
                if (node == nullptr)
                    return nullptr; // Invalid index
                node = node->next;
            }
            if (node == nullptr)
                return nullptr; // Invalid index
            return (Header0*)(node->data);
        }

        void AddPCIDevice(Header0* device) {
            LinkedList::insertNode(g_deviceList, (uint64_t)to_HHDM(device));
        }

        void RemovePCIDevice(uint64_t index) {
            Header0* device = GetPCIDevice(index);
            if (device == nullptr)
                return;
            LinkedList::deleteNode(g_deviceList, (uint64_t)device);
        }
    }

}