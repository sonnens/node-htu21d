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

static v8::Handle<Value>
GetDevice(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
  Htu21d* obj = node::ObjectWrap::Unwrap<Htu21d>(info.Holder()); 
  return String::New(obj->device.c_str());
}

void
Htu21d::updateStatusReg() {
  unsigned char buf = HTU21D_CMD_READ_CONFIG;

  useconds_t delay = 15000;

  if (write(fd,&buf,1) != 1) {
    ThrowException(Exception::Error(String::New("Could not write to I2C")));
    return;
  }
  usleep(delay);
  int i = read(fd,&buf,1);
  if (i != 1) {
    ThrowException(Exception::Error(String::New("Could not read from I2C")));
    return;
  }
  user_reg = buf;
}

static v8::Handle<Value>
GetResolution(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
  Htu21d* obj = node::ObjectWrap::Unwrap<Htu21d>(info.Holder()); 
  char temp, rh;
  char res = obj->user_reg & 0b10000001;
  switch (res) {
    case HTU21D_RESOLUTION_12BITS:
      temp = 14;
      rh = 12;
      break;
    case HTU21D_RESOLUTION_8BITS:
      temp = 12;
      rh = 8;
      break;
    case HTU21D_RESOLUTION_10BITS:
      temp = 10;
      rh = 13;
      break;
    case HTU21D_RESOLUTION_11BITS:
      temp = 11;
      rh = 11;
      break;
    default:
      temp = -1;
      rh = -1;
      break;
  }
  Local<Object> resobj = Object::New();
  resobj->Set(String::NewSymbol("temperature"), Number::New(temp));
  resobj->Set(String::NewSymbol("humidity"), Number::New(rh));
  return resobj;
}

static v8::Handle<Value>
GetBattery(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
  Htu21d* obj = node::ObjectWrap::Unwrap<Htu21d>(info.Holder()); 

  // if we're reading the battery status, probably want to trigger a register read
  obj->updateStatusReg();

  return Boolean::New(obj->user_reg & HTU21D_END_OF_BATTERY);
}

static v8::Handle<Value>
GetHeater(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
  Htu21d* obj = node::ObjectWrap::Unwrap<Htu21d>(info.Holder()); 

  obj->updateStatusReg();
  return Boolean::New(obj->user_reg & HTU21D_ENABLE_ONCHIP_HEATER);
}

void Htu21d::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Htu21d"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set(String::NewSymbol("humidity"),
      FunctionTemplate::New(Humidity)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("temperature"),
      FunctionTemplate::New(Temp)->GetFunction());
/*
  tpl->PrototypeTemplate()->Set(String::NewSymbol("set_resolution"),
      FunctionTemplate::New(Resolution)->GetFunction());
*/
  tpl->InstanceTemplate()->SetAccessor(String::New("address"), GetAddress);
  tpl->InstanceTemplate()->SetAccessor(String::New("devnode"), GetDevice);
  tpl->InstanceTemplate()->SetAccessor(String::New("resolution"), GetResolution);
  tpl->InstanceTemplate()->SetAccessor(String::New("lowbattery"), GetBattery);
  tpl->InstanceTemplate()->SetAccessor(String::New("heater"), GetHeater);

  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Htu21d"), constructor);
}

Handle<Value>
Htu21d::Humidity(const v8::Arguments& args) {

  HandleScope scope;
  Htu21d* obj = ObjectWrap::Unwrap<Htu21d>(args.This());

  char buf[3];
  useconds_t delay = 50000;

  buf[0] = HTU21D_CMD_HUMIDITY;

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

  buf[0] = HTU21D_CMD_TEMPERATURE;

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
    int address = args[1]->IsUndefined() ? HTU21D_DEFAULT_ADDRESS : args[1]->IntegerValue();
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
