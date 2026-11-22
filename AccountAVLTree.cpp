#include "AccountAVLTree.h"

AccountAVLTree::AccountAVLTree() : root(nullptr), accountCount(0) {}

AccountAVLTree::~AccountAVLTree() {
    clear();
}

int AccountAVLTree::heightOf(Node* node) {
    return node != nullptr ? node->height : 0;
}

void AccountAVLTree::updateHeight(Node* node) {
    int leftHeight = heightOf(node->left);
    int rightHeight = heightOf(node->right);
    node->height = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

int AccountAVLTree::balanceFactorOf(Node* node) {
    return node != nullptr ? heightOf(node->left) - heightOf(node->right) : 0;
}

AccountAVLTree::Node* AccountAVLTree::rotateRight(Node* unbalancedNode) {
    Node* newRoot = unbalancedNode->left;
    unbalancedNode->left = newRoot->right;
    newRoot->right = unbalancedNode;
    updateHeight(unbalancedNode);
    updateHeight(newRoot);
    return newRoot;
}

AccountAVLTree::Node* AccountAVLTree::rotateLeft(Node* unbalancedNode) {
    Node* newRoot = unbalancedNode->right;
    unbalancedNode->right = newRoot->left;
    newRoot->left = unbalancedNode;
    updateHeight(unbalancedNode);
    updateHeight(newRoot);
    return newRoot;
}

AccountAVLTree::Node* AccountAVLTree::rebalance(Node* node) {
    updateHeight(node);
    int balanceFactor = balanceFactorOf(node);
    if (balanceFactor > 1) {
        if (balanceFactorOf(node->left) < 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }
    if (balanceFactor < -1) {
        if (balanceFactorOf(node->right) > 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }
    return node;
}

AccountAVLTree::Node* AccountAVLTree::insertNode(Node* node, Account* account, bool& wasInserted) {
    if (node == nullptr) {
        wasInserted = true;
        return new Node{account, nullptr, nullptr, 1};
    }
    if (account->accountNumber < node->account->accountNumber) {
        node->left = insertNode(node->left, account, wasInserted);
    } else if (account->accountNumber > node->account->accountNumber) {
        node->right = insertNode(node->right, account, wasInserted);
    } else {
        return node;
    }
    return rebalance(node);
}

AccountAVLTree::Node* AccountAVLTree::findMinimumNode(Node* node) {
    while (node->left != nullptr) {
        node = node->left;
    }
    return node;
}

AccountAVLTree::Node* AccountAVLTree::removeNode(Node* node, int accountNumber, bool& wasRemoved) {
    if (node == nullptr) {
        return nullptr;
    }
    if (accountNumber < node->account->accountNumber) {
        node->left = removeNode(node->left, accountNumber, wasRemoved);
    } else if (accountNumber > node->account->accountNumber) {
        node->right = removeNode(node->right, accountNumber, wasRemoved);
    } else {
        wasRemoved = true;
        if (node->left == nullptr || node->right == nullptr) {
            Node* onlyChild = node->left != nullptr ? node->left : node->right;
            delete node;
            return onlyChild;
        }
        Node* successorNode = findMinimumNode(node->right);
        node->account = successorNode->account;
        bool ignoredFlag = false;
        node->right = removeNode(node->right, successorNode->account->accountNumber, ignoredFlag);
    }
    return rebalance(node);
}

bool AccountAVLTree::insert(Account* account) {
    bool wasInserted = false;
    root = insertNode(root, account, wasInserted);
    if (wasInserted) {
        ++accountCount;
    }
    return wasInserted;
}

bool AccountAVLTree::remove(int accountNumber) {
    bool wasRemoved = false;
    root = removeNode(root, accountNumber, wasRemoved);
    if (wasRemoved) {
        --accountCount;
    }
    return wasRemoved;
}

Account* AccountAVLTree::find(int accountNumber) const {
    Node* current = root;
    while (current != nullptr) {
        if (accountNumber < current->account->accountNumber) {
            current = current->left;
        } else if (accountNumber > current->account->accountNumber) {
            current = current->right;
        } else {
            return current->account;
        }
    }
    return nullptr;
}

void AccountAVLTree::collectNodes(Node* node, DynamicArray<Account*>& collectedAccounts) {
    if (node == nullptr) {
        return;
    }
    collectNodes(node->left, collectedAccounts);
    collectedAccounts.add(node->account);
    collectNodes(node->right, collectedAccounts);
}

DynamicArray<Account*> AccountAVLTree::collectInOrder() const {
    DynamicArray<Account*> sortedAccounts;
    collectNodes(root, sortedAccounts);
    return sortedAccounts;
}

int AccountAVLTree::size() const {
    return accountCount;
}

void AccountAVLTree::destroySubtree(Node* node) {
    if (node == nullptr) {
        return;
    }
    destroySubtree(node->left);
    destroySubtree(node->right);
    delete node;
}

void AccountAVLTree::clear() {
    destroySubtree(root);
    root = nullptr;
    accountCount = 0;
}
