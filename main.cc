/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeNode.h"
#include "BTreeIndex.h"

int main()
{
    SqlEngine::run(stdin);
    // BTreeIndex bIndex;
    // RecordId rid;
    // rid.pid = 1;
    // rid.sid = 0;

    // bIndex.open("newIndex", 'w');
    // for(int i = 0; i < 84; i++)
    // {
    //     bIndex.insert(i, rid);
    // }
    //bIndex.insert(1, rid);

  // run the SQL engine taking user commands from standard input (console).
  //SqlEngine::run(stdin);]

    // BTLeafNode node;
    // RecordId rid;
    // rid.pid = 1;
    // rid.sid = 0;
    // node.insert(1,rid);
    // node.insert(5,rid);
    // node.insert(3,rid);

    // //node.printBuffer();
    // node.insert(4,rid);
    // //node.insert(6,rid);
    // BTLeafNode newNode;
    // int siblingKey;
    // node.insertAndSplit(2, rid, newNode, siblingKey);
    // newNode.printBuffer();
    // node.printBuffer();

    // BTNonLeafNode node, newNode;
    // int midKey;
    // node.insert(2, 1);
    // node.insert(4, 3);
    // node.insert(6, 5);
    // node.insertAndSplit(3, 2, newNode, midKey);
    // newNode.printBuffer();
    
    // BTNonLeafNode node;
    // PageId pid = 1;
    // for(int i = 1; i < 79; i++)
    // {
    // 	node.insert(i,pid);
    // }
    // BTNonLeafNode newNode;
    // int midKey;
    // node.insertAndSplit(84, pid, newNode, midKey);
    // newNode.printBuffer();
    //node.printBuffer();
  return 0;
}
