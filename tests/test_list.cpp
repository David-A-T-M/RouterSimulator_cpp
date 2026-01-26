#include <gtest/gtest.h>
#include <string>
#include <utility>
#include "structures/list.h"

// Structs for Testing ===============
struct DestructorSpy {
    inline static int count = 0;

    DestructorSpy() { count++; }
    DestructorSpy(const DestructorSpy&) { count++; }
    DestructorSpy(DestructorSpy&&) noexcept { count++; }
    ~DestructorSpy() { count--; }
};

struct Person {
    std::string name;
    int age;

    Person() : age(0) {}
    Person(std::string n, int a) : name(std::move(n)), age(a) {}

    bool operator==(const Person& other) const { return name == other.name && age == other.age; }

    friend std::ostream& operator<<(std::ostream& os, const Person& p) {
        os << p.name << "(" << p.age << ")";
        return os;
    }
};

struct Resource {
    int* data;

    explicit Resource(int val = 0) : data(new int(val)) {}
    Resource(const Resource& other) : data(new int(*other.data)) {}
    Resource(Resource&& other) noexcept : data(other.data) { other.data = nullptr; }
    Resource& operator=(const Resource& other) {
        if (this != &other) {
            const auto new_data = new int(*other.data);
            delete data;
            data = new_data;
        }
        return *this;
    }
    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            delete data;
            data       = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    ~Resource() { delete data; }

    bool operator==(const Resource& other) const {
        if (!data && !other.data)
            return true;
        if (!data || !other.data)
            return false;
        return *data == *other.data;
    }

    friend std::ostream& operator<<(std::ostream& os, const Resource& r) {
        if (r.data) {
            os << "Resource(" << *r.data << ")";
        } else {
            os << "Resource(null)";
        }
        return os;
    }
};

// =============== Constructors and assignment tests ===============
TEST(ListConstructors, DefaultConstructor) {
    const List<int> list;
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(ListConstructors, CopyConstructor) {
    List<int> list1;
    list1.pushBack(1);
    list1.pushBack(2);
    list1.pushBack(3);

    List list2(list1);

    EXPECT_EQ(list2.size(), 3);
    EXPECT_EQ(list2[0], 1);
    EXPECT_EQ(list2[1], 2);
    EXPECT_EQ(list2[2], 3);

    list1[0] = 100;
    EXPECT_EQ(list1[0], 100);
    EXPECT_EQ(list2[0], 1);
}

TEST(ListConstructors, CopyConstructorEmpty) {
    const List<int> list1;
    List list2(list1);

    EXPECT_TRUE(list2.isEmpty());
    EXPECT_EQ(list2.size(), 0);
}

TEST(ListConstructors, MoveConstructor) {
    List<int> list1;
    list1.pushBack(1);
    list1.pushBack(2);
    list1.pushBack(3);

    List list2(std::move(list1));

    EXPECT_EQ(list2.size(), 3);
    EXPECT_EQ(list2[0], 1);
    EXPECT_EQ(list2[1], 2);
    EXPECT_EQ(list2[2], 3);

    EXPECT_TRUE(list1.isEmpty());
    EXPECT_EQ(list1.size(), 0);
}

TEST(ListConstructors, CopyAssignmentOperator) {
    List<int> list1;
    list1.pushBack(10);
    list1.pushBack(20);

    List<int> list2;
    list2.pushBack(99);

    list2 = list1;

    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list2[0], 10);
    EXPECT_EQ(list2[1], 20);

    list1[0] = 100;
    EXPECT_EQ(list1[0], 100);
    EXPECT_EQ(list2[0], 10);
}

TEST(ListConstructors, CopyAssignmentSelfAssignment) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);

    list = list;

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list[0], 1);
    EXPECT_EQ(list[1], 2);
}

TEST(ListConstructors, MoveAssignmentOperator) {
    List<int> list1;
    list1.pushBack(10);
    list1.pushBack(20);
    list1.pushBack(30);

    List<int> list2;
    list2.pushBack(99);

    list2 = std::move(list1);

    EXPECT_EQ(list2.size(), 3);
    EXPECT_EQ(list2[0], 10);
    EXPECT_EQ(list2[1], 20);
    EXPECT_EQ(list2[2], 30);

    EXPECT_EQ(list1.size(), 1);
    EXPECT_EQ(list1[0], 99);
}

TEST(ListConstructors, MoveAssignmentSelfAssignment) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);

    list = std::move(list);

    EXPECT_EQ(list.size(), 2);
}

// =============== Destructor tests ===============
TEST(ListDestructor, DestructorActuallyDeletesNodes) {
    DestructorSpy::count = 0;
    {
        List<DestructorSpy> list;
        list.pushBack(DestructorSpy());
        list.pushBack(DestructorSpy());
        list.pushBack(DestructorSpy());

        EXPECT_EQ(DestructorSpy::count, 3);
    }

    EXPECT_EQ(DestructorSpy::count, 0);
}

// =============== Capacity tests ===============
TEST(ListCapacity, Size) {
    List<int> list;
    EXPECT_EQ(list.size(), 0);

    list.pushBack(1);
    EXPECT_EQ(list.size(), 1);

    list.pushBack(2);
    list.pushBack(3);
    EXPECT_EQ(list.size(), 3);
}

TEST(ListCapacity, IsEmpty) {
    List<int> list;
    EXPECT_TRUE(list.isEmpty());

    list.pushBack(1);
    EXPECT_FALSE(list.isEmpty());

    list.popFront();
    EXPECT_TRUE(list.isEmpty());
}

// =============== Element Access tests ===============
TEST(ListElementAccess, GetHeadValid) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    EXPECT_EQ(list.getHead(), 10);

    list.pushFront(5);

    EXPECT_EQ(list.getHead(), 5);
}

TEST(ListElementAccess, GetHeadEmpty) {
    List<int> list;
    EXPECT_THROW((void)list.getHead(), std::runtime_error);

    list.pushBack(1);
    list.popFront();
    EXPECT_THROW((void)list.getHead(), std::runtime_error);
}

TEST(ListElementAccess, GetHeadModify) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    list.getHead() = 99;

    EXPECT_EQ(list.getHead(), 99);
}

TEST(ListElementAccess, GetHeadConst) {
    List<int> list;
    list.pushBack(42);

    const List<int>& constList = list;
    EXPECT_EQ(constList.getHead(), 42);
}

TEST(ListElementAccess, GetTailValid) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    EXPECT_EQ(list.getTail(), 20);

    list.pushBack(30);

    EXPECT_EQ(list.getTail(), 30);
}

TEST(ListElementAccess, GetTailEmpty) {
    List<int> list;
    EXPECT_THROW((void)list.getTail(), std::runtime_error);
}

TEST(ListElementAccess, GetTailModify) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    list.getTail() = 99;

    EXPECT_EQ(list.getTail(), 99);
}

TEST(ListElementAccess, GetTailConst) {
    List<int> list;
    list.pushBack(42);
    list.pushBack(99);

    const List<int>& constList = list;
    EXPECT_EQ(constList.getTail(), 99);
}

TEST(ListElementAccess, GetAtValid) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    EXPECT_EQ(list.getAt(0), 10);
    EXPECT_EQ(list.getAt(1), 20);
    EXPECT_EQ(list.getAt(2), 30);
}

TEST(ListElementAccess, GetAtInvalid) {
    List<int> list;
    list.pushBack(10);

    EXPECT_THROW((void)list.getAt(-1), std::out_of_range);
    EXPECT_THROW((void)list.getAt(1), std::out_of_range);
    EXPECT_THROW((void)list.getAt(100), std::out_of_range);
}

TEST(ListElementAccess, OperatorBracket) {
    List<int> list;
    list.pushBack(5);
    list.pushBack(15);
    list.pushBack(25);

    EXPECT_EQ(list[0], 5);
    EXPECT_EQ(list[1], 15);
    EXPECT_EQ(list[2], 25);

    list[1] = 100;
    EXPECT_EQ(list[1], 100);
}

TEST(ListElementAccess, OperatorBracketInvalidPos) {
    List<int> list;
    list.pushBack(10);

    EXPECT_THROW((void)list[99], std::out_of_range);
}

TEST(ListElementAccess, OperatorBracketConst) {
    List<int> list;
    list.pushBack(7);
    list.pushBack(14);

    const List<int>& constList = list;
    EXPECT_EQ(constList[0], 7);
    EXPECT_EQ(constList[1], 14);
}

// ===============  Modifiers tests ===============
TEST(ListModifiers, Clear) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);

    EXPECT_EQ(list.size(), 3);

    list.clear();
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(ListModifiers, ClearEmpty) {
    List<int> list;
    list.clear();
    EXPECT_TRUE(list.isEmpty());
}

TEST(ListModifiers, PushFrontLValue) {
    List<int> list;
    const int val1 = 10;
    const int val2 = 20;

    list.pushFront(val1);
    EXPECT_EQ(list.getHead(), 10);
    EXPECT_EQ(list.size(), 1);

    list.pushFront(val2);
    EXPECT_EQ(list.getHead(), 20);
    EXPECT_EQ(list.size(), 2);
}

TEST(ListModifiers, PushFrontRValue) {
    List<std::string> list;

    list.pushFront(std::string("Hello"));
    EXPECT_EQ(list.getHead(), "Hello");

    list.pushFront(std::string("World"));
    EXPECT_EQ(list.getHead(), "World");
}

TEST(ListModifiers, PushFrontMoveSemantics) {
    List<std::string> list;
    std::string str = "Long String to move";

    list.pushFront(std::move(str));

    EXPECT_EQ(list.getHead(), "Long String to move");

    EXPECT_TRUE(str.empty());
}

TEST(ListModifiers, PushBackLValue) {
    List<int> list;
    const int val1 = 10;
    const int val2 = 20;

    list.pushBack(val1);
    EXPECT_EQ(list.getTail(), 10);
    EXPECT_EQ(list.size(), 1);

    list.pushBack(val2);
    EXPECT_EQ(list.getTail(), 20);
    EXPECT_EQ(list.size(), 2);
}

TEST(ListModifiers, PushBackRValue) {
    List<std::string> list;

    list.pushBack(std::string("First"));
    EXPECT_EQ(list.getTail(), "First");

    list.pushBack(std::string("Second"));
    EXPECT_EQ(list.getTail(), "Second");
}

TEST(ListModifiers, PushBackMoveSemantics) {
    List<std::string> list;
    std::string str = "Long String to move";

    list.pushBack(std::move(str));

    EXPECT_EQ(list.getHead(), "Long String to move");

    EXPECT_TRUE(str.empty());
}

TEST(ListModifiers, InsertAtBeginning) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    list.insertAt(5, 0);
    EXPECT_EQ(list[0], 5);
    EXPECT_EQ(list[1], 10);
    EXPECT_EQ(list.size(), 3);
}

TEST(ListModifiers, InsertAtEnd) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    list.insertAt(30, 2);
    EXPECT_EQ(list[2], 30);
    EXPECT_EQ(list.size(), 3);
}

TEST(ListModifiers, InsertAtMiddle) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(30);

    list.insertAt(20, 1);
    EXPECT_EQ(list[0], 10);
    EXPECT_EQ(list[1], 20);
    EXPECT_EQ(list[2], 30);
}

TEST(ListModifiers, InsertAtInvalid) {
    List<int> list;
    list.pushBack(10);

    EXPECT_THROW(list.insertAt(5, -1), std::out_of_range);
    EXPECT_THROW(list.insertAt(5, 5), std::out_of_range);
}

TEST(ListModifiers, InsertAtRValue) {
    List<std::string> list;
    list.pushBack("A");
    list.pushBack("C");

    list.insertAt(std::string("B"), 1);
    EXPECT_EQ(list[1], "B");
}

TEST(ListModifiers, PopFront) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    list.popFront();
    EXPECT_EQ(list.getHead(), 20);
    EXPECT_EQ(list.size(), 2);

    list.popFront();
    EXPECT_EQ(list.getHead(), 30);
    EXPECT_EQ(list.size(), 1);
}

TEST(ListModifiers, PopFrontEmpty) {
    List<int> list;
    EXPECT_THROW(list.popFront(), std::runtime_error);
}

TEST(ListModifiers, PopFrontSingleElement) {
    List<int> list;
    list.pushBack(42);

    list.popFront();
    EXPECT_TRUE(list.isEmpty());
}

TEST(ListModifiers, PopBack) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    list.popBack();
    EXPECT_EQ(list.getTail(), 20);
    EXPECT_EQ(list.size(), 2);

    list.popBack();
    EXPECT_EQ(list.getTail(), 10);
    EXPECT_EQ(list.size(), 1);
}

TEST(ListModifiers, PopBackEmpty) {
    List<int> list;
    EXPECT_THROW(list.popBack(), std::runtime_error);
}

TEST(ListModifiers, PopBackSingleElement) {
    List<int> list;
    list.pushBack(42);

    list.popBack();
    EXPECT_TRUE(list.isEmpty());
}

TEST(ListModifiers, RemoveAtBeginning) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    list.removeAt(0);
    EXPECT_EQ(list[0], 20);
    EXPECT_EQ(list.size(), 2);
}

TEST(ListModifiers, RemoveAtEnd) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    list.removeAt(2);
    EXPECT_EQ(list.getTail(), 20);
    EXPECT_EQ(list.size(), 2);
}

TEST(ListModifiers, RemoveAtMiddle) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    list.removeAt(1);
    EXPECT_EQ(list[0], 10);
    EXPECT_EQ(list[1], 30);
    EXPECT_EQ(list.size(), 2);
}

TEST(ListModifiers, RemoveAtInvalid) {
    List<int> list;
    list.pushBack(10);

    EXPECT_THROW(list.removeAt(-1), std::out_of_range);
    EXPECT_THROW(list.removeAt(1), std::out_of_range);
}

TEST(ListModifiers, Swap) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    list.swap(0, 2);
    EXPECT_EQ(list[0], 30);
    EXPECT_EQ(list[2], 10);
}

TEST(ListModifiers, SwapSamePosition) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    list.swap(0, 0);
    EXPECT_EQ(list[0], 10);
}

TEST(ListModifiers, SwapInvalid) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    EXPECT_THROW(list.swap(-1, 0), std::out_of_range);
    EXPECT_THROW(list.swap(0, 5), std::out_of_range);
}

TEST(ListModifiers, ReverseMultipleElements) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.pushBack(4);

    list.reverse();

    EXPECT_EQ(list[0], 4);
    EXPECT_EQ(list[1], 3);
    EXPECT_EQ(list[2], 2);
    EXPECT_EQ(list[3], 1);
}

TEST(ListModifiers, ReverseEmpty) {
    List<int> list;
    list.reverse();
    EXPECT_TRUE(list.isEmpty());
}

TEST(ListModifiers, ReverseSingleElement) {
    List<int> list;
    list.pushBack(42);

    list.reverse();
    EXPECT_EQ(list[0], 42);
}

// =============== Iterators tests ===============
TEST(ListIterators, IteratorBeginEnd) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);

    auto it = list.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_TRUE(it == list.end());
}

TEST(ListIterators, IteratorForLoop) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    int sum = 0;
    for (const auto& val : list) {
        sum += val;
    }
    EXPECT_EQ(sum, 60);
}

TEST(ListIterators, ConstIterator) {
    List<int> list;
    list.pushBack(5);
    list.pushBack(10);

    const List<int>& constList = list;

    int sum   = 0;
    int count = 0;
    for (const auto& val : constList) {
        count++;
        sum += val;
    }
    EXPECT_EQ(count, 2);
    EXPECT_EQ(sum, 15);
}

TEST(ListIterators, IteratorModification) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);

    for (auto& val : list) {
        val *= 2;
    }

    EXPECT_EQ(list[0], 2);
    EXPECT_EQ(list[1], 4);
    EXPECT_EQ(list[2], 6);
}

// =============== Utilities tests ===============
TEST(ListUtilities, ToStringEmpty) {
    const List<int> list;
    EXPECT_EQ(list.toString(), "List is empty");
}

TEST(ListUtilities, ToStringSingleElement) {
    List<int> list;
    list.pushBack(42);
    EXPECT_EQ(list.toString(), "42");
}

TEST(ListUtilities, ToStringMultipleElements) {
    List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    EXPECT_EQ(list.toString(), "1 -> 2 -> 3");
}

TEST(ListUtilities, ToStringWithStrings) {
    List<std::string> list;
    list.pushBack("Hello");
    list.pushBack("World");
    EXPECT_EQ(list.toString(), "Hello -> World");
}

TEST(ListUtilities, ContainsFound) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    EXPECT_TRUE(list.contains(10));
    EXPECT_TRUE(list.contains(20));
    EXPECT_TRUE(list.contains(30));
}

TEST(ListUtilities, ContainsNotFound) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    EXPECT_FALSE(list.contains(99));
    EXPECT_FALSE(list.contains(0));
}

TEST(ListUtilities, ContainsEmpty) {
    List<int> list;
    EXPECT_FALSE(list.contains(42));
}

TEST(ListUtilities, FindFound) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);
    list.pushBack(30);

    EXPECT_EQ(list.find(10), 0);
    EXPECT_EQ(list.find(20), 1);
    EXPECT_EQ(list.find(30), 2);
}

TEST(ListUtilities, FindNotFound) {
    List<int> list;
    list.pushBack(10);
    list.pushBack(20);

    EXPECT_EQ(list.find(99), std::nullopt);
}

TEST(ListUtilities, FindEmpty) {
    const List<int> list;
    EXPECT_EQ(list.find(42), std::nullopt);
}

// =============== Complex tests ===============
TEST(ListComplex, MixedOperations) {
    List<int> list;

    for (int i = 0; i < 50; i++) {
        list.pushBack(i);
    }
    EXPECT_EQ(list.size(), 50);

    list.insertAt(99, 2);
    EXPECT_EQ(list[2], 99);
    list.insertAt(100, 40);
    EXPECT_EQ(list[40], 100);
    EXPECT_EQ(list[50], 48);
    EXPECT_EQ(list[51], 49);

    list.removeAt(0);
    list.removeAt(list.size() - 1);

    EXPECT_EQ(list.size(), 50);
    EXPECT_EQ(list.getHead(), 1);
    EXPECT_EQ(list.getTail(), 48);
    list.reverse();
    EXPECT_EQ(list.getHead(), 48);
    EXPECT_EQ(list.getTail(), 1);

    EXPECT_FALSE(list.isEmpty());
}

TEST(ListComplex, StressTest) {
    List<int> list;

    for (int i = 0; i < 10000; i++) {
        list.pushBack(i);
    }
    EXPECT_EQ(list.size(), 10000);

    for (int i = 0; i < 10000; i++) {
        list.popFront();
    }
    EXPECT_TRUE(list.isEmpty());
}

// =============== Person Struct Tests ===============
TEST(ListPerson, PersonBasicOperations) {
    List<Person> people;

    people.pushBack(Person("Alice", 25));
    people.pushBack(Person("Bob", 30));
    people.pushBack(Person("Charlie", 35));

    EXPECT_EQ(people.size(), 3);
    EXPECT_EQ(people[0].name, "Alice");
    EXPECT_EQ(people[1].age, 30);
    EXPECT_EQ(people[2].name, "Charlie");
}

TEST(ListPerson, PersonContainsAndFind) {
    List<Person> people;

    const Person alice("Alice", 25);
    const Person bob("Bob", 30);
    const Person charlie("Charlie", 35);

    people.pushBack(alice);
    people.pushBack(bob);

    EXPECT_TRUE(people.contains(alice));
    EXPECT_TRUE(people.contains(bob));
    EXPECT_FALSE(people.contains(charlie));

    EXPECT_EQ(people.find(alice), 0);
    EXPECT_EQ(people.find(bob), 1);
    EXPECT_EQ(people.find(charlie), std::nullopt);
}

TEST(ListPerson, PersonToString) {
    List<Person> people;

    people.pushBack(Person("Alice", 25));
    people.pushBack(Person("Bob", 30));

    const std::string expected = "Alice(25) -> Bob(30)";
    EXPECT_EQ(people.toString(), expected);
}

TEST(ListPerson, PersonSwapAndReverse) {
    List<Person> people;

    people.pushBack(Person("First", 1));
    people.pushBack(Person("Second", 2));
    people.pushBack(Person("Third", 3));

    people.swap(0, 2);
    EXPECT_EQ(people[0].name, "Third");
    EXPECT_EQ(people[2].name, "First");

    people.reverse();
    EXPECT_EQ(people[0].name, "First");
    EXPECT_EQ(people[1].name, "Second");
    EXPECT_EQ(people[2].name, "Third");
}

TEST(ListPerson, PersonIterator) {
    List<Person> people;

    people.pushBack(Person("Alice", 25));
    people.pushBack(Person("Bob", 30));
    people.pushBack(Person("Charlie", 35));

    int totalAge = 0;
    for (const auto& person : people) {
        totalAge += person.age;
    }

    EXPECT_EQ(totalAge, 90);
}

TEST(ListPerson, PersonModifyViaIterator) {
    List<Person> people;

    people.pushBack(Person("Alice", 25));
    people.pushBack(Person("Bob", 30));

    for (auto& person : people) {
        person.age += 1;
    }

    EXPECT_EQ(people[0].age, 26);
    EXPECT_EQ(people[1].age, 31);
}

TEST(ListPerson, ComplexStructOperations) {
    List<Person> team;

    team.pushBack(Person("Manager", 45));
    team.pushBack(Person("Developer1", 28));
    team.pushBack(Person("Developer2", 32));
    team.pushBack(Person("Designer", 26));

    EXPECT_EQ(team.size(), 4);

    team.removeAt(1);
    EXPECT_EQ(team.size(), 3);
    EXPECT_EQ(team[1].name, "Developer2");

    team.insertAt(Person("Intern", 22), 1);
    EXPECT_EQ(team.size(), 4);
    EXPECT_EQ(team[1].name, "Intern");

    team.reverse();
    EXPECT_EQ(team[0].name, "Designer");
    EXPECT_EQ(team[3].name, "Manager");
}

TEST(ListPerson, EmptyListWithStruct) {
    List<Person> people;

    EXPECT_TRUE(people.isEmpty());
    EXPECT_FALSE(people.contains(Person("Nobody", 0)));
    EXPECT_EQ(people.find(Person("Nobody", 0)), std::nullopt);
    EXPECT_EQ(people.toString(), "List is empty");
}

// =============== Resource Struct Tests ===============
TEST(ListResource, ResourceMoveSemantics) {
    List<Resource> resources;

    resources.pushBack(Resource(100));
    resources.pushBack(Resource(200));

    EXPECT_EQ(*resources[0].data, 100);
    EXPECT_EQ(*resources[1].data, 200);
}

TEST(ListResource, ResourceCopySemantics) {
    List<Resource> resources;

    const Resource r1(42);
    const Resource r2(99);

    resources.pushBack(r1);
    resources.pushBack(r2);

    EXPECT_EQ(*resources[0].data, 42);
    EXPECT_EQ(*resources[1].data, 99);

    *r1.data = 1000;
    EXPECT_EQ(*resources[0].data, 42);
}

TEST(ListResource, ResourceInsertMove) {
    List<Resource> resources;

    resources.pushBack(Resource(10));
    resources.pushBack(Resource(30));

    resources.insertAt(Resource(20), 1);

    EXPECT_EQ(*resources[0].data, 10);
    EXPECT_EQ(*resources[1].data, 20);
    EXPECT_EQ(*resources[2].data, 30);
}

TEST(ListResource, ResourcePushFrontMove) {
    List<Resource> resources;

    resources.pushFront(Resource(1));
    resources.pushFront(Resource(2));
    resources.pushFront(Resource(3));

    EXPECT_EQ(*resources[0].data, 3);
    EXPECT_EQ(*resources[1].data, 2);
    EXPECT_EQ(*resources[2].data, 1);
}

TEST(ListResource, ResourceContains) {
    List<Resource> resources;

    resources.pushBack(Resource(10));
    resources.pushBack(Resource(20));
    resources.pushBack(Resource(30));

    const Resource search(20);
    EXPECT_TRUE(resources.contains(search));

    const Resource notFound(99);
    EXPECT_FALSE(resources.contains(notFound));
}

// =============== Nested List Tests ===============
TEST(ListNested, ListOfListsBasic) {
    List<List<int>> matrix;

    List<int> row1;
    row1.pushBack(1);
    row1.pushBack(2);
    row1.pushBack(3);

    List<int> row2;
    row2.pushBack(4);
    row2.pushBack(5);
    row2.pushBack(6);
    row2.pushBack(7);

    matrix.pushBack(std::move(row1));
    matrix.pushBack(std::move(row2));

    EXPECT_EQ(matrix.size(), 2);
    EXPECT_EQ(matrix[0].size(), 3);
    EXPECT_EQ(matrix[1].size(), 4);
    EXPECT_EQ(matrix[0][0], 1);
    EXPECT_EQ(matrix[1][2], 6);
}

TEST(ListNested, ListOfListsCopyConstructor) {
    List<List<int>> matrix1;

    List<int> row;
    row.pushBack(10);
    row.pushBack(20);
    matrix1.pushBack(row);

    List matrix2(matrix1);

    EXPECT_EQ(matrix2.size(), 1);
    EXPECT_EQ(matrix2[0].size(), 2);
    EXPECT_EQ(matrix2[0][0], 10);

    matrix1[0][0] = 999;
    EXPECT_EQ(matrix2[0][0], 10);
}

TEST(ListNested, ListOfListsMoveConstructor) {
    List<List<int>> matrix1;

    List<int> row1;
    row1.pushBack(1);
    row1.pushBack(2);

    List<int> row2;
    row2.pushBack(3);
    row2.pushBack(4);

    matrix1.pushBack(std::move(row1));
    matrix1.pushBack(std::move(row2));

    List matrix2(std::move(matrix1));

    EXPECT_EQ(matrix2.size(), 2);
    EXPECT_EQ(matrix2[0][1], 2);
    EXPECT_EQ(matrix2[1][0], 3);
    EXPECT_TRUE(matrix1.isEmpty());
}

TEST(ListNested, ListOfListsAssignment) {
    List<List<int>> matrix1;
    List<int> row;
    row.pushBack(100);
    matrix1.pushBack(row);

    List<List<int>> matrix2 = matrix1;

    EXPECT_EQ(matrix2[0][0], 100);

    matrix1[0][0] = 999;
    EXPECT_EQ(matrix2[0][0], 100);
}

TEST(ListNested, ListOfListsIteration) {
    List<List<int>> matrix;

    for (int i = 0; i < 3; i++) {
        List<int> row;
        for (int j = 0; j < 3; j++) {
            row.pushBack(i * 3 + j);
        }
        matrix.pushBack(row);
    }

    int sum = 0;
    for (const auto& row : matrix) {
        for (const auto& val : row) {
            sum += val;
        }
    }

    EXPECT_EQ(sum, 36);
}

TEST(ListNested, ListOfListsModification) {
    List<List<int>> matrix;

    List<int> row1;
    row1.pushBack(1);
    row1.pushBack(2);

    List<int> row2;
    row2.pushBack(3);
    row2.pushBack(4);

    matrix.pushBack(std::move(row1));
    matrix.pushBack(std::move(row2));

    matrix[0][1] = 100;
    EXPECT_EQ(matrix[0][1], 100);

    List<int> row3;
    row3.pushBack(5);
    row3.pushBack(6);
    matrix.pushBack(std::move(row3));

    EXPECT_EQ(matrix.size(), 3);
    EXPECT_EQ(matrix[2][0], 5);
}

TEST(ListNested, ListOfListsClear) {
    List<List<int>> matrix;

    for (int i = 0; i < 5; i++) {
        List<int> row;
        row.pushBack(i);
        matrix.pushBack(row);
    }

    EXPECT_EQ(matrix.size(), 5);

    matrix.clear();
    EXPECT_TRUE(matrix.isEmpty());
    EXPECT_EQ(matrix.size(), 0);
}

TEST(ListNested, ThreeLevelNesting) {
    List<List<List<int>>> cube;

    List<List<int>> plane;
    List<int> line;
    line.pushBack(42);
    plane.pushBack(line);
    cube.pushBack(plane);

    EXPECT_EQ(cube.size(), 1);
    EXPECT_EQ(cube[0].size(), 1);
    EXPECT_EQ(cube[0][0].size(), 1);
    EXPECT_EQ(cube[0][0][0], 42);
}
