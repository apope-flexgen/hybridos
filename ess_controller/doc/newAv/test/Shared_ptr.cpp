#include <iostream>
#include <cstring>
#include <vector>
#include <map>

// g++ -g -o t -std=c++11 Shared_ptr.cpp
using namespace std;

//int AssetVar::av_id = 0;

class AssetVar
{
public:
static int av_id; 

static vector<AssetVar*> aVars;
static map<AssetVar*,AssetVar*> aVmap;

    AssetVar(const char* _name, const char* _value)
    {
        name = strdup(_name);
        value = strdup(_value);
		id = av_id++;
		aVars.push_back(this);
		aVmap[this]=this;
    }
    ~AssetVar()
    {
		cout  << "Delete Av Name : " << name <<" Value : " << value << " ID : "
		<< id << " "
		<< endl;
        free((void*)name);
        free((void*)value);
		aVmap[this]=nullptr;
    }
    void show()
    {
        cout <<" name ["<<name<<"] value ["<<value<<"]"<<endl;
    }
    // Overloading << operator
	friend ostream& operator<<(ostream& os,
							const AssetVar& av)
	{
		os << "Av Name : " << av.name <<" Value : " << av.value << " ID : "<< av.id << " ";
		//<< endl;
		return os;
	}

    private:
        const char* name;
        const char* value; 
		int id;
};

// Class representing a reference counter class
class Counter
{
public:
	// Constructor
	Counter()
		: m_counter(0){};

	Counter(const Counter&) = delete;
	Counter& operator=(const Counter&) = delete;

	// Destructor
	~Counter() {}

	void reset()
	{
	m_counter = 0;
	}

	unsigned int get()
	{
	return m_counter;
	}

	// Overload post/pre increment
	void operator++()
	{
	m_counter++;
	}

	void operator++(int)
	{
	m_counter++;
	}

	// Overload post/pre decrement
	void operator--()
	{
	m_counter--;
	}
	void operator--(int)
	{
	m_counter--;
	}

	// Overloading << operator
	friend ostream& operator<<(ostream& os,
							const Counter& counter)
	{
		os << "Counter Value : " << counter.m_counter
		<< endl;
		return os;
	}

private:
	unsigned int m_counter{};
};

// Class representing a shared pointer
template <typename T>
class Shared_ptr
{
public:
	// Constructor
	explicit Shared_ptr(T* ptr = nullptr)
	{
		m_ptr = ptr;
		m_counter = new Counter();
		if (ptr)
		{
			(*m_counter)++;
		}
	}

	// Copy constructor
	Shared_ptr(Shared_ptr<T>& sp)
	{
		m_ptr = sp.m_ptr;
		m_counter = sp.m_counter;
		(*m_counter)++;
	}

	// Reference count
	unsigned int use_count()
	{
	return m_counter->get();
	}

	// Get the pointer
	T* get()
	{
	return m_ptr;
	}

    // overload =
	T& operator= (const T& sp)
	{
		cout << " Shared pointer counter "<<m_counter->get()<<endl;

		(*m_counter)--;
		if (m_counter->get() == 0)
		{
            cout << " operator = bye Sharedpointer"<<endl;
            cout << *m_ptr<<endl;
			delete m_counter;
			//delete m_ptr;
		}
		m_ptr = sp.m_ptr;
		m_counter = sp.m_counter;
		(*m_counter)++;
	}
	// Overload * operator
	T& operator*()
	{
	return *m_ptr;
	}

	// Overload -> operator
	T* operator->()
	{
	return m_ptr;
	}

	// Destructor
	~Shared_ptr()
	{
		(*m_counter)--;
		if (m_counter->get() == 0)
		{
            cout << " by bye Sharedpointer"<<endl;
            //cout << *m_ptr<<endl;
			delete m_counter;
			//delete m_ptr;
		}
	}

	friend ostream& operator<<(ostream& os,
							Shared_ptr<T>& sp)
	{
		os << "Address pointed : " << sp.get() << endl;
		os << *(sp.m_counter) << endl;
		return os;
	}

private:
	// Reference counter
	Counter* m_counter;

	// Shared pointer
	T* m_ptr;
};

int AssetVar::av_id = 0;

vector<AssetVar*>AssetVar::aVars;
map<AssetVar*, AssetVar*>AssetVar::aVmap;

int main()
{
	
	// ptr1 pointing to an integer.
	Shared_ptr<int> ptr1(new int(151));
	cout << "--- Shared pointers ptr1 ---"<< *ptr1 << endl;
	*ptr1 = 100;
	cout << " ptr1's value now: " << *ptr1 << endl;
	cout << ptr1;

    Shared_ptr<AssetVar> av1 (new AssetVar("av1","value1"));
    Shared_ptr<AssetVar> av2 (new AssetVar("av2","value2"));
	cout << " lets see av1 : " << *av1 << " ptr " << &av1<< endl;
	cout << " lets see av2 : " << *av2 << " ptr " << &av2<<endl;
	//av2 = av1;
	cout <<endl<< " lets see av2 after av2 == av1 : " << *av2 << endl;
	
	{
		// ptr2 pointing to same integer
		// which ptr1 is pointing to
		// Shared pointer reference counter
		// should have increased now to 2.
		Shared_ptr<int> ptr2 = ptr1;
		cout << "--- Shared pointers ptr1, ptr2 ---\n";
		cout << ptr1;
		cout << ptr2;

		{
			// ptr3 pointing to same integer
			// which ptr1 and ptr2 are pointing to.
			// Shared pointer reference counter
			// should have increased now to 3.
			Shared_ptr<int> ptr3(ptr2);
			cout << "--- Shared pointers ptr1, ptr2, ptr3 "
					"---\n";
			cout << ptr1;
			cout << ptr2;
			cout << ptr3;
		}

		// ptr3 is out of scope.
		// It would have been destructed.
		// So shared pointer reference counter
		// should have decreased now to 2.
		cout << "--- Shared pointers ptr1, ptr2 ---\n";
		cout << ptr1;
		cout << ptr2;
	}

	// ptr2 is out of scope.
	// It would have been destructed.
	// So shared pointer reference counter
	// should have decreased now to 1.
	cout << "--- Shared pointers ptr1 ---\n";
	cout << ptr1;

    delete av2.get();
	cout << " AssetMap size :"<< AssetVar::aVmap.size()<<endl; 

	for (auto x : AssetVar::aVmap)
	{
		cout << " Key :"<< x.first << " Av :"<< x.second <<endl;
	}
	cout << " AssetVars size :"<< AssetVar::aVars.size()<<endl; 

	for (auto x : AssetVar::aVars)
	{
		if(AssetVar::aVmap[x])
		{
			cout << " yyKey :" << x << " running delete" << endl;
			delete x;
		}
		else
		{
			cout << " yyKey :" << x << " skipping delete" << endl;
 		}
	}
	cout << " After delete AssetMap size :"<< AssetVar::aVmap.size()<<endl; 

	for (auto x : AssetVar::aVmap)
	{
		cout << " xxKey :"<< x.first << " Av :"<< x.second <<endl;
	}

	return 0;
}
