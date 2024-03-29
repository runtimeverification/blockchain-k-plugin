#include <unistd.h>

#include <vector>

#include "k.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "runtime/alloc.h"
#include "runtime/header.h"

using namespace rapidjson;

static block *dotk =
    (block *)((((uint64_t)get_tag_for_symbol_name("dotk{}")) << 32) | 1);
static blockheader kseqHeader = {
    get_block_header_for_symbol((uint64_t)get_tag_for_symbol_name("kseq{}"))};
static struct blockheader boolHdr = get_block_header_for_symbol(
    get_tag_for_symbol_name("inj{SortBool{}, SortJSON{}}"));
static struct blockheader intHdr = get_block_header_for_symbol(
    get_tag_for_symbol_name("inj{SortInt{}, SortJSON{}}"));
static struct blockheader strHdr = get_block_header_for_symbol(
    get_tag_for_symbol_name("inj{SortString{}, SortJSON{}}"));
static block *dotList =
    (block *)((((uint64_t)get_tag_for_symbol_name(
                   "Lbl'Stop'List'LBraQuot'JSONs'QuotRBraUnds'JSONs{}"))
               << 32) |
              1);
static block *null =
    (block *)((((uint64_t)get_tag_for_symbol_name("LblJSONnull{}")) << 32) | 1);
static block *undef =
    (block *)((((uint64_t)get_tag_for_symbol_name("LblJSON-RPCundef{}"))
               << 32) |
              1);
static blockheader listHdr =
    get_block_header_for_symbol(get_tag_for_symbol_name("LblJSONs{}"));
static blockheader membHdr =
    get_block_header_for_symbol(get_tag_for_symbol_name("LblJSONEntry{}"));
static blockheader objHdr =
    get_block_header_for_symbol(get_tag_for_symbol_name("LblJSONObject{}"));
static blockheader listWrapHdr =
    get_block_header_for_symbol(get_tag_for_symbol_name("LblJSONList{}"));
static block *eof =
    (block *)((((uint64_t)get_tag_for_symbol_name("Lbl'Hash'EOF{}")) << 32) |
              1);
static blockheader ioErrorHdr = get_block_header_for_symbol(
    get_tag_for_symbol_name("inj{SortIOError{}, SortIOJSON{}}"));
static blockheader jsonHdr = get_block_header_for_symbol(
    get_tag_for_symbol_name("inj{SortJSON{}, SortIOJSON{}}"));
static blockheader jsonPutResponseErrorHdr = get_block_header_for_symbol(
    get_tag_for_symbol_name("LblJSON-RPC'Unds'putResponseError{}"));

class FDStream {
 public:
  typedef char Ch;
  FDStream(int fd) : fd(fd) { fillreadbuf(); }
  Ch Peek() {  // 1
    if (rsize > 0) return readbuf[rhead];
    if (fillreadbuf()) {
      return readbuf[rhead];
    } else {
      return '\0';
    }
  }
  Ch Take() {  // 2
    Ch c = Peek();
    if (c) {
      rhead = (rhead + 1) % BUF_SIZE;
      rsize--;
    }
    return c;
  }
  size_t Tell() const { return -1; }  // 3

  Ch *PutBegin() {
    assert(false);
    return 0;
  }
  void Put(Ch c) {  // 1
    if (wsize == BUF_SIZE) Flush();
    writebuf[wsize++] = c;
  }
  void Flush() {  // 2
    write(fd, writebuf, wsize);
    wsize = 0;
  }
  size_t PutEnd(Ch *) {
    assert(false);
    return 0;
  }

 private:
  FDStream(const FDStream &);
  FDStream &operator=(const FDStream &);
  bool fillreadbuf() {
    bool bytesread = false;
    int readstart = (rhead + rsize) % BUF_SIZE;
    int readlength = ((readstart >= rhead) ? BUF_SIZE : rhead) - readstart;
    int n = read(fd, &readbuf[readstart], readlength);
    if (n > 0) {
      bytesread = true;
      rsize += n;
    }
    return bytesread;
  }
  int fd;
  static const int BUF_SIZE = 8192;
  Ch readbuf[BUF_SIZE], writebuf[BUF_SIZE];
  int rhead = 0, rsize = 0, wsize = 0;
};

struct KoreHandler : BaseReaderHandler<UTF8<>, KoreHandler> {
  block *result;
  std::vector<block *> stack;

  bool Null() {
    stack.push_back(null);
    return true;
  }
  bool Bool(bool b) {
    boolinj *inj = (boolinj *)kore_alloc(sizeof(boolinj));
    inj->h = boolHdr;
    inj->data = b;
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool RawNumber(const char *str, SizeType length, bool copy) {
    zinj *inj = (zinj *)kore_alloc(sizeof(zinj));
    inj->h = intHdr;
    mpz_t z;
    mpz_init_set_str(z, str, 10);
    inj->data = move_int(z);
    result = (block *)inj;
    stack.push_back(result);
    return true;
  }

  bool String(const char *str, SizeType len, bool copy) {
    stringinj *inj = (stringinj *)kore_alloc(sizeof(stringinj));
    inj->h = strHdr;
    string *token = (string *)kore_alloc_token(sizeof(string) + len);
    init_with_len(token, len);
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
      jsonmember *member = (jsonmember *)kore_alloc(sizeof(jsonmember));
      member->h = membHdr;
      member->val = stack.back();
      stack.pop_back();
      member->key = stack.back();
      stack.pop_back();
      jsonlist *list = (jsonlist *)kore_alloc(sizeof(jsonlist));
      list->h = listHdr;
      list->hd = (block *)member;
      list->tl = (jsonlist *)result;
      result = (block *)list;
    }
    json *wrap = (json *)kore_alloc(sizeof(json));
    wrap->h = objHdr;
    wrap->data = (jsonlist *)result;
    stack.push_back((block *)wrap);
    return true;
  }

  bool StartArray() { return true; }

  bool EndArray(SizeType elementCount) {
    result = dotList;
    for (int i = 0; i < elementCount; i++) {
      jsonlist *list = (jsonlist *)kore_alloc(sizeof(jsonlist));
      list->h = listHdr;
      list->hd = stack.back();
      stack.pop_back();
      list->tl = (jsonlist *)result;
      result = (block *)list;
    }
    json *wrap = (json *)kore_alloc(sizeof(json));
    wrap->h = listWrapHdr;
    wrap->data = (jsonlist *)result;
    stack.push_back((block *)wrap);
    return true;
  }
};

struct KoreWriter : Writer<FDStream> {
  bool RawNumber(const Ch *str, rapidjson::SizeType length, bool copy = false) {
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
    } else if (tag_hdr(data->h.hdr) == tag_hdr(boolHdr.hdr)) {
      boolinj *inj = (boolinj *)data;
      writer.Bool(inj->data);
    } else if (tag_hdr(data->h.hdr) == tag_hdr(intHdr.hdr)) {
      zinj *inj = (zinj *)data;
      string *str = hook_STRING_int2string(inj->data);
      writer.RawNumber(str->data, len(str), false);
    } else if (tag_hdr(data->h.hdr) == tag_hdr(strHdr.hdr)) {
      stringinj *inj = (stringinj *)data;
      writer.String(inj->data->data, len(inj->data), false);
    } else if (tag_hdr(data->h.hdr) == tag_hdr(objHdr.hdr)) {
      writer.StartObject();
      json *obj = (json *)data;
      return_value = write_json(writer, (block *)obj->data);
      writer.EndObject();
    } else if (tag_hdr(data->h.hdr) == tag_hdr(listWrapHdr.hdr)) {
      writer.StartArray();
      json *obj = (json *)data;
      return_value = write_json(writer, (block *)obj->data);
      writer.EndArray();
    } else if (tag_hdr(data->h.hdr) == tag_hdr(listHdr.hdr)) {
      jsonlist *list = (jsonlist *)data;
      return_value =
          write_json(writer, list->hd) && write_json(writer, (block *)list->tl);
    } else if (tag_hdr(data->h.hdr) == tag_hdr(membHdr.hdr)) {
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

  bool result =
      reader.Parse<kParseStopWhenDoneFlag | kParseNumbersAsStringsFlag>(
          is, handler);
  if (result) {
    block *semifinal = handler.stack.back();
    if (semifinal->h.hdr == objHdr.hdr || semifinal->h.hdr == listWrapHdr.hdr) {
      inj *res = (inj *)kore_alloc(sizeof(inj));
      res->h = jsonHdr;
      res->data = semifinal;
      return (block *)res;
    } else {
      return semifinal;
    }
  } else if (reader.GetParseErrorCode() == kParseErrorDocumentEmpty) {
    inj *error = (inj *)kore_alloc(sizeof(inj));
    error->h = ioErrorHdr;
    error->data = eof;
    return (block *)error;
  } else {
    inj *res = (inj *)kore_alloc(sizeof(inj));
    res->h = jsonHdr;
    res->data = undef;
    return (block *)res;
  }
}

block *hook_JSON_write(block *json, mpz_ptr fd_z) {
  int fd = mpz_get_si(fd_z);
  FDStream os(fd);
  KoreWriter writer(os);

  if (!write_json(writer, json)) {
    block *retBlock = (block *)kore_alloc(sizeof(block) + 2 * sizeof(block *));
    retBlock->h = kseqHeader;

    inj *res = (inj *)kore_alloc(sizeof(inj));
    res->h = jsonPutResponseErrorHdr;
    res->data = json;

    memcpy(retBlock->children, &res, sizeof(block *));
    memcpy(retBlock->children + 1, &dotk, sizeof(block *));

    return retBlock;
  }
  return dotk;
}
}
