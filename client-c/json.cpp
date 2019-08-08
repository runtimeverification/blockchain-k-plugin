#include <vector>
#include <unistd.h>

#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

#include "runtime/header.h"
#include "runtime/alloc.h"

using namespace rapidjson;

extern "C" {
  string *hook_STRING_int2string(mpz_ptr);
}

typedef struct binj {
  struct blockheader h;
  bool b;
} binj;

typedef struct zinj {
  struct blockheader h;
  mpz_ptr z;
} zinj;

typedef struct sinj {
  struct blockheader h;
  string *s;
} sinj;

struct jsonlist {
  blockheader h;
  block *hd;
  jsonlist* tl;
};

struct json {
  blockheader h;
  jsonlist *data;
};

struct jsonmember {
  blockheader h;
  block *key;
  block *val;
};

static block * dotk = (block *)((((uint64_t)getTagForSymbolName("dotk{}")) << 32) | 1);
static struct blockheader boolHdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortBool{}, SortJSON{}}"));
static struct blockheader intHdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortInt{}, SortJSON{}}"));
static struct blockheader strHdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortString{}, SortJSON{}}"));
static block *dotList = (block *)((((uint64_t)getTagForSymbolName("Lbl'Stop'List'LBraQuotUndsCommUndsUnds'EVM-DATA'UndsUnds'JSON'Unds'JSONList'QuotRBraUnds'JSONList{}")) << 32) | 1);
static blockheader listHdr = getBlockHeaderForSymbol(getTagForSymbolName("Lbl'UndsCommUndsUnds'EVM-DATA'UndsUnds'JSON'Unds'JSONList{}"));
static blockheader membHdr = getBlockHeaderForSymbol(getTagForSymbolName("Lbl'UndsColnUndsUnds'EVM-DATA'UndsUnds'JSONKey'Unds'JSON{}"));
static blockheader objHdr = getBlockHeaderForSymbol(getTagForSymbolName("Lbl'LBraUndsRBraUnds'EVM-DATA'UndsUnds'JSONList{}"));
static blockheader listWrapHdr = getBlockHeaderForSymbol(getTagForSymbolName("Lbl'LSqBUndsRSqBUnds'EVM-DATA'UndsUnds'JSONList{}"));

struct KoreHandler : BaseReaderHandler<UTF8<>, KoreHandler> {
  block *result;
  std::vector<block *> stack;

  bool Null() { return false; }
  bool Bool(bool b) {
    binj *inj = (binj *)koreAlloc(sizeof(binj));
    inj->h = boolHdr;
    inj->b = b;
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool Int(int i) {
    zinj *inj = (zinj *)koreAlloc(sizeof(zinj));
    inj->h = intHdr;
    mpz_t z;
    mpz_init_set_si(z, i);
    inj->z = move_int(z);
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool Uint(unsigned i) {
    zinj *inj = (zinj *)koreAlloc(sizeof(zinj));
    inj->h = intHdr;
    mpz_t z;
    mpz_init_set_ui(z, i);
    inj->z = move_int(z);
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool Int64(int64_t i) {
    zinj *inj = (zinj *)koreAlloc(sizeof(zinj));
    inj->h = intHdr;
    mpz_t z;
    mpz_init_set_si(z, i);
    inj->z = move_int(z);
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool Uint64(uint64_t i) {
    zinj *inj = (zinj *)koreAlloc(sizeof(zinj));
    inj->h = intHdr;
    mpz_t z;
    mpz_init_set_ui(z, i);
    inj->z = move_int(z);
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool Double(double d) { return false; }

  bool String(const char *str, SizeType len, bool copy) {
    sinj *inj = (sinj *)koreAlloc(sizeof(sinj));
    inj->h = strHdr;
    string *token = (string *)koreAllocToken(len);
    set_len(token, len);
    memcpy(token->data, str, len);
    inj->s = token;
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool StartObject() { return true; }
  
  bool Key(const char *str, SizeType len, bool copy) {
    return String(str, len, copy);
  }


  bool EndObject(SizeType memberCount) {
    result = dotList;
    std::vector<block *> children;
    for (int i = 0; i < memberCount; i++) {
      children.push_back(stack.back());
      stack.pop_back();
      children.push_back(stack.back());
      stack.pop_back();
    }
    for (int i = 0; i < memberCount; i++) {
      jsonmember *member = (jsonmember *)koreAlloc(sizeof(jsonmember));
      member->h = membHdr;
      member->key = children.back();
      children.pop_back();
      member->val = children.back();
      children.pop_back();
      jsonlist *list = (jsonlist *)koreAlloc(sizeof(jsonlist));
      list->h = listHdr;
      list->hd = (block *)member;
      list->tl = (jsonlist *)result;
      result = (block *)list;
    }
    json *wrap = (json *)koreAlloc(sizeof(json));
    wrap->h = objHdr;
    wrap->data = (jsonlist *)result;
    stack.push_back((block *)wrap);
    return true;
  }

  bool StartArray() { return true; }

  bool EndArray(SizeType elementCount) {
    result = dotList;
    std::vector<block *> children;
    for (int i = 0; i < elementCount; i++) {
      children.push_back(stack.back());
      stack.pop_back();
    }
    for (int i = 0; i < elementCount; i++) {
      jsonlist *list = (jsonlist *)koreAlloc(sizeof(jsonlist));
      list->h = listHdr;
      list->hd = children.back();
      children.pop_back();
      list->tl = (jsonlist *)result;
      result = (block *)list;
    }
    json *wrap = (json *)koreAlloc(sizeof(json));
    wrap->h = listWrapHdr;
    wrap->data = (jsonlist *)result;
    stack.push_back((block *)wrap);
    return true;
  }
};

void write_json(Writer<FileWriteStream> &writer, block *data) {
  if (data == dotList) {
    return;
  }
  if (data->h.hdr == boolHdr.hdr) {
    binj *inj = (binj *)data;
    writer.Bool(inj->b);
  } else if (data->h.hdr == intHdr.hdr) {
    zinj *inj = (zinj *)data;
    string *str = hook_STRING_int2string(inj->z);
    writer.RawNumber(str->data, len(str), false);
  } else if (data->h.hdr == strHdr.hdr) {
    sinj *inj = (sinj *)data;
    writer.String(inj->s->data, len(inj->s), false);
  } else if (data->h.hdr == objHdr.hdr) {
    writer.StartObject();
    json *obj = (json *)data;
    write_json(writer, (block *)obj->data);
    writer.EndObject();
  } else if (data->h.hdr == listWrapHdr.hdr) {
    writer.StartArray();
    json *obj = (json *)data;
    write_json(writer, (block *)obj->data);
    writer.EndArray();
  } else if (data->h.hdr == listHdr.hdr) {
    jsonlist *list = (jsonlist *)data;
    write_json(writer, list->hd);
    write_json(writer, (block *)list->tl);
  } else if (data->h.hdr == membHdr.hdr) {
    jsonmember *memb = (jsonmember *)data;
    sinj *inj = (sinj *)memb->key;
    writer.Key(inj->s->data, len(inj->s), false);
    write_json(writer, memb->val);
  } else {
    abort();
  }
}

extern "C" {

struct block *hook_JSON_read(mpz_t fd_z) {
  KoreHandler handler;
  Reader reader;
  int fd = mpz_get_si(fd_z);
  FILE *f = fdopen(dup(fd), "r");
  char readBuffer[4096];
  FileReadStream is(f, readBuffer, sizeof(readBuffer));

  reader.Parse(is, handler);
  fclose(f);
  return handler.result;
}

block *hook_JSON_write(block *json, mpz_ptr fd_z) {
  int fd = mpz_get_si(fd_z);
  FILE *f = fdopen(dup(fd), "w");
  char writeBuffer[4096];
  FileWriteStream os(f, writeBuffer, sizeof(writeBuffer));
  Writer<FileWriteStream> writer(os);

  write_json(writer, json);
  fclose(f);
  return dotk;
}

}
