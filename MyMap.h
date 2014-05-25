#ifndef MYMAP_INCLUDED
#define MYMAP_INCLUDED

#include <iostream>
#include <string>
#include <queue>

template <class KeyType, class ValueType>
class MyMap
{
private:
	struct BSTNODE
	{
		// By definition the BSTNODE constructor will only be called with parameters given
		BSTNODE(KeyType keyInit, ValueType valueInit)
		{
			left = right = parent = nullptr;
			key = keyInit;
			value = valueInit;
		}
		BSTNODE* left;
		BSTNODE* right;
		BSTNODE* parent;
		KeyType key;
		ValueType value;
	};

public:
	MyMap()
	{
		m_root = nullptr;
		m_nodeCounter = 0;

		// Initialize m_valid to false since the tree is empty upon calling the constructor.
		m_valid = false;
	}

	~MyMap()
	{
		clear();
	}

	void clear()
	{
		BSTNODE* temp = m_root;
		freeTree(temp);
	}

	int size() const
	{
		return m_nodeCounter;
	}

	void associate(const KeyType& key, const ValueType& value)
	{
		// Check if tree is empty
		if (m_root == nullptr)
		{
			BSTNODE* temp = new BSTNODE(key, value);
			m_root = temp;
			m_nodeCounter++;
			m_valid = true;
			return;
		}

		BSTNODE* cur = m_root;
		for (;;)
		{
			// Duplicate values will be swapped for the newest value
			if (key == cur->key)
			{
				cur->value = value;
				return;
			}

			else if (key < cur->key)
			{
				if (cur->left != nullptr)
					cur = cur->left;
				else
				{
					BSTNODE* newNode = new BSTNODE(key, value);
					cur->left = newNode;
					newNode->parent = cur;
					m_nodeCounter++;
					return;
				}
			}

			else  // key > cur->key
			{
				if (cur->right != nullptr)
					cur = cur->right;
				else
				{
					BSTNODE* newNode = new BSTNODE(key, value);
					cur->right = newNode;
					newNode->parent = cur;
					m_nodeCounter++;
					return;
				}
			}
		}
	}

	const ValueType* find(const KeyType& key) const
	{
		BSTNODE* cur = m_root;

		while (cur != nullptr)
		{
			if (key == cur->key)
			{
				ValueType* valPtr;
				valPtr = &cur->value;
				return valPtr;
			}

			else if (key < cur->key)
				cur = cur->left;

			else
				cur = cur->right;
		}

		return nullptr;
	}

	ValueType* find(const KeyType& key)
	{
		// Do not change the implementation of this overload of find
		const MyMap<KeyType, ValueType>* constThis = this;
		return const_cast<ValueType*>(constThis->find(key));
	}

	ValueType* getFirst(KeyType& key)
	{
		if (m_root == nullptr)
			return nullptr;

		BSTNODE* temp = m_root;

		// Using a queue to complete a level-order traversal
		addChildrenNodesToQueue(temp);

		ValueType* getFirstValue;
		getFirstValue = &temp->value;
		key = temp->key;
		return getFirstValue;
	}

	ValueType* getNext(KeyType& key)
	{
		if (m_traverseQueue.empty())
			return nullptr;

		// Dequeue top node pointer and process
		BSTNODE* temp = m_traverseQueue.front();
		addChildrenNodesToQueue(temp);
		m_traverseQueue.pop();

		ValueType* getValue;
		getValue = &temp->value;
		key = temp->key;
		return getValue;
	}

	// Test printing
	// TODO: Remove 
	void testPrintInit()
	{
		BSTNODE* cur = m_root;
		testPrint(cur);
	}

	void testPrint(BSTNODE* cur)
	{
		if (cur == nullptr)
			return;

		testPrint(cur->left);

		std::cerr << cur->key << " " << cur->value << std::endl;

		testPrint(cur->right);
	}

private:
	MyMap(const MyMap &other);
	MyMap &operator=(const MyMap &other);

	// Private methods
	void freeTree(BSTNODE* cur);
	bool isValid();
	void addChildrenNodesToQueue(BSTNODE* cur);

	// Private data members
	BSTNODE* m_root;
	unsigned int m_nodeCounter;
	bool m_valid;
	std::queue<BSTNODE*> m_traverseQueue;
};

// Externally defined functions
template <class KeyType, class ValueType>
void MyMap<KeyType, ValueType>::freeTree(BSTNODE* cur)
{
	if (cur == nullptr)
		return;

	freeTree(cur->left);
	freeTree(cur->right);

	delete cur;
}

template <class KeyType, class ValueType>
bool MyMap<KeyType, ValueType>::isValid()
{
	return m_valid;
}

template <class KeyType, class ValueType>
void MyMap<KeyType, ValueType>::addChildrenNodesToQueue(BSTNODE* cur)
{
	if (cur->left != nullptr)
		m_traverseQueue.push(cur->left);

	if (cur->right != nullptr)
		m_traverseQueue.push(cur->right);
}

#endif // MYMAP_INCLUDED
