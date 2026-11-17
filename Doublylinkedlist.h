#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <cstddef>
#include <fstream>
#include <new>
#include <sstream>
#include <string>
#include <utility>

// A reusable doubly linked list.
// The stored type T must provide:
//   int id;
//   bool isValid() const;
// and stream operators for reading/writing one record.
template <typename T>
class DoublyLinkedList {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        explicit Node(const T& value)
            : data(value), prev(nullptr), next(nullptr) {}
    };

    Node* head_;
    Node* tail_;
    std::size_t size_;

    void swap(DoublyLinkedList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    bool containsId(int id) const {
        for (Node* current = head_; current != nullptr; current = current->next) {
            if (current->data.id == id) {
                return true;
            }
        }
        return false;
    }

public:
    DoublyLinkedList() noexcept : head_(nullptr), tail_(nullptr), size_(0) {}

    // Deep-copy constructor: creates new nodes and copies each record.
    DoublyLinkedList(const DoublyLinkedList& other)
        : head_(nullptr), tail_(nullptr), size_(0) {
        try {
            for (Node* current = other.head_; current != nullptr; current = current->next) {
                if (!pushBack(current->data)) {
                    throw std::bad_alloc();
                }
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    // Move constructor.
    DoublyLinkedList(DoublyLinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    // Copy-and-swap assignment.
    DoublyLinkedList& operator=(DoublyLinkedList other) noexcept {
        swap(other);
        return *this;
    }

    // Destructor releases every dynamically allocated node.
    ~DoublyLinkedList() {
        clear();
    }

    bool pushBack(const T& value) {
        if (!value.isValid() || containsId(value.id)) {
            return false;
        }

        Node* node = nullptr;
        try {
            node = new Node(value);
        } catch (const std::bad_alloc&) {
            return false;
        }

        node->prev = tail_;
        if (tail_ != nullptr) {
            tail_->next = node;
        } else {
            head_ = node;
        }
        tail_ = node;
        ++size_;
        return true;
    }

    bool pushFront(const T& value) {
        if (!value.isValid() || containsId(value.id)) {
            return false;
        }

        Node* node = nullptr;
        try {
            node = new Node(value);
        } catch (const std::bad_alloc&) {
            return false;
        }

        node->next = head_;
        if (head_ != nullptr) {
            head_->prev = node;
        } else {
            tail_ = node;
        }
        head_ = node;
        ++size_;
        return true;
    }

    T* find(int id) {
        for (Node* current = head_; current != nullptr; current = current->next) {
            if (current->data.id == id) {
                return &current->data;
            }
        }
        return nullptr;
    }

    const T* find(int id) const {
        for (Node* current = head_; current != nullptr; current = current->next) {
            if (current->data.id == id) {
                return &current->data;
            }
        }
        return nullptr;
    }

    bool remove(int id) {
        Node* current = head_;

        while (current != nullptr) {
            if (current->data.id == id) {
                if (current->prev != nullptr) {
                    current->prev->next = current->next;
                } else {
                    head_ = current->next;
                }

                if (current->next != nullptr) {
                    current->next->prev = current->prev;
                } else {
                    tail_ = current->prev;
                }

                delete current;
                --size_;
                return true;
            }
            current = current->next;
        }

        return false;
    }

    bool update(int id, const T& replacement) {
        if (!replacement.isValid() || replacement.id != id) {
            return false;
        }

        T* existing = find(id);
        if (existing == nullptr) {
            return false;
        }

        try {
            T copy(replacement);
            using std::swap;
            swap(*existing, copy);
        } catch (const std::bad_alloc&) {
            return false;
        }
        return true;
    }

    void clear() noexcept {
        Node* current = head_;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }

    std::size_t size() const noexcept {
        return size_;
    }

    bool empty() const noexcept {
        return size_ == 0;
    }

    // Calls the supplied function for each record, from head to tail.
    template <typename Function>
    void forEach(Function function) const {
        for (Node* current = head_; current != nullptr; current = current->next) {
            function(current->data);
        }
    }

    // Loads valid, non-duplicate records. Blank lines and # comments are ignored.
    // The existing list is replaced only after the file has been read successfully.
    bool loadFromFile(const std::string& filename,
                      std::size_t& loaded,
                      std::size_t& rejected) {
        loaded = 0;
        rejected = 0;

        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        DoublyLinkedList<T> temporary;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }

            std::istringstream input(line);
            T record{};

            if ((input >> record) && temporary.pushBack(record)) {
                ++loaded;
            } else {
                ++rejected;
            }
        }

        if (file.bad()) {
            return false;
        }

        swap(temporary);
        return true;
    }

    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        file << "# Library returns register\n";
        file << "# id|member|title|due|returned\n";

        bool writeOkay = true;
        forEach([&](const T& record) {
            if (writeOkay) {
                file << record << '\n';
                if (!file) {
                    writeOkay = false;
                }
            }
        });

        file.flush();
        return writeOkay && file.good();
    }
};

#endif
