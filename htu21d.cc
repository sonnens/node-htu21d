#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "htu21d.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Htu21d::Init(exports);
}

NODE_MODULE(htu21d, InitAll)
