#ifndef ACCOUNT_AVL_TREE_H
#define ACCOUNT_AVL_TREE_H

#include "Account.h"
#include "DynamicArray.h"

class AccountAVLTree {
public:
    AccountAVLTree();
    ~AccountAVLTree();
    AccountAVLTree(const AccountAVLTree&) = delete;
    AccountAVLTree& operator=(const AccountAVLTree&) = delete;

    bool insert(Account* account);
    bool remove(int accountNumber);
    Account* find(int accountNumber) const;
    DynamicArray<Account*> collectInOrder() const;
    int size() const;
    void clear();

private:
    class Node {
    public:
        Account* account;
        Node* left;
        Node* right;
        int height;
    };

    Node* root;
    int accountCount;

    static int heightOf(Node* node);
    static void updateHeight(Node* node);
    static int balanceFactorOf(Node* node);
    static Node* rotateRight(Node* unbalancedNode);
    static Node* rotateLeft(Node* unbalancedNode);
    static Node* rebalance(Node* node);
    static Node* insertNode(Node* node, Account* account, bool& wasInserted);
    static Node* removeNode(Node* node, int accountNumber, bool& wasRemoved);
    static Node* findMinimumNode(Node* node);
    static void collectNodes(Node* node, DynamicArray<Account*>& collectedAccounts);
    static void destroySubtree(Node* node);
};

#endif
