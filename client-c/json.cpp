#include <vector>
#include <unistd.h>

#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

#include "runtime/header.h"
#include "runtime/alloc.h"

#include "init.h"

using namespace rapidjson;

static block * dotk = (block *)((((uint64_t)getTagForSymbolName("dotk{}")) << 32) | 1);
static struct blockheader boolHdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortBool{}, SortJSON{}}"));
static struct blockheader intHdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortInt{}, SortJSON{}}"));
static struct blockheader strHdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortString{}, SortJSON{}}"));
static block *dotList = (block *)((((uint64_t)getTagForSymbolName("Lbl'Stop'List'LBraQuot'JSONs'QuotRBraUnds'JSONs{}()")) << 32) | 1);
static block *null = (block *)((((uint64_t)getTagForSymbolName("LblJSONnull{}")) << 32) | 1);
static block *undef = (block *)((((uint64_t)getTagForSymbolName("Lblundef'Unds'JSON-RPC'Unds'JSON{}")) << 32) | 1);
static blockheader listHdr = getBlockHeaderForSymbol(getTagForSymbolName("LblJSONs{}"));
static blockheader membHdr = getBlockHeaderForSymbol(getTagForSymbolName("LblJSONEntry{}"));
static blockheader objHdr = getBlockHeaderForSymbol(getTagForSymbolName("LblJSONObject{}"));
static blockheader listWrapHdr = getBlockHeaderForSymbol(getTagForSymbolName("LblJSONList{}"));
static block * eof = (block *)((((uint64_t)getTagForSymbolName("Lbl'Hash'EOF{}")) << 32) | 1);
static blockheader ioErrorHdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortIOError{}, SortIOJSON{}}"));
static blockheader jsonHdr = getBlockHeaderForSymbol(getTagForSymbolName("inj{SortJSON{}, SortIOJSON{}}"));

class FDStream {
public:
    typedef char Ch;
    FDStream(int fd) : fd(fd) {
    }
    Ch Peek() const { // 1
        char c;
        int nread = recv(fd, &c, 1, MSG_PEEK);
        return nread ? c : '\0';
    }
    Ch Take() { // 2
        char c;
        int nread = read(fd, &c, 1);
        return nread ? c : '\0';
    }
    size_t Tell() const { return -1; } // 3

    Ch* PutBegin() { assert(false); return 0; }
    void Put(Ch c) { write(fd, &c, 1); }                  // 1
    void Flush() { }                   // 2
    size_t PutEnd(Ch*) { assert(false); return 0; }
private:
    FDStream(const FDStream&);
    FDStream& operator=(const FDStream&);
    int fd;
};

struct KoreHandler : BaseReaderHandler<UTF8<>, KoreHandler> {
  block *result;
  std::vector<block *> stack;

  bool Null() { stack.push_back(null); return true; }
  bool Bool(bool b) {
    boolinj *inj = (boolinj *)koreAlloc(sizeof(boolinj));
    inj->h = boolHdr;
    inj->data = b;
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool RawNumber(const char *str, SizeType length, bool copy) {
    zinj *inj = (zinj *)koreAlloc(sizeof(zinj));
    inj->h = intHdr;
    mpz_t z;
    mpz_init_set_str(z, str, 10);
    inj->data = move_int(z);
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool String(const char *str, SizeType len, bool copy) {
    stringinj *inj = (stringinj *)koreAlloc(sizeof(stringinj));
    inj->h = strHdr;
    string *token = (string *)koreAllocToken(sizeof(string) + len);
    set_len(token, len);
    memcpy(token->data, str, len);
    inj->data = token;
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
    for (int i = 0; i < memberCount; i++) {
      jsonmember *member = (jsonmember *)koreAlloc(sizeof(jsonmember));
      member->h = membHdr;
      member->val = stack.back();
      stack.pop_back();
      member->key = stack.back();
      stack.pop_back();
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
    for (int i = 0; i < elementCount; i++) {
      jsonlist *list = (jsonlist *)koreAlloc(sizeof(jsonlist));
      list->h = listHdr;
      list->hd = stack.back();
      stack.pop_back();
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

struct KoreWriter : Writer<FDStream> {
  bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false) {
    (void)copy;
    Prefix(rapidjson::kNumberType);
    return EndValue(WriteRawValue(str, length));
  }

  KoreWriter(FDStream &os) : Writer(os) {}
};

bool write_json(KoreWriter &writer, block *data) {
  bool return_value = true;
  if (data != dotList) {
    if (data == null) {
      writer.Null();
    } else if (data->h.hdr == boolHdr.hdr) {
      boolinj *inj = (boolinj *)data;
      writer.Bool(inj->data);
    } else if (data->h.hdr == intHdr.hdr) {
      zinj *inj = (zinj *)data;
      string *str = hook_STRING_int2string(inj->data);
      writer.RawNumber(str->data, len(str), false);
    } else if (data->h.hdr == strHdr.hdr) {
      stringinj *inj = (stringinj *)data;
      writer.String(inj->data->data, len(inj->data), false);
    } else if (data->h.hdr == objHdr.hdr) {
      writer.StartObject();
      json *obj = (json *)data;
      return_value = write_json(writer, (block *)obj->data);
      writer.EndObject();
    } else if (data->h.hdr == listWrapHdr.hdr) {
      writer.StartArray();
      json *obj = (json *)data;
      return_value = write_json(writer, (block *)obj->data);
      writer.EndArray();
    } else if (data->h.hdr == listHdr.hdr) {
      jsonlist *list = (jsonlist *)data;
      return_value = write_json(writer, list->hd) && write_json(writer, (block *)list->tl);
    } else if (data->h.hdr == membHdr.hdr) {
      jsonmember *memb = (jsonmember *)data;
      stringinj *inj = (stringinj *)memb->key;
      writer.Key(inj->data->data, len(inj->data), false);
      return_value = write_json(writer, memb->val);
    } else {
      return_value = false;
    }
  }
  return return_value;
}

extern "C" {

struct block *hook_JSON_read(mpz_t fd_z) {
  KoreHandler handler;
  Reader reader;
  int fd = mpz_get_si(fd_z);
  FDStream is(fd);

  bool result = reader.Parse<kParseStopWhenDoneFlag | kParseNumbersAsStringsFlag>(is, handler);
  if (result) {
    block *semifinal = handler.stack.back();
    if (semifinal->h.hdr == objHdr.hdr || semifinal->h.hdr == listWrapHdr.hdr) {
      inj *res = (inj *)koreAlloc(sizeof(inj));
      res->h = jsonHdr;
      res->data = semifinal;
      return (block *)res;
    } else {
      return semifinal;
    }
  } else if (reader.GetParseErrorCode() == kParseErrorDocumentEmpty) {
    inj *error = (inj *)koreAlloc(sizeof(inj));
    error->h = ioErrorHdr;
    error->data = eof;
    return (block *)error;
  } else {
    inj *res = (inj *)koreAlloc(sizeof(inj));
    res->h = jsonHdr;
    res->data = undef;
    return (block *)res;
  }
}

block *hook_JSON_write(block *json, mpz_ptr fd_z) {
  int fd = mpz_get_si(fd_z);
  FDStream os(fd);
  KoreWriter writer(os);

  if (! write_json(writer, json)) {
    printConfiguration("/dev/stderr", json);
    abort();
  }
  return dotk;
}

}
