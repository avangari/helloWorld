/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

// RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
// {
//   IndexCursor c;
//   RecordFile rf;   // RecordFile containing the table
//   RecordId   rid;  // record cursor for table scanning
//   BTreeIndex btree;

//   RC     rc;
//   int    key;     
//   string value;
//   int    count;
//   int    diff;
//   int    lKey = 0;

//   // open the table file
//   if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
//     fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
//     return rc;
//   }

//   // scan the table file from the beginning
//   rid.pid = rid.sid = 0;
//   count = 0;

//   if(btree.open(table+".idx",'r') == 0)
//   {
//     printf("INDEX EXISTS\n");
//     int eq = SelCond::EQ;
//     int ne = SelCond::NE;
//     int lt = SelCond::LT;
//     int gt = SelCond::GT;
//     int le = SelCond::LE;
//     int ge = SelCond::GE;

//     for(int i = 0; i < cond.size(); i++){ //loop through the conditions
//       if(cond[i].attr != 1) 
//         continue;
//       else if(cond[i].comp == eq){
//         lKey = atoi(cond[i].value);
//         break;
//       }
//       else if(cond[i].comp == ge){
//         if(lKey == 0){
//           lKey = atoi(cond[i].value);
//           continue;
//         }
//         int compareval = atoi(cond[i].value);
//         if(compareval > lKey)
//           lKey = compareval;
        
//       }
//       else if(cond[i].comp == gt){
//         if(lKey == 0){
//           lKey = atoi(cond[i].value);
//           continue;
//         }
//         int compareval = atoi(cond[i].value)+1;
//         if(compareval > lKey)
//           lKey = compareval;
//       }
//     }

//     btree.locate(lKey,c);

// while(btree.readForward(c,key,rid) == 0){ //same loop as non btree search below, except with iterator removed
//       if (attr != 4) { //we don't want values when doing count(*)
//         if ((rc = rf.read(rid, key, value)) < 0) {
//           fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
//           goto exit_select;
//         }
//       }
//       // check the conditions on the tuple
//       for (unsigned i = 0; i < cond.size(); i++) {
//         // compute the difference between the tuple value and the condition value
//         switch (cond[i].attr) {
//         case 1:
//       diff = key - atoi(cond[i].value);
//       break;
//         case 2:
//       diff = strcmp(value.c_str(), cond[i].value);
//       break;
//         }

//         // skip the tuple if any condition is not met
//         switch (cond[i].comp) {
//         case SelCond::EQ:
//       if (diff != 0) 
//       {
//         if(cond[i].attr == 1) 
//         { 
//           goto exit_select; 
//         }
//         else {continue;} //non inclusive
//       }
//       break;
//         case SelCond::NE:
//       if (diff == 0) continue;
//       break;
//         case SelCond::GT:
//       if (diff <= 0) continue;
//       break;
//         case SelCond::LT:
//       if (diff >= 0)
//       {
//         if(cond[i].attr == 1) 
//         {
//           goto exit_select;
//         } 
//         else 
//         {
//           continue;
//         } //non inclusive
//       }
//       break;
//         case SelCond::GE:
//       if (diff < 0) continue;
//       break;
//         case SelCond::LE:
//       if (diff > 0) 
//       {
//         if(cond[i].attr == 1)
//         {
//           goto exit_select;
//         }
//         else {continue;} //non inclusive
//       }
//       break;
//         }
//       }

//       // the condition is met for the tuple. 
//       // increase matching tuple counter
//       count++;

//       // print the tuple 
//       switch (attr) {
//       case 1:  // SELECT key
//         fprintf(stdout, "%d\n", key);
//         break;
//       case 2:  // SELECT value
//         fprintf(stdout, "%s\n", value.c_str());
//         break;
//       case 3:  // SELECT *
//         fprintf(stdout, "%d '%s'\n", key, value.c_str());
//         break;
//       }
//     }
//   }
//   else
//   {
//     printf("INDEX DOESN'T EXIST\n");
//     while (rid < rf.endRid()) 
//     {
//       // read the tuple
//       if ((rc = rf.read(rid, key, value)) < 0) {
//         fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
//         goto exit_select;
//       }

//       // check the conditions on the tuple
//       for (unsigned i = 0; i < cond.size(); i++) {
//         // compute the difference between the tuple value and the condition value
//         switch (cond[i].attr) {
//         case 1:
//   	diff = key - atoi(cond[i].value);
//   	break;
//         case 2:
//   	diff = strcmp(value.c_str(), cond[i].value);
//   	break;
//         }

//         // skip the tuple if any condition is not met
//         switch (cond[i].comp) {
//         case SelCond::EQ:
//         	if (diff != 0) goto next_tuple;
//         	break;
//               case SelCond::NE:
//         	if (diff == 0) goto next_tuple;
//         	break;
//               case SelCond::GT:
//         	if (diff <= 0) goto next_tuple;
//         	break;
//               case SelCond::LT:
//         	if (diff >= 0) goto next_tuple;
//         	break;
//               case SelCond::GE:
//         	if (diff < 0) goto next_tuple;
//         	break;
//               case SelCond::LE:
//         	if (diff > 0) goto next_tuple;
//         	break;
//         }
//       }

//       // the condition is met for the tuple. 
//       // increase matching tuple counter
//       count++;

//       // print the tuple 
//       switch (attr) {
//       case 1:  // SELECT key
//         fprintf(stdout, "%d\n", key);
//         break;
//       case 2:  // SELECT value
//         fprintf(stdout, "%s\n", value.c_str());
//         break;
//       case 3:  // SELECT *
//         fprintf(stdout, "%d '%s'\n", key, value.c_str());
//         break;
//       }

//       // move to the next tuple
//       next_tuple:
//       ++rid;
//     }
//   }

//   // print matching tuple count if "select count(*)"
//   if (attr == 4) {
//     fprintf(stdout, "%d\n", count);
//   }
//   rc = 0;

//   // close the table file and return
//   exit_select:
//   rf.close();
//   return rc;
// }

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  BTreeIndex index;// B+Tree Index for the table, if it exists
  IndexCursor cursor;

  bool hasIndex;

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  int    lookup;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // Open the index file, if exists
  if (!index.open(table+".idx", 'r'))
  {
    // Run this algorithm if index exists

    lookup = -1;

    // Determine which key to look up
    for (unsigned i = 0; i < cond.size(); i++)
    {
      // We only care about conditions on keys
      if (cond[i].attr != 1)
        continue;

      // The first EQ condition trumps all other conditions
      if (cond[i].comp == SelCond::EQ)
      {
        lookup = i;
        break;
      }
      // Determine what range to start from
      if (cond[i].comp == SelCond::GT || cond[i].comp == SelCond::GE)
      {
        if (lookup == -1 || atoi(cond[i].value) > atoi(cond[lookup].value))
          lookup = i;
      }
    }

    // Locate cursor in index tree
    if (lookup > -1)
      index.locate(atoi(cond[lookup].value), cursor);
    else
      index.locate(0,cursor); //Finds first entry in tree

    // Scan the table beginning from the cursor
    count = 0;
    while (!index.readForward(cursor, key, rid))
    {
      // read the tuple
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      // Check the conditions on the tuple
      for (unsigned i = 0; i < cond.size(); i++)
      {
        // Compute the difference betweent the tuple and the condition
        switch (cond[i].attr)
        {
          case 1:
            diff = key - atoi(cond[i].value);
            break;
          case 2:
            diff = strcmp(value.c_str(), cond[i].value);
            break;
        }

        // Determine if condition has been met
        switch (cond[i].comp)
        {
        case SelCond::EQ:
          if (diff != 0)
            if (cond[i].attr == 1)
              goto finish_read;
            else
              continue;
          break;
        case SelCond::NE:
          if (diff == 0) continue;
          break;
        case SelCond::GT:
          if (diff <= 0) continue;
          break;
        case SelCond::LT:
          if (diff >= 0)
            if (cond[i].attr == 1)
              goto finish_read;
            else
              continue;
          break;
        case SelCond::GE:
          if (diff < 0) continue;
          break;
        case SelCond::LE:
          if (diff > 0)
            if (cond[i].attr == 1)
              goto finish_read;
            else
              continue;
          break;
        }
      }

      // Tuple matches conditions. Increment count
      count++;

      // print the tuple 
      switch (attr) {
      case 1:  // SELECT key
        fprintf(stdout, "%d\n", key);
        break;
      case 2:  // SELECT value
        fprintf(stdout, "%s\n", value.c_str());
        break;
      case 3:  // SELECT *
        fprintf(stdout, "%d '%s'\n", key, value.c_str());
        break;
      }
    }
  }
  else
  {
    // scan the table file from the beginning if no index
    rid.pid = rid.sid = 0;
    count = 0;
    while (rid < rf.endRid()) {
      // read the tuple
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      // check the conditions on the tuple
      for (unsigned i = 0; i < cond.size(); i++) {
        // compute the difference between the tuple and the condition value
        switch (cond[i].attr) {
        case 1:
    diff = key - atoi(cond[i].value);
    break;
        case 2:
    diff = strcmp(value.c_str(), cond[i].value);
    break;
        }

        // skip the tuple if any condition is not met
        switch (cond[i].comp) {
        case SelCond::EQ:
    if (diff != 0) goto next_tuple;
    break;
        case SelCond::NE:
    if (diff == 0) goto next_tuple;
    break;
        case SelCond::GT:
    if (diff <= 0) goto next_tuple;
    break;
        case SelCond::LT:
    if (diff >= 0) goto next_tuple;
    break;
        case SelCond::GE:
    if (diff < 0) goto next_tuple;
    break;
        case SelCond::LE:
    if (diff > 0) goto next_tuple;
    break;
        }
      }

      // the condition is met for the tuple. 
      // increase matching tuple counter
      count++;

      // print the tuple 
      switch (attr) {
      case 1:  // SELECT key
        fprintf(stdout, "%d\n", key);
        break;
      case 2:  // SELECT value
        fprintf(stdout, "%s\n", value.c_str());
        break;
      case 3:  // SELECT *
        fprintf(stdout, "%d '%s'\n", key, value.c_str());
        break;
      }

      // move to the next tuple
      next_tuple:
      ++rid;
    }
  }
  finish_read:
  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}


RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  BTreeIndex btree;
  RecordFile rf; // new instance of RecordFile
  RC rc; // for return status
  string tableWithExt = table + ".tbl";

  // try opening/creating the table file, in order to write to it
  if ((rc = rf.open(tableWithExt, 'w')) != 0) {
    fprintf(stderr, "Error creating/opening table:\n");
    return rc;
  }

  if(index){ //if we need to create an index
    if(btree.open(table + ".idx", 'w') < 0) //if opening fails
      return -1;
  }

  // file stream based off loadfile
  ifstream inputFile(loadfile.c_str());
  string str; // used to read line by line

  if(!inputFile) // means error occured while creating the stream
  {
    fprintf(stderr, "Error opening file:\n");
    return RC_FILE_OPEN_FAILED; //defined in Bruinbase.h
  }

  int key;
  string value;

  printf("Starting reading file...\n");

  // while a new line exists, place entire line into str variable
  while(getline(inputFile, str))
  {
    // try parsing line in order to extract key and value
    if((rc = parseLoadLine(str, key, value)) != 0) // parsing failed
    {
      fprintf(stderr, "Error parsing loadfile: \n");
      return rc;
    }

    RecordId rid;

    // try appending key value pair into the RecordFile
    if((rc = rf.append(key, value, rid)) != 0) //appending failed
    {
      fprintf(stderr, "Error appending key value into RecordFile.\n");
      return rc;
    }
    else if(index)
      btree.insert(key, rid);
  }
  printf("Finished reading file and appending to record file...\n");

  if(index)
    btree.close();
  inputFile.close(); // close the file stream
  rf.close(); // close the record file  
  rc = 0; // everything was successful at this point
  
  printf("File and record file closed properly\n");
  return rc;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
