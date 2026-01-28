#pragma once

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

/**
 * @class List
 * @brief A generic singly linked list implementation.
 *
 * This class provides a dynamic sequence container that allows for O(1) insertion
 * and removal at both ends. It implements the "Rule of Five" for proper resource management.
 *
 * @tparam T The type of the elements stored in the list.
 */
template <typename T>
class List {
    struct Node;

    Node* pHead;      /**< Pointer to the first node of the list. */
    Node* pTail;      /**< Pointer to the last node of the list. */
    size_t nodeCount; /**< Number of elements currently in the list. */

public:
    // =============== Iterators ===============
    /**
     * @class List::Iterator
     * @brief Forward iterator for @c List<T>.
     *
     * Provides mutable access to elements and supports forward traversal only
     * (pre-increment).
     *
     * @note Construct with @c Iterator(nullptr) to represent @c end().
     */
    class Iterator {
    public:
        /**
         * @brief Construct an iterator pointing to a node.
         * @param node Pointer to the node this iterator will reference (may be nullptr).
         */
        explicit Iterator(Node* node) : current(node) {}

        /**
         * @brief Dereference the iterator.
         * @return Reference to the element pointed by the iterator.
         * @pre The iterator must not be @c end().
         */
        T& operator*() const { return current->data; }

        /**
         * @brief Member access through iterator.
         * @return Pointer to the element pointed by the iterator.
         * @pre The iterator must not be @c end().
         */
        T* operator->() const { return &(current->data); }

        /**
         * @brief Compare two iterators for equality.
         * @param other Iterator to compare with.
         * @return @c true if both iterators point to the same node; otherwise @c false.
         */
        bool operator==(const Iterator& other) const { return current == other.current; }

        /**
         * @brief Compare two iterators for inequality.
         * @param other Iterator to compare with.
         * @return @c true if iterators point to different nodes; otherwise @c false.
         */
        bool operator!=(const Iterator& other) const { return current != other.current; }
        Iterator& operator++() {
            if (current) {
                current = current->next;
            }
            return *this;
        }

    private:
        Node* current; /**< Pointer to the current node referenced by the iterator. */
    };

    /**
     * @class List::ConstIterator
     * @brief Forward iterator for const access to @c List<T>.
     *
     * Provides read-only access to elements and supports forward traversal only.
     * Use @c ConstIterator(nullptr) to represent @c end().
     */
    class ConstIterator {
    public:
        /**
         * @brief Construct a const iterator pointing to a node.
         * @param node Pointer to the node this iterator will reference (may be nullptr).
         */
        explicit ConstIterator(Node* node) : current(node) {}

        /**
         * @brief Dereference the const iterator.
         * @return Const reference to the element pointed by the iterator.
         * @pre The iterator must not be @c end().
         */
        const T& operator*() const { return current->data; }

        /**
         * @brief Member access through const iterator.
         * @return Pointer to the element pointed by the iterator.
         * @pre The iterator must not be @c end().
         */
        const T* operator->() const { return &(current->data); }

        /**
         * @brief Compare two const iterators for inequality.
         * @param other ConstIterator to compare with.
         * @return @c true if iterators point to different nodes; otherwise @c false.
         */
        bool operator!=(const ConstIterator& other) const { return current != other.current; }

        /**
         * @brief Pre-increment: advance const iterator to the next element.
         * @return Reference to the advanced const iterator.
         */
        ConstIterator& operator++() {
            if (current) {
                current = current->next;
            }
            return *this;
        }

    private:
        Node* current; /**< Pointer to the current node referenced by the const iterator. */
    };

    // =============== Constructors & Destructor ===============
    /**
     * @brief Default constructor. Creates an empty list.
     */
    List();

    /**
     * @brief Copy constructor (Deep copy).
     * @param other The list to be copied.
     * @note Basic exception safety: if an element's copy constructor throws,
     * all partially allocated nodes are freed and the exception is rethrown.
     */
    List(const List& other);

    /**
     * @brief Copy assignment operator using the copy-and-swap idiom.
     * @param other The list to be copied.
     * @return Reference to this list.
     * @note Strong exception safety: if the copy fails, the original list remains intact.
     */
    List& operator=(const List& other);

    /**
     * @brief Move constructor. Transfers ownership of resources.
     * @param other The list to move from.
     * @note Noexcept: guaranteed not to throw as it only performs pointer manipulation.
     */
    List(List&& other) noexcept;

    /**
     * @brief Move assignment operator. Exchanges resources between lists.
     * @param other The list to move from.
     * @return Reference to this list.
     * @note Noexcept: guaranteed not to throw.
     */
    List& operator=(List&& other) noexcept;

    /**
     * @brief Destructor. Deallocates all nodes in the list.
     */
    ~List() noexcept;

    // =============== Capacity ===============
    /**
     * @brief Returns the number of elements in the list.
     * @return The current size of the list.
     */
    [[nodiscard]] size_t size() const noexcept;

    /**
     * @brief Checks if the list contains no elements.
     * @return true if the list is empty, false otherwise.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    // =============== Element access ===============
    /**
     * @brief Access the first element.
     * @return A mutable reference to the data in the head node.
     * @throw std::runtime_error if the list is empty.
     */
    [[nodiscard]] T& getHead();

    /**
     * @brief Access the first element.
     * @return Const reference to the data in the head node.
     * @throw std::runtime_error if the list is empty.
     */
    [[nodiscard]] const T& getHead() const;

    /**
     * @brief Access the last element.
     * @return A mutable reference to the data in the tail node.
     * @throw std::runtime_error if the list is empty.
     */
    [[nodiscard]] T& getTail();

    /**
     * @brief Access the last element.
     * @return A const reference to the data in the tail node.
     * @throw std::runtime_error if the list is empty.
     */
    [[nodiscard]] const T& getTail() const;

    /**
     * @brief Access an element at a specific position with bounds checking.
     * @param pos Zero-based index of the element.
     * @return A mutable reference to the data at the given position.
     * @throw std::out_of_range if the index is invalid.
     */
    [[nodiscard]] T& getAt(size_t pos);

    /**
     * @brief Access an element at a specific position with bounds checking.
     * @param pos Zero-based index of the element.
     * @return A const reference to the data at the given position.
     * @throw std::out_of_range if the index is invalid.
     */
    [[nodiscard]] const T& getAt(size_t pos) const;

    /**
     * @brief Access an element at a specific position with bounds checking.
     * @param pos Zero-based index of the element.
     * @return A mutable reference to the data at the given position.
     * @throw std::out_of_range if the index is invalid.
     */
    [[nodiscard]] T& operator[](size_t pos);

    /**
     * @brief Access an element at a specific position with bounds checking.
     * @param pos Zero-based index of the element.
     * @return A const reference to the data at the given position.
     * @throw std::out_of_range if the index is invalid.
     */
    [[nodiscard]] const T& operator[](size_t pos) const;

    // =============== Modifiers ===============
    /**
     * @brief Removes all elements from the list.
     *
     * Replaces the list with an empty state. All nodes are deallocated.
     * @note Noexcept: guaranteed to clean up all resources without throwing.
     * @note Complexity: O(n).
     */
    void clear() noexcept;

    /**
     * @brief Adds an element to the beginning of the list by copying.
     * @param val The value to be copied into a new node.
     * @throw std::bad_alloc if memory for the new node cannot be allocated.
     * @note Complexity: O(1).
     */
    void pushFront(const T& val);

    /**
     * @brief Adds an element to the beginning of the list by moving.
     * @param val The value to be moved into a new node.
     * @note Complexity: O(1).
     */
    void pushFront(T&& val);

    /**
     * @brief Adds an element to the end of the list by copying.
     * @param val The value to be copied into a new node.
     * @note Complexity: O(1).
     */
    void pushBack(const T& val);

    /**
     * @brief Adds an element to the end of the list by moving.
     * @param val The value to be moved into a new node.
     * @note Complexity: O(1).
     */
    void pushBack(T&& val);

    /**
     * @brief Inserts a copy of the element at the specified position.
     * @param val The value to be copied.
     * @param pos Zero-based index where the element will be inserted.
     * @throw std::out_of_range if pos > size().
     * @note Complexity: O(n) as it may require traversing the list.
     */
    void insertAt(const T& val, size_t pos);

    /**
     * @brief Inserts a moved element at the specified position.
     * @param val The value to be moved.
     * @param pos Zero-based index where the element will be inserted.
     * @throw std::out_of_range if pos > size().
     * @note Complexity: O(n) as it may require traversing the list.
     */
    void insertAt(T&& val, size_t pos);

    /**
     * @brief Removes the first element of the list.
     * @throw std::runtime_error if the list is empty.
     * @note Complexity: O(1).
     */
    void popFront();

    /**
     * @brief Removes the last element of the list.
     * @throw std::runtime_error if the list is empty.
     * @note Complexity: O(n) in singly linked lists to find the penultimate node.
     */
    void popBack();

    /**
     * @brief Removes the element at the specified position.
     * @param pos Zero-based index of the element to remove.
     * @throw std::out_of_range if pos >= size().
     * @note Complexity: O(n).
     */
    void removeAt(size_t pos);

    /**
     * @brief Swaps the data values between two nodes at the given positions.
     * @param pos1 Zero-based index of the first node.
     * @param pos2 Zero-based index of the second node.
     * @throw std::out_of_range if either index is invalid.
     * @note This implementation swaps the data content, not the nodes themselves.
     */
    void swap(size_t pos1, size_t pos2);

    /**
     * @brief Reverses the order of the elements in the list.
     *
     * Performed in-place by reassigning node pointers.
     * @note Noexcept: guaranteed not to throw as no allocation occurs.
     * @note Complexity: O(n).
     */
    void reverse() noexcept;

    // =============== Iteration ===============
    /**
     * @brief Returns an iterator to the first element of the list.
     * @return An Iterator pointing to the head node.
     * @note If the list is empty, the returned iterator will be equal to end().
     */
    [[nodiscard]] Iterator begin() noexcept;

    /**
     * @brief Returns an iterator to the element following the last element.
     * @return An Iterator representing the past-the-end element (nullptr).
     * @note This iterator acts as a placeholder; attempting to dereference it
     * results in undefined behavior.
     */
    [[nodiscard]] Iterator end() noexcept;

    /**
     * @brief Returns a constant iterator to the first element of the list.
     * @return A ConstIterator pointing to the head node.
     * @note This version is called on constant List objects and prevents
     * modification of the elements through the iterator.
     */
    [[nodiscard]] ConstIterator begin() const noexcept;

    /**
     * @brief Returns a constant iterator to the element following the last element.
     * @return A ConstIterator representing the past-the-end element (nullptr).
     */
    [[nodiscard]] ConstIterator end() const noexcept;

    /**
     * @brief Returns a constant iterator to the first element.
     * @return A ConstIterator pointing to the head node.
     * @note Unlike begin(), this always returns a ConstIterator even if
     * the list itself is not const.
     */
    [[nodiscard]] ConstIterator cbegin() const noexcept;

    /**
     * @brief Returns a constant iterator to the past-the-end element.
     * @return A ConstIterator representing nullptr.
     */
    [[nodiscard]] ConstIterator cend() const noexcept;

    // =============== Utilities ===============
    /**
     * @brief Returns a string representation of the list elements.
     * @return A string with format "elem1 -> elem2 -> ...".
     * If empty, returns "List is empty".
     * @note Requires type T to support the operator<< for streams.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Prints the string representation of the list to standard output.
     */
    void print() const;

    /**
     * @brief Checks if a specific value exists within the list.
     * @param val The value to search for.
     * @return true if the value is found, false otherwise.
     * @note Complexity: O(n). Requires T to support operator==.
     */
    [[nodiscard]] bool contains(const T& val) const;

    /**
     * @brief Searches for a value and returns its position.
     * @param val The value to search for.
     * @return An std::optional containing the zero-based index if found,
     * or std::nullopt if the value is not in the list.
     * @note Complexity: O(n). Requires T to support operator==.
     */
    [[nodiscard]] std::optional<size_t> find(const T& val) const;

private:
    // =============== Node Structure ===============
    /**
     * @brief Internal node structure for the singly linked list.
     * * Each node contains a data element and a pointer to the next node.
     */
    struct Node {
        T data;      ///< The actual data stored in the node.
        Node* next;  ///< Pointer to the next node in the sequence.
        /**
         * @brief Constructs a node by copying the data.
         */
        explicit Node(const T& data, Node* next = nullptr) : data(data), next(next) {}
        /**
         * @brief Constructs a node by moving the data (efficient for large objects).
         */
        explicit Node(T&& data, Node* next = nullptr) : data(std::move(data)), next(next) {}
    };

    // =============== Private Helpers ===============
    /**
     * @brief Adds a node to the very end of the list.
     * @param newNode Pointer to a pre-allocated node.
     * @pre newNode must not be nullptr.
     */
    void appendNode(Node* newNode) noexcept;

    /**
     * @brief Adds a node to the very beginning of the list.
     * @param newNode Pointer to a pre-allocated node.
     * @pre newNode must not be nullptr.
     */
    void prependNode(Node* newNode) noexcept;

    /**
     * @brief Traverses the list to find a node pointer by its index.
     * @param index Zero-based position.
     * @return Pointer to the Node at that position.
     * @pre index must be valid (0 <= index < nodeCount).
     * @note Complexity: O(n).
     */
    Node* getNodeAt(size_t index) const;

    /**
     * @brief Links a new node immediately after an existing one.
     * @param prevNode Pointer to the node that will precede the new one.
     * @param newNode Pointer to the node to be inserted.
     * @pre Both pointers must not be nullptr.
     */
    void insertAfter(Node* prevNode, Node* newNode);

    /**
     * @brief Efficiently swaps the internal state of two List objects.
     *
     * Only swaps pointers and the count, not the actual nodes.
     */
    void swap(List& other) noexcept;
};

// =============== Constructors & Destructor ===============
template <typename T>
List<T>::List() : pHead(nullptr), pTail(nullptr), nodeCount(0) {}

template <typename T>
List<T>::List(const List& other) : List() {
    try {
        for (const auto& item : other) {
            pushBack(item);
        }
    } catch (...) {
        clear();
        throw;
    }
}

template <typename T>
List<T>& List<T>::operator=(const List& other) {
    if (this != &other) {
        List temp(other);
        this->swap(temp);
    }
    return *this;
}

template <typename T>
List<T>::List(List&& other) noexcept : pHead(other.pHead), pTail(other.pTail), nodeCount(other.nodeCount) {
    other.pHead     = nullptr;
    other.pTail     = nullptr;
    other.nodeCount = 0;
}

template <typename T>
List<T>& List<T>::operator=(List&& other) noexcept {
    this->swap(other);
    return *this;
}

template <typename T>
List<T>::~List() noexcept {
    clear();
}

// =============== Capacity ===============
template <typename T>
size_t List<T>::size() const noexcept {
    return nodeCount;
}

template <typename T>
bool List<T>::isEmpty() const noexcept {
    return nodeCount == 0;
}

// =============== Element access ===============
template <typename T>
T& List<T>::getHead() {
    if (!pHead) {
        throw std::runtime_error("List is empty");
    }
    return pHead->data;
}

template <typename T>
const T& List<T>::getHead() const {
    if (!pHead) {
        throw std::runtime_error("List is empty");
    }
    return pHead->data;
}

template <typename T>
T& List<T>::getTail() {
    if (!pTail) {
        throw std::runtime_error("List is empty");
    }
    return pTail->data;
}

template <typename T>
const T& List<T>::getTail() const {
    if (!pTail) {
        throw std::runtime_error("List is empty");
    }
    return pTail->data;
}

template <typename T>
T& List<T>::getAt(size_t pos) {
    if (pos >= nodeCount) {
        throw std::out_of_range("getAt: Index out of bounds");
    }
    return getNodeAt(pos)->data;
}

template <typename T>
const T& List<T>::getAt(size_t pos) const {
    if (pos >= nodeCount) {
        throw std::out_of_range("getAt: Index out of bounds");
    }
    return getNodeAt(pos)->data;
}

template <typename T>
T& List<T>::operator[](size_t pos) {
    return getAt(pos);
}

template <typename T>
const T& List<T>::operator[](size_t pos) const {
    return getAt(pos);
}

// =============== Modifiers ===============
template <typename T>
void List<T>::clear() noexcept {
    while (pHead != nullptr) {
        Node* temp = pHead;

        pHead = pHead->next;
        delete temp;
    }
    pTail = nullptr;

    nodeCount = 0;
}

template <typename T>
void List<T>::pushFront(const T& val) {
    Node* newNode = new Node(val);
    prependNode(newNode);
}

template <typename T>
void List<T>::pushFront(T&& val) {
    Node* newNode = new Node(std::move(val));
    prependNode(newNode);
}

template <typename T>
void List<T>::pushBack(const T& val) {
    Node* newNode = new Node(val);
    appendNode(newNode);
}

template <typename T>
void List<T>::pushBack(T&& val) {
    Node* newNode = new Node(std::move(val));
    appendNode(newNode);
}

template <typename T>
void List<T>::insertAt(const T& val, size_t pos) {
    if (pos > nodeCount) {
        throw std::out_of_range("Index out of bounds");
    }

    if (pos == 0) {
        return pushFront(val);
    }
    if (pos == nodeCount) {
        return pushBack(val);
    }

    Node* newNode = new Node(val);
    Node* prev    = getNodeAt(pos - 1);
    insertAfter(prev, newNode);
}

template <typename T>
void List<T>::insertAt(T&& val, size_t pos) {
    if (pos > nodeCount) {
        throw std::out_of_range("Index out of bounds");
    }

    if (pos == 0) {
        return pushFront(std::move(val));
    }

    if (pos == nodeCount) {
        return pushBack(std::move(val));
    }

    Node* newNode = new Node(val);
    Node* prev    = getNodeAt(pos - 1);
    insertAfter(prev, newNode);
}

template <typename T>
void List<T>::popFront() {
    if (pHead == nullptr) {
        throw std::runtime_error("List is empty");
    }

    Node* oldHead = pHead;

    if (nodeCount == 1) {
        pHead = nullptr;
        pTail = nullptr;
    } else {
        pHead = pHead->next;
    }

    delete oldHead;
    nodeCount--;
}

template <typename T>
void List<T>::popBack() {
    if (pHead == nullptr) {
        throw std::runtime_error("List is empty");
    }

    Node* oldTail = pTail;

    if (nodeCount == 1) {
        pHead = nullptr;
        pTail = nullptr;
    } else {
        Node* newTail = getNodeAt(nodeCount - 2);
        newTail->next = nullptr;
        pTail         = newTail;
    }

    delete oldTail;
    nodeCount--;
}

template <typename T>
void List<T>::removeAt(size_t pos) {
    if (pos >= nodeCount) {
        throw std::out_of_range("Index out of bounds");
    }

    if (pos == 0) {
        return popFront();
    }

    if (pos == nodeCount - 1) {
        return popBack();
    }

    Node* prevNode     = getNodeAt(pos - 1);
    Node* nodeToDelete = prevNode->next;

    prevNode->next = nodeToDelete->next;

    delete nodeToDelete;
    nodeCount--;
}

template <typename T>
void List<T>::swap(size_t pos1, size_t pos2) {
    if (pos1 == pos2)
        return;
    if (pos1 >= nodeCount || pos2 >= nodeCount) {
        throw std::out_of_range("swap: Index out of bounds");
    }

    Node* node1 = getNodeAt(pos1);
    Node* node2 = getNodeAt(pos2);

    std::swap(node1->data, node2->data);
}

template <typename T>
void List<T>::reverse() noexcept {
    if (nodeCount <= 1) {
        return;
    }

    Node* prev = nullptr;
    Node* curr = pHead;
    Node* next = nullptr;

    pTail = pHead;

    while (curr != nullptr) {
        next       = curr->next;
        curr->next = prev;
        prev       = curr;
        curr       = next;
    }
    pHead = prev;
}

// =============== Iteration ===============
template <typename T>
List<T>::Iterator List<T>::begin() noexcept {
    return Iterator(pHead);
}

template <typename T>
List<T>::Iterator List<T>::end() noexcept {
    return Iterator(nullptr);
}

template <typename T>
List<T>::ConstIterator List<T>::begin() const noexcept {
    return ConstIterator(pHead);
}

template <typename T>
List<T>::ConstIterator List<T>::end() const noexcept {
    return ConstIterator(nullptr);
}

template <typename T>
List<T>::ConstIterator List<T>::cbegin() const noexcept {
    return ConstIterator(pHead);
}

template <typename T>
List<T>::ConstIterator List<T>::cend() const noexcept {
    return ConstIterator(nullptr);
}

// =============== Utilities ===============
template <typename T>
std::string List<T>::toString() const {
    if (isEmpty())
        return "List is empty";

    std::stringstream ss;
    auto it = begin();
    ss << *it;
    ++it;

    for (; it != end(); ++it) {
        ss << " -> " << *it;
    }
    return ss.str();
}

template <typename T>
void List<T>::print() const {
    std::cout << toString() << std::endl;
}

template <typename T>
bool List<T>::contains(const T& val) const {
    // cppcheck-suppress useStlAlgorithm
    for (const auto& item : *this) {
        if (item == val) {
            return true;
        }
    }
    return false;
    /* Alternative using STL algorithm:
    return std::any_of(this->begin(), this->end(), [&val](const T& item) {
        return item == val;});
    */
}

template <typename T>
std::optional<size_t> List<T>::find(const T& val) const {
    size_t index = 0;
    // cppcheck-suppress useStlAlgorithm
    for (const auto& item : *this) {
        if (item == val) {
            return index;
        }
        ++index;
    }
    return std::nullopt;
    /* Alternative using STL algorithm:
    auto it = std::find(this->begin(), this->end(), val);
    if (it != this->end()) {
        return std::distance(this->begin(), it);
    }
    return std::nullopt;
    */
}

// =============== Private Helpers ===============
template <typename T>
void List<T>::appendNode(Node* newNode) noexcept {
    if (!pHead) {
        pHead = pTail = newNode;
    } else {
        pTail->next = newNode;
        pTail       = newNode;
    }
    nodeCount++;
}

template <typename T>
void List<T>::prependNode(Node* newNode) noexcept {
    if (!pHead) {
        pHead = pTail = newNode;
    } else {
        newNode->next = pHead;
        pHead         = newNode;
    }
    nodeCount++;
}

template <typename T>
List<T>::Node* List<T>::getNodeAt(size_t index) const {
    Node* curr = pHead;
    for (int i = 0; i < index; ++i) {
        curr = curr->next;
    }
    return curr;
}

template <typename T>
void List<T>::insertAfter(Node* prevNode, Node* newNode) {
    newNode->next  = prevNode->next;
    prevNode->next = newNode;
    nodeCount++;
}

template <typename T>
void List<T>::swap(List& other) noexcept {
    std::swap(pHead, other.pHead);
    std::swap(pTail, other.pTail);
    std::swap(nodeCount, other.nodeCount);
}
