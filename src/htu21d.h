#ifndef HTU21D_H
#define HTU21D_H

#include <node.h>
using namespace v8;
using namespace std;

class Htu21d : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  explicit Htu21d(char* device, int address);
  ~Htu21d();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Humidity(const v8::Arguments& args);
  static v8::Handle<v8::Value> Temp(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;
  int fd;
};

#endif
