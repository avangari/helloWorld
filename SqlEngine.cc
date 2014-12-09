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
#include <limits>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"
#define MAX_INT 2147483647

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

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  BTreeIndex btree;
  vector<RecordId> ridVector;
  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  int    low = 0, high = MAX_INT;
  bool indexExists = false;
  IndexCursor cursor;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;

  if(btree.open((table+".idx"),'r') == 0)
  {
    printf("INDEX EXISTS\n");
    indexExists = true;
  }
  if(indexExists)
  {
    for(int i = 0; i < cond.size(); i++)
    {
      if(cond[i].attr == 1)
      {
          switch (cond[i].comp) 
          {
                case SelCond::EQ:
                    if(atoi(cond[i].value) < low || atoi(cond[i].value) > high)
                      low = MAX_INT;
                    else
                    {
                      low = atoi(cond[i].value);
                      high = atoi(cond[i].value+1);
                    }
                  break;
                case SelCond::NE:           
                  break;
                case SelCond::GT:
                    if(atoi(cond[i].value) > low)
                      low = atoi(cond[i].value+1);
                  break;
                case SelCond::LT:
                    if(atoi(cond[i].value) < high)
                      high = atoi(cond[i].value);
                  break;
                case SelCond::GE:
                    if(atoi(cond[i].value) > low)
                      low = atoi(cond[i].value);
                  break;
                case SelCond::LE:
                  if(atoi(cond[i].value) < high)
                      high = atoi(cond[i].value+1);
                  break;
          }
          btree.locate(low,cursor);

          do
          {
            if(btree.readForward(cursor, key, rid) == 0)
              ridVector.push_back(rid);
          }
          while(key < high);
      }
    }
  }
  else
  {
    printf("INDEX DOESNT EXIST\n");
    while (rid < rf.endRid()) 
    {
      ridVector.push_back(rid);
      rid++;
    }
  }

  for(int i = 0; i < ridVector.size(); i++) 
  {
    rid = ridVector.at(i);
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }

    // check the conditions on the tuple
    for (unsigned i = 0; i < cond.size(); i++) {
          // compute the difference between the tuple value and the condition value
          switch (cond[i].attr) 
          {
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
    ;
    //++rid;
  }

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
