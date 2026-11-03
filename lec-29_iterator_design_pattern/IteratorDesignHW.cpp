#include <iostream>
#include <vector>
#include <stack>
#include <string>
using namespace std;

// ============================================================
// EXTENDED ITERATOR INTERFACE
// ============================================================
template <typename T>
class Iterator {
public:
    virtual bool hasNext()     = 0;
    virtual T    next()        = 0;
    virtual bool hasPrevious() = 0;
    virtual T    previous()    = 0;
    virtual ~Iterator() {}
};

template <typename T>
class Iterable {
public:
    virtual Iterator<T>* getIterator() = 0;
    virtual ~Iterable() {}
};

// ============================================================
// LINKED LIST (doubly-linked to support previous)
// ============================================================
template <typename T>
class LinkedList : public Iterable<T> {
public:
    T data;
    LinkedList* next;
    LinkedList* prev;    // NEW: for reverse traversal

    LinkedList(T d, LinkedList* n = nullptr, LinkedList* p = nullptr)
        : data(d), next(n), prev(p) {}

    Iterator<T>* getIterator() override;
};

// Iterator holds BOTH current and previous pointers
template <typename T>
class LinkedListIterator : public Iterator<T> {
    LinkedList<T>* current;
    LinkedList<T>* previousNode;
public:
    LinkedListIterator(LinkedList<T>* head)
        : current(head), previousNode(nullptr) {}

    bool hasNext() override { return current != nullptr; }

    T next() override {
        if (!current) throw runtime_error("No more elements");
        T val = current->data;
        previousNode = current;
        current = current->next;
        return val;
    }

    bool hasPrevious() override {
        // hasPrevious is true if we've already moved forward at least once
        // or if we are currently on a node whose prev exists
        return previousNode != nullptr;
    }

    T previous() override {
        if (!hasPrevious()) throw runtime_error("No previous element");
        // Move back to the previously visited node
        if (current == nullptr) {
            // We're past the end — current should be previous
            current = previousNode;
        } else {
            current = previousNode;
        }
        T val = current->data;
        previousNode = current->prev;
        return val;
    }
};

template <typename T>
Iterator<T>* LinkedList<T>::getIterator() {
    return new LinkedListIterator<T>(this);
}

// ============================================================
// BINARY TREE + BIDIRECTIONAL INORDER ITERATOR
// ============================================================
template <typename T>
class BinaryTree : public Iterable<T> {
public:
    T data;
    BinaryTree* left;
    BinaryTree* right;
    BinaryTree* parent;    // NEW: needed for reverse traversal

    BinaryTree(T d, BinaryTree* p = nullptr)
        : data(d), left(nullptr), right(nullptr), parent(p) {}
    Iterator<T>* getIterator() override;
};

// Bidirectional inorder iterator using two stacks
template <typename T>
class BinaryTreeInorderIterator : public Iterator<T> {
    stack<BinaryTree<T>*> nextStack;
    stack<BinaryTree<T>*> prevStack;

    void pushLeft(BinaryTree<T>* node) {
        while (node) { nextStack.push(node); node = node->left; }
    }
    void pushRight(BinaryTree<T>* node) {
        while (node) { prevStack.push(node); node = node->right; }
    }
public:
    BinaryTreeInorderIterator(BinaryTree<T>* root) {
        pushLeft(root);
    }

    bool hasNext() override { return !nextStack.empty(); }

    T next() override {
        if (nextStack.empty()) throw runtime_error("No more elements");
        BinaryTree<T>* node = nextStack.top(); nextStack.pop();
        pushLeft(node->right);
        // Push node onto prevStack so we can go backwards
        prevStack.push(node);
        return node->data;
    }

    bool hasPrevious() override {
        return !prevStack.empty();
    }

    T previous() override {
        if (prevStack.empty()) throw runtime_error("No previous element");
        BinaryTree<T>* node = prevStack.top(); prevStack.pop();
        pushRight(node->left);
        nextStack.push(node);
        return node->data;
    }
};

template <typename T>
Iterator<T>* BinaryTree<T>::getIterator() {
    return new BinaryTreeInorderIterator<T>(this);
}

// ============================================================
// PLAYLIST + BIDIRECTIONAL ITERATOR
// ============================================================
class Song {
    string title, artist;
public:
    Song(string t, string a) : title(t), artist(a) {}
    string getTitle() const { return title; }
    string getArtist() const { return artist; }
};

class Playlist : public Iterable<Song> {
    vector<Song> songs;
public:
    void addSong(const Song& s) { songs.push_back(s); }
    vector<Song>& getSongs() { return songs; }
    Iterator<Song>* getIterator() override;
};

class PlaylistIterator : public Iterator<Song> {
    vector<Song>* songs;
    int index;
public:
    PlaylistIterator(vector<Song>* s) : songs(s), index(0) {}

    bool hasNext() override { return index < (int)songs->size(); }

    Song next() override {
        if (!hasNext()) throw runtime_error("No more songs");
        return (*songs)[index++];
    }

    bool hasPrevious() override { return index > 1; }

    Song previous() override {
        if (!hasPrevious()) throw runtime_error("No previous song");
        index -= 2;              // step back two — because next() had advanced one
        return (*songs)[index++];
    }
};

Iterator<Song>* Playlist::getIterator() {
    return new PlaylistIterator(&songs);
}

// ============================================================
// CLIENT
// ============================================================
int main() {
    // ---- LinkedList forward + backward ----
    LinkedList<int>* n3 = new LinkedList<int>(3);
    LinkedList<int>* n2 = new LinkedList<int>(2, n3);
    n3->prev = n2;
    LinkedList<int>* n1 = new LinkedList<int>(1, n2);
    n2->prev = n1;

    Iterator<int>* it = n1->getIterator();
    cout << "LinkedList forward:  ";
    while (it->hasNext()) cout << it->next() << " ";
    cout << "\nLinkedList backward: ";
    while (it->hasPrevious()) cout << it->previous() << " ";
    cout << "\n\n";
    delete it;

    // ---- BinaryTree forward + backward ----
    BinaryTree<int>* root = new BinaryTree<int>(2);
    root->left  = new BinaryTree<int>(1, root);
    root->right = new BinaryTree<int>(3, root);

    Iterator<int>* btIt = root->getIterator();
    cout << "BinaryTree forward:  ";
    while (btIt->hasNext()) cout << btIt->next() << " ";
    cout << "\nBinaryTree backward: ";
    while (btIt->hasPrevious()) cout << btIt->previous() << " ";
    cout << "\n\n";
    delete btIt;

    // ---- Playlist forward + backward ----
    Playlist playlist;
    playlist.addSong(Song("Admiring You", "Karan Aujla"));
    playlist.addSong(Song("Hustle", "Anuj Jain"));
    playlist.addSong(Song("Kesariya", "Arijit Singh"));

    Iterator<Song>* plIt = playlist.getIterator();
    cout << "Playlist forward:\n";
    while (plIt->hasNext()) {
        Song s = plIt->next();
        cout << "  - " << s.getTitle() << " by " << s.getArtist() << "\n";
    }
    cout << "Playlist backward:\n";
    while (plIt->hasPrevious()) {
        Song s = plIt->previous();
        cout << "  - " << s.getTitle() << " by " << s.getArtist() << "\n";
    }
    delete plIt;

    return 0;
}
