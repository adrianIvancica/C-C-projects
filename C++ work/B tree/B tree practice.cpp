#include <iostream>
#include  <fstream>
#include <string>

// log(M*N) compared to binary tree of log2(N)
//binary tree can become unbalanced
//b trees may have numerous children, database systems and handle large data may not fit to primary memory
// reduce disk operations proportional to trees height
// big O(logn) for many operations
// both primary and secondary memory
//b tree node is like a disk page, root is in main memory

//code for a B tree
using namespace std;

// Define the B-Tree node using a struct
struct BTreeNode {
    int *keys;               // Array of keys
    int minDegree;           // Minimum degree (defines max/min children)
    BTreeNode **children;    // Array of child pointers
    int numKeys;             // Number of keys currently in the node
    bool isLeaf;             // True if node is a leaf

    // Constructor to initialize the node
    BTreeNode(int degree, bool leafNode) {
        minDegree = degree;
        isLeaf = leafNode;
        keys = new int[2 * degree - 1];
        children = new BTreeNode*[2 * degree];
        numKeys = 0;
    }

    // Function to traverse and print the keys
    void traverse() {
        int i;
        for (i = 0; i < numKeys; i++) {
            if (!isLeaf)
                children[i]->traverse();
            cout << " " << keys[i];
        }
        if (!isLeaf)
            children[i]->traverse();
    }

    // Search for a key in the node
    BTreeNode* search(int key) {
        int i = 0;
        while (i < numKeys && key > keys[i])
            i++;

        if (i < numKeys && keys[i] == key)
            return this;

        if (isLeaf)
            return nullptr;

        return children[i]->search(key);
    }

    // Insert a key into a node that's not full
    void insertIntoNonFull(int key) {
        int i = numKeys - 1;

        if (isLeaf) {
            while (i >= 0 && keys[i] > key) {
                keys[i + 1] = keys[i];
                i--;
            }
            keys[i + 1] = key;
            numKeys++;
        } else {
            while (i >= 0 && keys[i] > key)
                i--;

            if (children[i + 1]->numKeys == 2 * minDegree - 1) {
                splitChild(i + 1, children[i + 1]);

                if (keys[i + 1] < key)
                    i++;
            }

            children[i + 1]->insertIntoNonFull(key);
        }
    }

    // Split a child node that's full
    void splitChild(int index, BTreeNode *child) {
        BTreeNode *newNode = new BTreeNode(child->minDegree, child->isLeaf);
        newNode->numKeys = minDegree - 1;

        for (int j = 0; j < minDegree - 1; j++)
            newNode->keys[j] = child->keys[j + minDegree];

        if (!child->isLeaf) {
            for (int j = 0; j < minDegree; j++)
                newNode->children[j] = child->children[j + minDegree];
        }

        child->numKeys = minDegree - 1;

        for (int j = numKeys; j >= index + 1; j--)
            children[j + 1] = children[j];

        children[index + 1] = newNode;

        for (int j = numKeys - 1; j >= index; j--)
            keys[j + 1] = keys[j];

        keys[index] = child->keys[minDegree - 1];
        numKeys++;
    }
};

// Define the B-Tree structure using a struct
struct BTree {
    BTreeNode *root;
    int minDegree;

    // Constructor to initialize the B-Tree
    BTree(int degree) {
        root = nullptr;
        minDegree = degree;
    }

    // Function to traverse and print the tree
    void traverse() {
        if (root) root->traverse();
        else cout << "(tree is empty)";
    }

    // Function to search for a key in the tree
    BTreeNode* search(int key) {
        return (root == nullptr) ? nullptr : root->search(key);
    }

    // Function to insert a key into the tree
    void insert(int key) {
        if (!root) {
            root = new BTreeNode(minDegree, true);
            root->keys[0] = key;
            root->numKeys = 1;
        } else {
            if (root->numKeys == 2 * minDegree - 1) {
                BTreeNode *newRoot = new BTreeNode(minDegree, false);
                newRoot->children[0] = root;
                newRoot->splitChild(0, root);

                int i = (newRoot->keys[0] < key) ? 1 : 0;
                newRoot->children[i]->insertIntoNonFull(key);

                root = newRoot;
            } else {
                root->insertIntoNonFull(key);
            }
        }
    }
};

int main() {  //created an initial menu to ask what should be done to the tree
    int degree;
    cout << "Enter minimum degree (t) for the B-Tree: ";
    cin >> degree;

    BTree myTree(degree);

    int option;
    do {
        cout << "\n===== Menu =====\n";
        cout << "1. Insert a number\n";
        cout << "2. Search a number\n";
        cout << "3. Display (in-order traversal)\n";
        cout << "4. Exit\n";
        cout << "Select an option: ";
        cin >> option;

        switch (option) {
            case 1: {
                int num;
                cout << "Number to insert: ";
                cin >> num;
                myTree.insert(num);
                cout << "Inserted " << num << ".\n";
                break;
            }

            case 2: {
                int target;
                cout << "Number to search: ";
                cin >> target;
                if (myTree.search(target))
                    cout << target << " exists in the tree.\n";
                else
                    cout << target << " not found.\n";
                break;
            }

            case 3:
                cout << "B-Tree contents (sorted):";
                myTree.traverse();
                cout << "\n";
                break;

            case 4:
                cout << "Exiting...\n";
                break;

            default:
                cout << "Invalid option. Try again.\n";
        }

    } while (option != 4);

    return 0;
}
