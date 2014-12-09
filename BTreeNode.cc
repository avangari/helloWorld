#include "BTreeNode.h"
#include  <math.h>
#include <iostream>

using namespace std;

BTLeafNode::BTLeafNode()
{
	bzero(buffer, PageFile::PAGE_SIZE);
	insertSplit = false;
	//std:cout << "done with constructor\n";
	//std::cout << MAX_KEYS;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	m_pid = pid;
	return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	// read first LeafElement from buffer (12 bytes)
	LeafElement* temp = (LeafElement *) buffer;
	return temp->key; // first LeafElement's key will hold the count value
}

int BTLeafNode::updateCount(int count)
{
	LeafElement* temp = (LeafElement *) buffer;
	temp->key = count;
	return 0;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	//std::cout << "start of insert\n";
	if(getKeyCount() >= MAX_KEYS && insertSplit == false) // node is full
		return RC_NODE_FULL;
	else // node is not full
	{
		int eid;
		int result = locate(key, eid);
		LeafElement* temp;// = (LeafElement *) buffer;
		if(result == -1) // append to end of node
		{
			temp = (LeafElement *) buffer + (getKeyCount()+1);
			temp->key = key;
			temp->rid = rid;
			//std::cout << "added to end of node\n";
		}
		else if(result == 0)// locate did return a value
		{
			// shift the elements to the right
			LeafElement* startPosition = (LeafElement *)buffer + eid;
			LeafElement* endPosition = (LeafElement *)buffer + (getKeyCount()+1);
			for(int i = (getKeyCount()+1); i > eid; i--)
			{
				LeafElement* temp = endPosition - 1;
				*endPosition = *temp;
				endPosition = temp;
			}

			// insert key and rid into new element
			temp = (LeafElement *) buffer + eid;
			temp->key = key;
			temp->rid = rid;
			printf("shit with key = %d is inserted (%d, %d)\n", key, m_pid, eid); 
		}
		else
			return -1;

		// update number of keys in node
		if(getKeyCount() <= MAX_KEYS && insertSplit == false)
		{
			temp = (LeafElement *) buffer;
			int numOfKeys = temp->key;
			numOfKeys++;
			temp->key = numOfKeys;
		}
	}
	return 0; 
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey)
{ 
	if(sibling.getKeyCount() != 0)
		return -1;

	insertSplit = true;
	if(insert(key, rid) == 0)
	{
		// split the node into two nodes
		int middle;
		int siblingNumOfKeys = 0;
		if((getKeyCount()+1) %2 != 0){
			middle = ceil(((double)getKeyCount()+1)/((double)2));
			//std::cout << getKeyCount();
			std::cout << "Odd number of nodes: middle: ";
			std::cout << middle;
			std::cout << endl;
		}
		else
			middle = (getKeyCount()+1)/2+1;
		LeafElement* temp;
		for(int i = middle; i <= getKeyCount()+1; i++)
		{
			temp = (LeafElement *) buffer+i;
			int tempKey = temp->key;
			RecordId tempRid = temp->rid;
			sibling.insert(tempKey, tempRid);
			siblingNumOfKeys++;
			if(i == middle)
				siblingKey = tempKey;
		}

		// update the count
		temp = (LeafElement *) buffer;
		int numOfKeys = temp->key;
		numOfKeys = middle - 1;
		temp->key = numOfKeys;

		//update suibling count
		sibling.updateCount(siblingNumOfKeys);
		insertSplit = false;
		return 0;
	}
	else
	{
		insertSplit = false;
		return -2;
	}
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	if(getKeyCount() == 0)
		return -1;

	LeafElement* temp = (LeafElement *) buffer +1; // start at element 1 because 0 is used for other info
	int count = 1; // start at 1, because 0 is where we store other info

	while(count <= getKeyCount())
	{
		if(temp->key >= searchKey)
		{
			eid = count;
			return 0;
		}
		count++;
		temp = (LeafElement *) buffer + count;
	}
	eid = -1;
	return -1;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
	if(eid <= 0 || eid >= getKeyCount())
		return -1;
	else
	{
		LeafElement* temp = (LeafElement *) buffer + eid;
		key = temp->key;
		rid = temp->rid;
	}
	return 0; 
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	LeafElement* temp = (LeafElement *) buffer;
	return temp->rid.pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	LeafElement* temp = (LeafElement *) buffer;
	temp->rid.pid = pid;
	return 0; 
}

int BTLeafNode::printBuffer()
{
	for(int i = 1; i <= getKeyCount(); i++)
	{ /*
		 LeafElement* temp = (LeafElement *) buffer+i;
		 std::cout << temp->key << endl;
		 std::cout << temp->rid.pid << endl;
		 std::cout << temp->rid.sid << endl;
		 std::cout << "\n"; */

		 LeafElement* temp = (LeafElement *) buffer+i;
		cout << "key= " << temp->key << ", ";
		
	}
	cout << endl;
	return 0;
}

BTNonLeafNode::BTNonLeafNode()
{
	bzero(buffer, PageFile::PAGE_SIZE);
	insertSplit = false;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 
	return pf.read(pid, buffer);
}

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	// read first LeafElement from buffer (12 bytes)
	NonLeafElement* temp = (NonLeafElement *) buffer;
	return temp->key; // first LeafElement's key will hold the count value
}

int BTNonLeafNode::updateCount(int count)
{
	NonLeafElement* temp = (NonLeafElement *) buffer;
	temp->key = count;
	return 0;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
	if(getKeyCount() >= MAX_KEYS && insertSplit == false) // node is full
		return RC_NODE_FULL;
	else // node is not full
	{
		int eid;
		int result = locate(key, eid);
		NonLeafElement* temp;// = (LeafElement *) buffer;
		NonLeafElement* tempTwo;
		if(result == -1) // append to end of node
		{	
			temp = (NonLeafElement*) buffer;
			int old=temp->pid;
			temp = (NonLeafElement *) buffer + (getKeyCount()+1);
			temp->key = key;
			temp->pid = old;
			
			temp = (NonLeafElement*) buffer;
			temp->pid = pid;
		}
		else if(result == 0)// locate did return a value
		{
			// shift the elements to the right
			NonLeafElement* startPosition = (NonLeafElement *)buffer + eid;
			NonLeafElement* endPosition = (NonLeafElement *)buffer + (getKeyCount()+1);
			for(int i = (getKeyCount()+1); i > eid; i--)
			{
				temp = (NonLeafElement*) buffer+i;
				tempTwo = (NonLeafElement*) buffer+i-1;
				temp->key = tempTwo->key;
				temp->pid = tempTwo->pid;
				// NonLeafElement* temp = endPosition - i;
				// *endPosition = *temp;
				// endPosition = temp;
			}

			// insert key and rid into new element
			temp = (NonLeafElement *) buffer + eid;
			temp->key = key;
			temp->pid = pid;
		}
		else
			return -1;

		// update number of keys in node
		if(insertSplit == false)
		{
			temp = (NonLeafElement *) buffer;
			int numOfKeys = temp->key;
			numOfKeys++;
			temp->key = numOfKeys;
		}
	}
	printBuffer();
	return 0; 
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
	if(sibling.getKeyCount() != 0)
		return -1;

	insertSplit = true;
	if(insert(key, pid) == 0)
	{
		// split the node into two nodes
		int middle;
		int siblingNumOfKeys = 0;
		if((getKeyCount()+1) %2 != 0)
			middle = ceil(((double)getKeyCount()+1)/(double)2);
		else
			middle = (getKeyCount()+1)/2+1;
		NonLeafElement* temp;
		temp = (NonLeafElement*) buffer+middle;
		midKey = temp->key;
		int oldPid = temp->pid;
		temp = (NonLeafElement *) buffer;
		int newPidForSibling = temp->pid;
		temp->pid = oldPid;
		sibling.changeEndRightPtr(newPidForSibling);
		for(int i = middle+1; i <= getKeyCount()+1; i++)
		{
			temp = (NonLeafElement *) buffer+i;
			int tempKey = temp->key;
			PageId tempPid = temp->pid;
			sibling.insert(tempKey, tempPid);
			siblingNumOfKeys++;
			if(i == middle)
				midKey = tempKey;
		}

		// update the count
		temp = (NonLeafElement *) buffer;
		int numOfKeys = temp->key;
		numOfKeys = middle - 1;
		temp->key = numOfKeys;

		//update suibling count
		sibling.updateCount(siblingNumOfKeys);
		insertSplit = false;
		return 0;
	}
	else
	{
		insertSplit = false;
		return -2;
	}
}


RC BTNonLeafNode::locate(int searchKey, int& eid)
{ 
	if(getKeyCount() == 0)
		return -1;

	NonLeafElement* temp = (NonLeafElement *) buffer +1; // start at element 1 because 0 is used for other info
	int count = 1; // start at 1, because 0 is where we store other info

	while(count <= getKeyCount())
	{
		if(temp->key >= searchKey)
		{
			eid = count;
			return 0;
		}
		count++;
		temp = (NonLeafElement *) buffer + count;
	}
	eid = -1;
	return -1;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
	for(int i = 1; i <= getKeyCount(); i++)
	{
		NonLeafElement* temp = (NonLeafElement *) buffer+i;
		if(i != getKeyCount())
		{
			if(searchKey < temp->key)
			{
				cout << searchKey<< " going left for " << temp->key << endl;
				pid = temp->pid;
				return 0;
			}
		}
		else
		{
			//printf("LOCATE CHILD... IN ELSE\n");
			if(searchKey < temp->key)
			{
				//printf("SEARCH KEY WAS LESS THAN PARENT KEY\n");
				pid = temp->pid;
				cout << searchKey<< " going left for " << temp->key << endl;
				pid = temp->pid;
				//printf("THE PID IS: %d\n", pid);
				return 0;
			}
			else
			{
				cout << searchKey<< " going right for " << temp->key << endl;

				temp = (NonLeafElement *) buffer;
				pid = temp->pid;
				return 0;
			}
		}
	}
	return -1;
}

RC BTNonLeafNode::changeEndRightPtr(PageId newPid)
{ 
	NonLeafElement* temp = (NonLeafElement *) buffer;
	temp->pid = newPid;
	return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
	//printf("IN INITIALIZE ROOT: LEFT PID IS %d\n",pid1);
	//printf("IN INITIALIZE ROOT: RIGHT PID IS %d\n",pid2);
	if(getKeyCount() == 0)
	{
		NonLeafElement* temp = (NonLeafElement *) buffer;
		temp->key = 1;
		temp->pid = pid2;
		temp = (NonLeafElement *) buffer+1;
		temp->key = key;
		temp->pid = pid1;
		return 0; 
	}
	else
		return -1;
}

int BTNonLeafNode::printBuffer()
{
	for(int i = 1; i <= getKeyCount(); i++)
	{
		 NonLeafElement* temp = (NonLeafElement *) buffer+i;
		 //std::cout << temp->key << endl;
		 //std::cout << temp->pid << endl;
		 //std::cout << temp->sid << endl;
		 //std::cout << "\n";
		cout << "NONLEAFkey= " << temp->key << ",";
	}
	cout << endl;
	return 0;
}
