/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	treeHeight = 0;
    rootPid = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
	// open page file, pagefile will create and open if
	// file doesn't exist
	if(pf.open(indexname, mode) < 0)
		return -1;

	if(pf.endPid() == 0) // new and empty index;
	{
		rootPid = -1;
		treeHeight = 0;
	}
	else // pagefile has pages 
	{
		if(pf.read(0, buffer) >= 0) //pid0 holds will hold rootPid, and treeHeight
		{
			printf("READ SUCCESSFUL\n");
			rootPid = *((PageId*)buffer);
			treeHeight = *((int*)(buffer+sizeof(int)));
			printf("ROOT ID: %d, HEIGHT: %d\n",rootPid, treeHeight);
		}
		else
			return -1;
	}
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	*((PageId*)buffer) = rootPid; // store the rootPid at time of closing
	*((int*)(buffer+sizeof(int))) = treeHeight; // store the treeHeight at the time of closing
	
	if(pf.write(0,buffer) < 0)
		return -1;

    if(pf.close() < 0)
    	return -1;

    printf("INDEX FILE CLOSED, ROOT PID: %d HEIGHT: %d\nn", rootPid, treeHeight);
    return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	if(treeHeight > 0)
	{
		int foundKey, newPid, rc;
		// pass in key, rid. Start with height of 1, provide two 
		// integers as reference. 
		rc = insertRecursive(key, rid, 1, rootPid, foundKey, newPid);
		if(rc == 0)
			return rc;
		else if(rc == 1) // means the leaf node was full, and has been split. 
		{
			printf("NODE WAS SPLIT, ADDING PARENT...\n");
			//rootPid = pf.endPid(); // new pid will be the root
			BTNonLeafNode parent = BTNonLeafNode();

			// make parent the root
			parent.initializeRoot(rootPid, foundKey, newPid);
			rootPid = pf.endPid();
			if(parent.write(rootPid,pf) < 0)
				return -1;
			
			treeHeight++; // since parent created, increment treeHeight
			printf("PARENT CREATED!\n");
			parent.printBuffer();
			PageId left, right;
			parent.locateChildPtr(41,left);
			printf("PARENT NODE LEFT POINTER IS %d\n", left);
			parent.locateChildPtr(42,right);
			printf("PARENT NODE RIGHT POINTER IS %d\n", right);
			return 0;
		}
		else 
			return -1;
	}
	else if (treeHeight == 0) // index empty, inserting first node
	{
		printf("In else, new index\n");
		rootPid = pf.endPid(); // rootPid is 0
		BTLeafNode ln = BTLeafNode();
		if(ln.insert(key, rid) < 0)
			return -1;
		ln.printBuffer();
		if(ln.write(rootPid, pf) < 0) // use page 0 of pageFile to store 
			return -1; 
		treeHeight++; // treeHeight is now 1
		return 0;
	}
	else
		return -1;
}

/*
 * insertRecursive (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @param currentHeight the height of the tree at point of calling function
 * @param currentPid the page pid that is currently being inspected
 * @param foundKey the integer reference where the key will be stored
 * @param newPid the integer reference where new pid will be stored
 * @return error code. 0 if no error
 */
RC BTreeIndex::insertRecursive(int& key, const RecordId& rid, int currentHeight, PageId currentPid, int& foundKey, int& newPid)
{
	printf("In recursive.\n");
	// if currentHeight = treeHeight, we are at the leaf node
	if(currentHeight == treeHeight)
	{
		BTLeafNode ln = BTLeafNode();
		if(ln.read(currentPid, pf) < 0)
			return -1;
		if(ln.getKeyCount() >= ln.MAX_KEYS) // leafNode is full, need to do insert and split
		{
			printf("No space in node.\n");
			BTLeafNode newNode = BTLeafNode();
			// split the full node, second half of elements placed into newNode
			if(ln.insertAndSplit(key, rid, newNode, foundKey) < 0)
				return -1;

			// rearrange the pointers for the leaf nodes
			newNode.setNextNodePtr(ln.getNextNodePtr());
			newPid = pf.endPid();
			ln.setNextNodePtr(newPid);

			printf("OLD NODE: PID IS %d\n", currentPid);
			ln.printBuffer();
			printf("NEW NODE: PID IS %d\n", newPid);
			newNode.printBuffer();

			// write both nodes to memory
			if(newNode.write(newPid, pf) < 0)
				return -1;
			
			if(ln.write(currentPid, pf) < 0)
				return -1;

			return 1;
		}
		else // there is space in node to insert 
		{
			printf("Space in node.\n");
			ln.insert(key, rid);
			if(ln.write(currentPid, pf) < 0)
				return -1;
			ln.printBuffer();
			return 0;
		}
	}
	else // need to iterate the non-leaf nodes
	{
		PageId child;
		int rc;
		BTNonLeafNode nl = BTNonLeafNode();
		
		if(nl.read(currentPid, pf) < 0)
			return -1;

		// Find out if you need to traverse left or right of node.
		// selected pid placed into child
		nl.locateChildPtr(key, child);

		// Go down a level in the tree (increment the current height) 
		// and recursively call same method with the pid of the child.  
		rc = insertRecursive(key, rid, currentHeight+1,child,foundKey,newPid);
		
		// means childnode was split, need to add new key to parent node 
		if(rc == 1)
		{
			if(nl.getKeyCount() >= nl.MAX_KEYS) // node is full
			{
				int sKey;
				BTNonLeafNode newNode = BTNonLeafNode();
				// split the full node, second half of elements placed into newNode
				nl.insertAndSplit(foundKey, newPid, newNode, sKey);
				
				newPid = pf.endPid();
				foundKey = sKey;
				
				if((newNode.write(newPid,pf)) < 0)
					return -1;

				if((newNode.write(currentPid,pf)) < 0)
					return -1;

				return 1;
			}
			else // node has space for new element 
			{
				nl.insert(foundKey,newPid);
				if(nl.write(currentPid, pf) < 0)
					return -1;

				return 0;
			}
		}
		else if(rc == 0)
			return 0;
		else
			return -1;

	}
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	PageId pid = rootPid;
	BTNonLeafNode nonLeafNode;
	int entryId;

	if(treeHeight > 1) // multilevel tree
	{
		// iterate each level of the tree
		for(int i = 0; i < treeHeight; i++)
		{
			if(nonLeafNode.read(pid, pf) < 0)
			{
				printf("NONLEAF LOCATE FAILED\n");
				return -1;
			}
			// Find out if you need to traverse left or right of node.
			// selected pid placed into pid
			if(nonLeafNode.locateChildPtr(searchKey, pid))
				return -1;
		}
	}

	BTLeafNode leafNode;

	if(leafNode.read(pid, pf) < 0)
	{
		printf("LEAF LOCATE FAILED\n");
		return -1;
	}
	// locate, the searchKey of selected pid, and store into entryId
	if(leafNode.locate(searchKey, entryId) < 0)
		return -1;

	// asisgn references
	cursor.pid = pid;
	cursor.eid = entryId;

    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{

  BTLeafNode ln;
  ln.read(cursor.pid, pf);
  ln.readEntry(cursor.eid, key, rid);

  //Check if we have a valid page
  if (cursor.pid <= 0 || cursor.pid >= pf.endPid())
  {
    return 1;
  }

  // Increment cursor
  cursor.eid++;
  if (cursor.eid >= ln.getKeyCount())
  {
    cursor.pid = ln.getNextNodePtr();
    cursor.eid = 0;
  }

  return 0;
	// BTLeafNode leafNode;

	// // read selected pid
	// if(leafNode.read(cursor.pid, pf) < 0)
	// 	return -1;

	// // get key and rid from selecte eid, and store into the references
	// if(leafNode.readEntry(cursor.eid, key, rid) < 0)
	// 	return -1;

	// cursor.eid++;

	// if(cursor.eid > leafNode.getKeyCount())
	// {
	// 	cursor.eid = 1;
	// 	cursor.pid = leafNode.getNextNodePtr();
	// }
 //    return 0;
}
