#include <node.h>
#include <string>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <linux/i2c-dev.h>

#include "htu21d.h"

using namespace v8;
using namespace std;

Persistent<Function> Htu21d::constructor;

Htu21d::Htu21d(char* dev, int addr) {
  fd = open(dev, O_RDWR);
  if (fd < 0) {
    ThrowException(Exception::Error(String::New("Failed to open the i2c bus")));
  }
  if (ioctl(fd, I2C_SLAVE, addr) < 0) {
    ThrowException(Exception::Error(String::New("Failed to acquire bus access and/or talk to slave.")));
  }
  address = addr;
  device = std::string(dev);
}

Htu21d::~Htu21d() {
  close(fd);
}

static v8::Handle<Value>
GetAddress(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
  Htu21d* obj = node::ObjectWrap::Unwrap<Htu21d>(info.Holder()); 
  return Number::New(obj->address);
}

static void
SetAddress(Local<String> property, Local<Value> value, const AccessorInfo& info) {
  Htu21d* obj = node::ObjectWrap::Unwrap<Htu21d>(info.Holder());
  if (value->IsInt32())
    obj->reaquire(&(obj->device), (int) value->ToInt32()->Value());
}

static v8::Handle<Value>
GetDevice(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
  Htu21d* obj = node::ObjectWrap::Unwrap<Htu21d>(info.Holder()); 
  return String::New(obj->device.c_str());
}

static void
SetDevice(Local<String> property, Local<Value> value, const AccessorInfo& info) {
  Htu21d* obj = node::ObjectWrap::Unwrap<Htu21d>(info.Holder()); 
  v8::String::Utf8Value v8str(value);
  string str = string(*v8str);
  obj->reaquire(&str, obj->address);
}

void
Htu21d::reaquire(string* dev, int addr) {

  close(fd);

  fd = open(dev->c_str(), O_RDWR);
  if (fd < 0) {
    ThrowException(Exception::Error(String::New("Failed to open the i2c bus")));
  }
  if (ioctl(fd, I2C_SLAVE, addr) < 0) {
    ThrowException(Exception::Error(String::New("Failed to acquire bus access and/or talk to slave.")));
  }
  address = addr;
  device = *dev;

}

void Htu21d::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Htu21d"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set(String::NewSymbol("humidity"),
      FunctionTemplate::New(Humidity)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("temperature"),
      FunctionTemplate::New(Temp)->GetFunction());
  //tpl->InstanceTemplate()->SetAccessor(String::New("resolution"), GetRes, SetRes);
  tpl->InstanceTemplate()->SetAccessor(String::New("address"), GetAddress, SetAddress);
  tpl->InstanceTemplate()->SetAccessor(String::New("devnode"), GetDevice, SetDevice);
  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Htu21d"), constructor);
}

Handle<Value>
Htu21d::Humidity(const v8::Arguments& args) {

  HandleScope scope;
  Htu21d* obj = ObjectWrap::Unwrap<Htu21d>(args.This());

  char buf[3];
  useconds_t delay = 50000;

  buf[0] = 0xF5 ;

  if (write(obj->fd,buf,1) != 1) {
    ThrowException(Exception::Error(String::New("Could not write to I2C")));
    return Undefined();
  }

  usleep(delay);

  int i = read(obj->fd,buf,3);
  if (i != 3) {
    ThrowException(Exception::Error(String::New("Could not read from I2C")));
    return Undefined();
  }
  double hum = (double) ((buf[0] << 8) + (buf[1] & 254));
  hum = ((125 * hum) / 65536) - 6;

  return scope.Close(Number::New(hum));
}


Handle<Value>
Htu21d::Temp(const v8::Arguments& args) {
  HandleScope scope;

  Htu21d* obj = ObjectWrap::Unwrap<Htu21d>(args.This());

  char buf[3];

  buf[0] = 0xF3;

  if (write(obj->fd,buf,1) != 1) {
    ThrowException(Exception::Error(String::New("Could not write to I2C")));
    return Undefined();
  }
  useconds_t delay = 50000;
  usleep(delay);
  int i = read(obj->fd,buf,3);
  if (i != 3) {
    ThrowException(Exception::Error(String::New("Could not read from I2C")));
    return Undefined();
  }
  double temp = (double) ((buf[0] << 8) + (buf[1] & 254));
  temp = -46.85 + (175.72 * temp) / 65536;

  return scope.Close(Number::New(temp));
}

Handle<Value>
Htu21d::New(const Arguments& args) {
  HandleScope scope;

  if (args.IsConstructCall()) {
    Local<String> device = args[0]->IsUndefined() ? String::New("/dev/i2c-1") : args[0]->ToString();
    int address = args[1]->IsUndefined() ? 0x40 : args[1]->IntegerValue();
    //std::string dev_std_str(*v8::String::Utf8Value(device));
    Htu21d* obj = new Htu21d(*v8::String::AsciiValue(device), address);
    obj->Wrap(args.This());
    return args.This();
  } else {
    // Invoked as plain function `Htu21d(...)`, turn into construct call.
    const int argc = 2;
    Local<Value> argv[argc] = { args[0], args[1] };
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}
