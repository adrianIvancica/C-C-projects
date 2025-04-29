#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

//difference between B and B+ trees, pack more child node points into each node
//second modification is the leaf nodes are all linked together as linked list
//examples for instructing disc type information into the system

using namespace std;

// B+ Tree order (max number of children in internal node)
const int ORDER = 3;

// Node definition
struct Node {
    bool isLeaf;                 // true if this is a leaf node
    vector<int> keys;           // keys in the node
    vector<Node*> children;     // pointers to children (used if internal node)
    Node* next;                 // used only for leaf nodes to point to next leaf

    // Constructor to initialize node type
    Node(bool leaf) {
        isLeaf = leaf;
        next = nullptr;
    }
};

// B+ Tree structure
struct BPlusTree {
    Node* root;

    BPlusTree() {
        root = new Node(true);  // start with an empty leaf node
    }

    // Public operations
    void insert(int key);
    void search(int key);
    void remove(int key);
    void display();

    // Internal helpers
    Node* findLeaf(int key);
    void insertInternal(int key, Node* leftChild, Node* rightChild);
    void splitLeaf(Node* leaf);
    void splitInternal(Node* internal);
};

// Find the leaf node where the key should be
Node* BPlusTree::findLeaf(int key) {
    Node* current = root;

    // Traverse down until a leaf is found
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i]) i++;
        current = current->children[i];
    }

    return current;
}

// Insert a key into the B+ Tree
void BPlusTree::insert(int key) {
    Node* leaf = findLeaf(key);

    // Insert key in the right position in the leaf node
    auto pos = lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    leaf->keys.insert(pos, key);

    // If leaf is overfull, split it
    if (leaf->keys.size() >= ORDER) {
        splitLeaf(leaf);
    }
}

// Split a leaf node into two and promote middle key
void BPlusTree::splitLeaf(Node* leaf) {
    Node* newLeaf = new Node(true);

    int mid = (ORDER + 1) / 2;

    // Move second half of keys to new leaf
    newLeaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    leaf->keys.resize(mid);

    // Link the leaf nodes together
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    // If splitting the root, make new root
    if (leaf == root) {
        root = new Node(false);
        root->keys.push_back(newLeaf->keys[0]);
        root->children.push_back(leaf);
        root->children.push_back(newLeaf);
    } else {
        // Insert promoted key into parent
        insertInternal(newLeaf->keys[0], leaf, newLeaf);
    }
}

// Handle promotion of key and pointers in internal node
void BPlusTree::insertInternal(int key, Node* leftChild, Node* rightChild) {
    Node* current = root;

    // Special case: inserting into root directly
    if (current == leftChild) {
        Node* newRoot = new Node(false);
        newRoot->keys.push_back(key);
        newRoot->children.push_back(leftChild);
        newRoot->children.push_back(rightChild);
        root = newRoot;
        return;
    }

    // Go down to find parent node of leftChild
    vector<Node*> path;
    while (!current->isLeaf) {
        path.push_back(current);
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i]) i++;
        current = current->children[i];
    }

    Node* parent = path.back();  // parent of the leaf we just split

    // Find insert position in parent
    int idx = 0;
    while (idx < parent->children.size() && parent->children[idx] != leftChild) idx++;

    parent->keys.insert(parent->keys.begin() + idx, key);
    parent->children.insert(parent->children.begin() + idx + 1, rightChild);

    if (parent->keys.size() >= ORDER) {
        splitInternal(parent);
    }
}

// Split an internal node
void BPlusTree::splitInternal(Node* node) {
    int mid = node->keys.size() / 2;
    int promoteKey = node->keys[mid];

    Node* newInternal = new Node(false);

    // Move second half of keys and children to new internal
    newInternal->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    newInternal->children.assign(node->children.begin() + mid + 1, node->children.end());

    node->keys.resize(mid);
    node->children.resize(mid + 1);

    if (node == root) {
        root = new Node(false);
        root->keys.push_back(promoteKey);
        root->children.push_back(node);
        root->children.push_back(newInternal);
    } else {
        insertInternal(promoteKey, node, newInternal);
    }
}

// Search for a key
void BPlusTree::search(int key) {
    Node* leaf = findLeaf(key);

    if (find(leaf->keys.begin(), leaf->keys.end(), key) != leaf->keys.end()) {
        cout << "Key " << key << " found in leaf node.\n";
    } else {
        cout << "Key " << key << " not found.\n";
    }
}

// Delete a key (no rebalancing here for simplicity)
void BPlusTree::remove(int key) {
    Node* leaf = findLeaf(key);

    auto it = find(leaf->keys.begin(), leaf->keys.end(), key);
    if (it != leaf->keys.end()) {
        leaf->keys.erase(it);
        cout << "Key " << key << " deleted.\n";
    } else {
        cout << "Key " << key << " not found.\n";
    }

    // Full rebalancing logic can be added here later if needed
}

// Display all keys in order by traversing leaves
void BPlusTree::display() {
    Node* current = root;

    // Go to the leftmost leaf
    while (!current->isLeaf)
        current = current->children[0];

    cout << "B+ Tree contents: ";
    while (current) {
        for (int key : current->keys)
            cout << key << " ";
        current = current->next;
    }
    cout << endl;
}

// Main program with menu interface
int main() {
    BPlusTree tree;
    int choice, key;

    while (true) {
        cout << "\nB+ Tree Menu:\n";
        cout << "1. Insert\n";
        cout << "2. Search\n";
        cout << "3. Delete\n";
        cout << "4. Display\n";
        cout << "5. Exit\n";
        cout << "Type a number:  ";
        cin >> choice;

        switch (choice) {
        case 1:
            cout << "Enter key to insert: ";
            cin >> key;
            tree.insert(key);
            break;
        case 2:
            cout << "Enter key to search: ";
            cin >> key;
            tree.search(key);
            break;
        case 3:
            cout << "Enter key to delete: ";
            cin >> key;
            tree.remove(key);
            break;
        case 4:
            tree.display();
            break;
        case 5:
            return 0;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    }
}
