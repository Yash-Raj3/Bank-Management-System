#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

template <typename ItemType>
class DynamicArray {
public:
    DynamicArray();
    DynamicArray(const DynamicArray& other);
    ~DynamicArray();
    DynamicArray& operator=(const DynamicArray& other);

    void add(const ItemType& item);
    void removeAt(int index);
    void clear();
    int size() const;

    ItemType& operator[](int index);
    const ItemType& operator[](int index) const;

private:
    ItemType* items;
    int itemCount;
    int capacity;

    void growCapacity();
};

#endif
