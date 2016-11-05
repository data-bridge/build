/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "Buffer.h"
#include "parse.h"
#include "Bexcept.h"

#define CHUNK_SIZE 1024


Buffer::Buffer()
{
  Buffer::reset();
}


Buffer::~Buffer()
{
}


void Buffer::reset()
{
  len = 0;
  current = 0;
  lines.resize(CHUNK_SIZE);
}


// http://stackoverflow.com/questions/17925051/
//   fast-textfile-reading-in-c

void Buffer::readBinaryFile(const string& fname)
{
  int fd = open(fname.c_str(), O_RDONLY);
  if (fd == -1)
    THROW("Could not open " + fname);

  // Advise the kernel of our access pattern.
  // FDADVICE_SEQUENTIAL
  // posix_fadvise(fd, 0, 0, 1);  

  const size_t BUFFER_SIZE = 16*1024;
  char buf[BUFFER_SIZE + 1];
  LineData lineData;
  unsigned no = 0;
  string leftover = "";

  while (int bytes_read = ::read(fd, buf, BUFFER_SIZE))
  {
    if (bytes_read == -1)
      THROW("Read failed");

    if (! bytes_read)
      break;

    char * prev = buf;
    char * p = buf;
    while (p = (char *) memchr(p, '\n', 
      static_cast<size_t>((buf + bytes_read) - p)))
    {
      lineData.len = static_cast<unsigned>(p-prev);
      lineData.line = leftover + 
        chars2str(prev, lineData.len);
      lineData.no = ++no;

      lines.push_back(lineData);
      prev = p;
      leftover = "";
    }
    leftover = chars2str(prev, 
      static_cast<unsigned>((buf + bytes_read) - prev));
  }

  len = lines.size();
  close(fd);
}


bool Buffer::isLIN(LineData& ld)
{
  if (ld.line.substr(2, 1) != "|")
    return false;

  ld.type = BRIDGE_BUFFER_STRUCTURED;
  ld.label = ld.line.substr(0, 2);
  ld.value = ld.line.substr(3);
  return true;
}


bool Buffer::isPBN(LineData& ld)
{
  if (ld.len < 4 || 
      ld.line.at(0) != '[' ||
      ld.line.at(ld.len-2) != '"' ||
      ld.line.at(ld.len-1) != ']')
    return false;

  size_t pos = ld.line.find(" ", 1);
  if (pos == string::npos)
    return false;

  ld.type = BRIDGE_BUFFER_STRUCTURED;
  ld.label = ld.line.substr(1, pos-1);

  pos = ld.line.find("\"", pos);
  if (pos == ld.len-2)
    return false;

  ld.value = ld.line.substr(pos+1, len-2-pos);
  return true;
}


bool Buffer::isRBN(LineData& ld)
{
  if (ld.len <= 1 || ld.line.at(1) != ' ')
    return false;

  ld.type = BRIDGE_BUFFER_STRUCTURED;
  ld.label = ld.line.substr(0, 1);

  if (ld.len > 2)
    ld.value = ld.line.substr(2);
  else
    ld.value = "";
  return true;
}


bool Buffer::isRBX(LineData& ld)
{
  if (ld.len <= 3 || ld.line.at(0) != '{' || ld.line.at(ld.len-1) != '}')
    return false;

  ld.type = BRIDGE_BUFFER_STRUCTURED;
  ld.label = ld.line.substr(1, 1);
  ld.value = ld.line.substr(1, ld.len-2);
  return true;
}


void Buffer::classify(
  LineData& ld,
  const Format format)
{
  if (ld.len == 0)
  {
    ld.type = BRIDGE_BUFFER_EMPTY;
    return;
  }
  else if (format == BRIDGE_FORMAT_TXT &&
      ld.len > 5 &&
      ld.line.substr(0, 5) == "-----")
  {
    ld.type = BRIDGE_BUFFER_DASHES;
    return;
  }
  else if (ld.line.at(0) == '%')
  {
    ld.type = BRIDGE_BUFFER_COMMENT;
    return;
  }

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      if (! Buffer::isLIN(ld))
        ld.type = BRIDGE_BUFFER_GENERAL;
      break;

    case BRIDGE_FORMAT_PBN:
      if (! Buffer::isPBN(ld))
        ld.type = BRIDGE_BUFFER_GENERAL;
      break;

    case BRIDGE_FORMAT_RBN:
      if (! Buffer::isRBN(ld))
        ld.type = BRIDGE_BUFFER_GENERAL;
      break;

    case BRIDGE_FORMAT_RBX:
      if (! Buffer::isRBX(ld))
        ld.type = BRIDGE_BUFFER_GENERAL;
      break;

    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      ld.type = BRIDGE_BUFFER_GENERAL;
      break;

    default:
      THROW("Invalid format: " + STR(format));
  }
}


bool Buffer::read(
  const string& fname,
  const Format format)
{
  Buffer::readBinaryFile(fname);
  if (len == 0)
    return false;

  for (auto &ld: lines)
    Buffer::classify(ld, format);
}


void Buffer::readRefFix(
  const string& fname,
  vector<RefFix>& refFix)
{
  regex re("\\.\\w+$");
  string refName = regex_replace(fname, re, string(".ref"));

  // There might not be a .ref file (not an error).
  ifstream refstr(refName.c_str());
  if (! refstr.is_open())
    return;

  string line, s;
  RefFix rf;
  regex rer("^\\s*\"(.*)\"\\s*$");
  smatch match;
  while (getline(refstr, line))
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    if (! str2unsigned(s, rf.lno))
      THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      
    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    if (s == "insert")
    {
      rf.type = BRIDGE_REF_INSERT;
      if (! regex_search(line, match, rer) || match.size() < 1)
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

      rf.value = match.str(1);
    }
    else if (s == "replace")
    {
      rf.type = BRIDGE_REF_REPLACE;
      if (! regex_search(line, match, rer) || match.size() < 1)
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

      rf.value = match.str(1);
    }
    else if (s == "delete")
    {
      rf.type = BRIDGE_REF_DELETE;
      if (getNextWord(line, s))
      {
        if (! str2unsigned(s, rf.count))
          THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      }
      else
        rf.count = 1;
    }
    else
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    refFix.push_back(rf);
  }
  refstr.close();
}


bool Buffer::fix(
  const string& fname,
  const Format format)
{
  vector<RefFix> refFix;
  Buffer::readRefFix(fname, refFix);

  if (refFix.size() == 0)
    return false;
   
  unsigned rno = 0;
  for (unsigned i = 0; i < len; i++)
  {
    LineData& ld = lines[i];
    if (refFix[rno].lno != ld.no)
      continue;

    if (refFix[rno].type == BRIDGE_REF_INSERT)
    {
      LineData lnew;
      lnew.line = refFix[rno].value;
      lnew.len = lnew.line.length();
      lnew.no = 0;
      Buffer::classify(lnew, format);
      lines.insert(lines.begin() + static_cast<int>(i), lnew);
      len++;
    }
    else if (refFix[rno].type == BRIDGE_REF_REPLACE)
    {
      ld.line = refFix[rno].value;
      Buffer::classify(ld, format);
    }
    else if (refFix[rno].type == BRIDGE_REF_DELETE)
    {
      if (i + refFix[rno].count > len)
        THROW("Too large deletion");

      i += refFix[rno].count;
    }
    else
      THROW("Bad reference line type");

    rno++;
  }
  return true;
}


bool Buffer::advance()
{
  if (current == len-1)
    return false;

  current++;
  return true;
}


void Buffer::getRecord(ValSide& vside) const
{
  vside.line = lines[current].line;
  vside.lno = lines[current].no;
}


string Buffer::getLine() const
{
  return lines[current].line;
}


LineType Buffer::getType() const
{
  return lines[current].type;
}


string Buffer::getLabel() const
{
  return lines[current].label;
}


string Buffer::getValue() const
{
  return lines[current].value;
}
  
