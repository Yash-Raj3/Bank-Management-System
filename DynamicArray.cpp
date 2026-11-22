#include "DynamicArray.h"

#include "Account.h"
#include "Models.h"
#include "String.h"

template <typename ItemType>
DynamicArray<ItemType>::DynamicArray() : items(nullptr), itemCount(0), capacity(0) {}

template <typename ItemType>
DynamicArray<ItemType>::DynamicArray(const DynamicArray& other) : items(nullptr), itemCount(0), capacity(0) {
    for (int index = 0; index < other.itemCount; ++index) {
        add(other.items[index]);
    }
}

template <typename ItemType>
DynamicArray<ItemType>::~DynamicArray() {
    delete[] items;
}

template <typename ItemType>
DynamicArray<ItemType>& DynamicArray<ItemType>::operator=(const DynamicArray& other) {
    if (this != &other) {
        clear();
        for (int index = 0; index < other.itemCount; ++index) {
            add(other.items[index]);
        }
    }
    return *this;
}

template <typename ItemType>
void DynamicArray<ItemType>::growCapacity() {
    int newCapacity = capacity == 0 ? 8 : capacity * 2;
    ItemType* newItems = new ItemType[newCapacity];
    for (int index = 0; index < itemCount; ++index) {
        newItems[index] = items[index];
    }
    delete[] items;
    items = newItems;
    capacity = newCapacity;
}

template <typename ItemType>
void DynamicArray<ItemType>::add(const ItemType& item) {
    if (itemCount == capacity) {
        growCapacity();
    }
    items[itemCount++] = item;
}

template <typename ItemType>
void DynamicArray<ItemType>::removeAt(int index) {
    for (int current = index; current + 1 < itemCount; ++current) {
        items[current] = items[current + 1];
    }
    --itemCount;
}

template <typename ItemType>
void DynamicArray<ItemType>::clear() {
    delete[] items;
    items = nullptr;
    itemCount = 0;
    capacity = 0;
}

template <typename ItemType>
int DynamicArray<ItemType>::size() const {
    return itemCount;
}

template <typename ItemType>
ItemType& DynamicArray<ItemType>::operator[](int index) {
    return items[index];
}

template <typename ItemType>
const ItemType& DynamicArray<ItemType>::operator[](int index) const {
    return items[index];
}

template class DynamicArray<int>;
template class DynamicArray<String>;
template class DynamicArray<Account*>;
template class DynamicArray<User*>;
template class DynamicArray<Transaction>;
template class DynamicArray<Loan>;
template class DynamicArray<ServiceToken>;
template class DynamicArray<AuditEntry>;
template class DynamicArray<TransferLink>;
template class DynamicArray<HeapItem>;
