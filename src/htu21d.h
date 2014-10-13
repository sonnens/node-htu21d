#ifndef HTU21D_H
#define HTU21D_H

#include <node.h>
#include <string>

using namespace v8;
using namespace std;

#define HTU21D_DEFAULT_ADDRESS		0x40

#define HTU21D_RESOLUTION_12BITS	0b00000000
#define HTU21D_RESOLUTION_8BITS		0b00000001
#define HTU21D_RESOLUTION_10BITS	0b10000000
#define HTU21D_RESOLUTION_11BITS	0b10000001
#define HTU21D_END_OF_BATTERY		0b01000000
#define HTU21D_ENABLE_ONCHIP_HEATER	0b00000100
#define HTU21D_DISABLE_ONCHIP_HEATER	0b00000000
#define HTU21D_ENABLE_OTP_RELOAD	0b00000000
#define HTU21D_DISABLE_OTP_RELOAD	0b00000010
#define HTU21D_RESERVED_BITMASK		0b00111000

#define HTU21D_CMD_TEMPERATURE		0xF3
#define HTU21D_CMD_HUMIDITY		0xF5
#define HTU21D_CMD_WRITE_CONFIG		0xE6
#define HTU21D_CMD_READ_CONFIG		0xE7
#define HTU21D_CMD_SOFT_RESET		0xFE

#define HTU21D_STATUS_BITMASK		0b00000011
#define HTU21D_STATUS_TEMPERATURE	0b00000000
#define HTU21D_STATUS_HUMIDITY		0b00000010
#define HTU21D_STATUS_LSBMASK		0b11111100

class Htu21d : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  int address;
  std::string device;

  unsigned char user_reg;
  void updateStatusReg();
 private:
  explicit Htu21d(char* device, int address);
  ~Htu21d();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Humidity(const v8::Arguments& args);
  static v8::Handle<v8::Value> Temp(const v8::Arguments& args);
  static v8::Handle<v8::Value> Reset(const v8::Arguments& args);
  static v8::Handle<v8::Value> Mode(const v8::Arguments& args);
  static v8::Handle<v8::Value> Heater(const v8::Arguments& args);
  
  static v8::Persistent<v8::Function> constructor;

  int fd;

};

#endif
