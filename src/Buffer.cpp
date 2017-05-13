/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <regex>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <cstdint>

#if defined(__CYGWIN__)
  #include <unistd.h>
#endif

#include "Buffer.h"
#include "parse.h"
#include "referr.h"
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
  fileName = "";
  len = 0;
  current = 0;
  format = BRIDGE_FORMAT_SIZE;
  posLIN = 0;
  posRBX = 0;
  lines.reserve(CHUNK_SIZE);
}


void Buffer::rewind()
{
  current = 0;
  posLIN = 0;
  posRBX = 0;
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

  const size_t BUFFER_SIZE = 64*1024;
  char buf[BUFFER_SIZE + 1];
  LineData lineData;
  unsigned no = 0;
  string leftover = "";
  lines.clear();

  while (int bytes_read = ::read(fd, buf, BUFFER_SIZE))
  {
    if (bytes_read == -1)
      THROW("Read failed");

    if (! bytes_read)
      break;

    char * prev = buf;
    char * p = buf;
    while ((p = static_cast<char *>(memchr(prev, '\n', 
        static_cast<size_t>
        (
          (reinterpret_cast<long int>(buf) + 
           static_cast<long int>(bytes_read)) -
           reinterpret_cast<long int>(prev)
        )
        ))) != nullptr)
      // static_cast<size_t>((buf + bytes_read) - prev))) != nullptr)
    {
      lineData.len = static_cast<unsigned>(p-prev);
      lineData.line = leftover + 
        chars2str(prev, lineData.len);
      lineData.len += static_cast<unsigned>(leftover.size());
      lineData.no = ++no;

      lines.push_back(lineData);
      prev = p+1;
      leftover = "";
    }
    leftover = chars2str(prev, 
      static_cast<unsigned>((buf + bytes_read) - prev));
  }

  if (leftover != "")
  {
    lineData.line = leftover;
    lineData.len = leftover.size();
    lineData.no = ++no;
    lines.push_back(lineData);
  }

  len = static_cast<unsigned>(lines.size());
  close(fd);
}


bool Buffer::isLIN(LineData& ld)
{
  if (ld.len < 3 || ld.line.substr(2, 1) != "|")
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
  if (pos+3 >= ld.len)
    ld.value = "";
  else
    ld.value = ld.line.substr(pos+1, ld.len-pos-3);
  return true;
}


bool Buffer::isRBN(LineData& ld)
{
  if (ld.len == 0)
    return false;
  else if (ld.line.at(0) == ' ')
    return false;
  else if (ld.len >= 2 && ld.line.at(1) != ' ')
    return false;

  ld.type = BRIDGE_BUFFER_STRUCTURED;
  ld.label = ld.line.substr(0, 1);

  if (ld.len > 2)
    ld.value = ld.line.substr(2);
  else
  {
    ld.value = "";
    if (ld.len == 1)
    {
      ld.line += " ";
      ld.len = 2;
    }
  }
  return true;
}


bool Buffer::isRBX(LineData& ld)
{
  if (ld.len <= 3 || ld.line.at(1) != '{' || ld.line.at(ld.len-1) != '}')
    return false;

  ld.type = BRIDGE_BUFFER_STRUCTURED;
  ld.label = ld.line.substr(1, 1);
  ld.value = ld.line.substr(1, ld.len-2);
  return true;
}


void Buffer::classify(LineData& ld)
{
  if (ld.len == 0)
  {
    ld.type = BRIDGE_BUFFER_EMPTY;
    return;
  }
  else if (ld.len == 1 &&
      (format == BRIDGE_FORMAT_PBN || format == BRIDGE_FORMAT_LIN_VG) &&
      ld.line == " ")
  {
    // Kludge for some versions of Double Dummy Captain.
    ld.type = BRIDGE_BUFFER_EMPTY;
    return;
  }
  else if ((format == BRIDGE_FORMAT_LIN || format == BRIDGE_FORMAT_LIN_TRN) &&
      ld.len >= 17 && ld.len <= 23 &&
      ld.line.substr(0, 3) == "qx|")
  {
    // Kludge for some empty hands from actual own BBO play.
    //
    if (ld.len >= 21 && ld.line.substr(ld.len-15) == "pn|,,,,,,,|pg||")
    {
      ld.type = BRIDGE_BUFFER_EMPTY;
      return;
    }
    else if (ld.len <= 19 && ld.line.substr(ld.len-11) == "pn|,,,|pg||")
    {
      ld.type = BRIDGE_BUFFER_EMPTY;
      return;
    }
  }
  else if (format == BRIDGE_FORMAT_TXT &&
      ld.len > 5 &&
      ld.line.substr(0, 5) == "-----")
  {
    ld.type = BRIDGE_BUFFER_DASHES;
    return;
  }
  else if (format ==  BRIDGE_FORMAT_EML &&
      ld.len > 50 &&
      (ld.line.substr(30, 5) == "-----" ||
       ld.line.substr(30, 5) == "====="))
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
      if (! Buffer::isRBX(ld) &&
          ! Buffer::isRBN(ld))
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
  const Format formatIn,
  RefControl& refControl,
  const RefUse use)
{
  fileName = fname;
  format = formatIn;

  vector<Refline> reflines;
  readRefFix(fname, reflines, refControl);
  if (refControl == ERR_REF_SKIP)
    return false;

  Buffer::readBinaryFile(fname);
  if (len == 0)
    return false;

  for (auto &ld: lines)
    Buffer::classify(ld);

  return Buffer::fix(reflines, use);

  return true;
}


bool Buffer::split(
  const string& st,
  const Format formatIn)
{
  format = formatIn;
  LineData lineData;
  size_t l = st.size();
  size_t p = 0;
  len = 0;

  while (p < l)
  {
    size_t found = st.find("\n", p);
    lineData.line = st.substr(p, found-p);
    lineData.len = static_cast<unsigned>(lineData.line.length());
    lineData.no = ++len;
    lines.push_back(lineData);
    if (found == string::npos)
      break;

    p = found+1;
  }

  if (len == 0)
    return false;

  for (auto &ld: lines)
    Buffer::classify(ld);
  return true;
}


bool Buffer::fix(
  const string& fname,
  RefControl& refControl,
  const RefUse use)
{
  vector<Refline> reflines;
  readRefFix(fname, reflines, refControl);
  if (refControl == ERR_REF_SKIP)
    return false;

  return Buffer::fix(reflines, use);
}


bool Buffer::fix(
  const vector<Refline>& reflines,
  const RefUse use)
{
  bool usedFlag = false;

  for (unsigned rno = 0; rno < reflines.size(); rno++)
  {
    if (use == BRIDGE_REF_ONLY_PARTIAL && 
        reflines[rno].isUncommented())
      continue;
    else if (use == BRIDGE_REF_ONLY_NONPARTIAL && 
        reflines[rno].isCommented())
      continue;

    const unsigned i = Buffer::getInternalNumber(reflines[rno].lineno());
    if (i == BIGNUM)
      THROW("Cannot find ref line number " + STR(reflines[rno].lineno()));
    LineData& ld = lines[i];

    const int refType = reflines[rno].type();
    if (refType == BRIDGE_REF_INSERT_GEN)
    {
      LineData lnew;
      reflines[rno].modify(lnew.line);
      lnew.len = static_cast<unsigned>(lnew.line.length());
      lnew.no = 0;
      Buffer::classify(lnew);
      lines.insert(lines.begin() + static_cast<int>(i), lnew);
      usedFlag = true;
      len++;
    }
    else if (refType == BRIDGE_REF_REPLACE_GEN)
    {
      reflines[rno].modify(ld.line);
      ld.len = static_cast<unsigned>(ld.line.length());
      Buffer::classify(ld);
      usedFlag = true;
    }
    else if (refType == BRIDGE_REF_DELETE_GEN)
    {
      const unsigned deletion = reflines[rno].deletion();
      if (i + deletion > len)
        THROW("Too large deletion");

      lines.erase(lines.begin() + static_cast<int>(i), 
          lines.begin() + static_cast<int>(deletion + i));
      len -= deletion;
      usedFlag = true;
    }
    else if (refType == BRIDGE_REF_INSERT_LIN ||
        refType == BRIDGE_REF_REPLACE_LIN ||
        refType == BRIDGE_REF_DELETE_LIN)
    {
      reflines[rno].modify(ld.line);
      ld.len = static_cast<unsigned>(ld.line.length());
      Buffer::classify(ld);
      usedFlag = true;
    }
    else
      THROW("Bad reference line type");
  }
  return usedFlag;
}


bool Buffer::advance()
{
  if (current == len-1)
    return false;

  current++;
  return true;
}


void Buffer::advanceLINPast(const size_t pos)
{
  if (pos+1 >= lines[current].len)
  {
    current++;
    posLIN = 0;
  }
  else
    posLIN = static_cast<unsigned>(pos)+1;
}


bool Buffer::extendLINValue(LineData& vside)
{
  bool endFlag = false;
  while (current < len-1)
  {
    current++;
    size_t e = lines[current].line.find('|');
    if (e == string::npos)
      vside.value += lines[current].line;
    else
    {
      endFlag = true;
      if (e > 0)
        vside.value += lines[current].line.substr(0, e);
      Buffer::advanceLINPast(e);
      break;
    }
  }
  return endFlag;
}


bool Buffer::nextLIN(
  LineData& vside,
  const bool skipChat)
{
  while (current < len && 
      (lines[current].type == BRIDGE_BUFFER_COMMENT ||
       lines[current].type == BRIDGE_BUFFER_EMPTY))
    current++;

  if (current > len-1)
    return false;

  size_t e = lines[current].line.find('|', posLIN);
  if (e == string::npos &&
      current == len-1 && 
      (vside.label == "pg" ||
     (vside.label == "nt" && skipChat)))
  {
    return false;
  }

  if (e != posLIN+2 || e == string::npos)
  {
    THROW("Bad LIN line: " + lines[current].line + 
      ", remainder: " + lines[current].line.substr(posLIN) + 
      ", line " + STR(current+1) + ", posLIN " + 
      STR(posLIN) + ", e " + STR(e));
  }

  vside.label = lines[current].line.substr(posLIN, 2);
  vside.no = lines[current].no;

  posLIN += 3;
  if (posLIN >= lines[current].len)
  {
    if (current == len-1)
    {
      THROW("Bad LIN line: " + lines[current].line + 
        ", " + STR(current));
    }
    else
    {
      // Could be a continuation of nt| on the next line.
      current++;
      posLIN = 0;
    }
  }

  e = lines[current].line.find('|', posLIN);
  if (e == posLIN)
  {
    vside.value = "";
    Buffer::advanceLINPast(e);
  }
  else if (e == string::npos)
  {
    // Could be an unterminated tag.
    vside.value = lines[current].line.substr(posLIN);
    if (! Buffer::extendLINValue(vside))
      THROW("Bad LIN line: " + lines[current].line + 
        ", " + STR(current));
  }
  else
  {
    if (vside.label == "nt")
    {
      while (e+3 < lines[current].len &&
          lines[current].line.at(e+3) != '|')
      {
        // Attempt to complete the comment.
        e = lines[current].line.find('|', e+1);
        if (e == string::npos)
          break;
      }
    }

    if (e == string::npos)
    {
      vside.value = lines[current].line.substr(posLIN);
      if (! Buffer::extendLINValue(vside))
        THROW("Bad LIN line: " + lines[current].line + 
          ", " + STR(current));
    }
    else
    {
      vside.value = lines[current].line.substr(posLIN, e-posLIN);
      Buffer::advanceLINPast(e);
    }
  }

  vside.type = BRIDGE_BUFFER_STRUCTURED;
  vside.line = vside.label + "|" + vside.value + "|";
  vside.len = 4 + static_cast<unsigned>(vside.value.length());

  // Skip over various labels. 
  if ((skipChat && vside.label == "nt") || // Chat
      vside.label == "pg" || // "Page"
      vside.label == "ob" || // Bidding in other room
      vside.label == "sa" || // ?
      vside.label == "mn" || // An older header term
      vside.label == "em" || // ?
      vside.label == "at" || // ?
      vside.label == "bt" || // ?
      vside.label == "ht" || // ?
      vside.label == "it" || // ?
      vside.label == "pa" || // ?
      vside.label == "tu" || // ?
      vside.label == "pf" || // ?
      (vside.label == "mb" && vside.value == "-"))
    return Buffer::nextLIN(vside, skipChat);
  else
    return true;
}


void Buffer::nextRBX(LineData& vside)
{
  // Turn RBX into RBN.

  if (posRBX >= lines[current].len)
  {
    // Make an empty line.
    vside.line = "";
    vside.len = 0;
    vside.type = BRIDGE_BUFFER_EMPTY;
    vside.label = "";
    vside.value = "";
    vside.no = lines[current].no;
    current++;
    posRBX = 0;
    return;
  }

  size_t e = lines[current].line.find('}', posRBX);
  if (e <= posRBX+1 || e == string::npos)
    THROW("Bad RBX line");

  vside.label = lines[current].line.substr(posRBX, 1);
  vside.no = lines[current].no;

  if (e == posRBX+2)
  {
    vside.len = 2;
    vside.line = vside.label + " ";
    vside.value = "";
  }
  else
  {
    vside.len = static_cast<unsigned>(e)-posRBX;
    vside.value = lines[current].line.substr(posRBX+2, vside.len-2);
    vside.line = vside.label + " " + vside.value;
  }

  Buffer::classify(vside);
  posRBX = static_cast<unsigned>(e)+1;
}


bool Buffer::next(
  LineData& vside,
  const bool skipChat)
{
  if (current > len-1)
    return false;

  if (format == BRIDGE_FORMAT_LIN ||
      format == BRIDGE_FORMAT_LIN_VG ||
      format == BRIDGE_FORMAT_LIN_TRN)
  {
    return Buffer::nextLIN(vside, skipChat);
  }
  else if (format == BRIDGE_FORMAT_RBX &&
      lines[current].type != BRIDGE_BUFFER_EMPTY)
  {
    Buffer::nextRBX(vside);
    return true;
  }
  else
  {
    vside = lines[current];
    current++;
    return true;
  }
}


bool Buffer::previous(LineData& vside)
{
  if (current <= 1)
    return false;

  vside = lines[current-2];
  return true;
}


unsigned Buffer::previousHeaderStart() const
{
  unsigned u = current;
  while (u >= 2 && lines[u].type != BRIDGE_BUFFER_DASHES)
    u--;

  return lines[u+2].no;
}


unsigned Buffer::firstRS() const
{
  if (format != BRIDGE_FORMAT_LIN_VG)
    return BIGNUM;

  for (unsigned i = 0; i < len; i++)
  {
    if (lines[i].type == BRIDGE_BUFFER_STRUCTURED &&
        lines[i].label == "rs")
      return lines[i].no;
  }

  return BIGNUM;
}


unsigned Buffer::getInternalNumber(const unsigned no) const
{
  if (no == 0 || no > lines[len-1].no)
    return BIGNUM;
  
  unsigned i = no-1;
  if (i >= len)
    i = len-1;

  if (lines[i].no == no)
    return i;

  if (lines[i].no > no)
  {
    // Happens with deletions.
    while (i >= 1 && lines[i-1].no != no)
      i--;
    if (i == 0)
      return BIGNUM;
    else
      return i-1;
  }
  else
  {
    // Happens with insertions.
    while (i <= len && lines[i-1].no != no)
      i++;
    if (i > len)
      return BIGNUM;
    else
      return i-1;
  }
}


string Buffer::getLine(const unsigned no) const
{
  if (no == 0 || no > lines[len-1].no)
    return "";

  const unsigned intNo = Buffer::getInternalNumber(no);
  if (intNo == BIGNUM)
    return "";
  else
    return lines[intNo].line;
}


string Buffer::name() const
{
  return fileName;
}


int Buffer::peek() 
{
  if (format == BRIDGE_FORMAT_LIN ||
      format == BRIDGE_FORMAT_LIN_VG ||
      format == BRIDGE_FORMAT_LIN_TRN)
  {
    const unsigned current0 = current;
    const unsigned posLIN0 = posLIN;

    LineData lineData;
    if (! Buffer::nextLIN(lineData))
      return 0x00;

    current = current0;
    posLIN = posLIN0;

    return static_cast<int>(lineData.line.at(0));
  }
  else if (current > len-1)
    return 0x00;
  else if (lines[current].len == 0)
    return 0x01; // Whatever
  else 
    return static_cast<int>(lines[current].line.at(0));
}


void Buffer::print() const
{
  cout << setw(4) << right << "No" <<
    setw(6) << "Type" << setw(4) << "Len" << left << " " <<
    setw(12) << "Label" << "Value" << endl;

  for (auto &ld: lines)
  {
    if (ld.type == BRIDGE_BUFFER_STRUCTURED)
    {
      cout << setw(4) << right << ld.no <<
        setw(6) << static_cast<unsigned>(ld.type) << 
        setw(4) << ld.len << left << " " <<
        setw(12) << ld.label << ld.value << endl;
    }
    else
    {
      cout << setw(4) << right << ld.no <<
        setw(6) << static_cast<unsigned>(ld.type) << 
        setw(4) << ld.len << left << " " <<
        setw(12) << "" << ld.line << endl;
    }
  }
}

