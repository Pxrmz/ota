#include "adm_test_tool.h"

#include <fstream>
#include <iostream>

void adm_test_tool_readConfig(Json::Value* root, const DCHAR *path){
  std::ifstream ifs;
  ifs.open(path);

  Json::CharReaderBuilder builder;
  JSONCPP_STRING errs;
  if (!parseFromStream(builder, ifs, root, &errs)) {
    std::cout << errs << std::endl;
  }
  ifs.close();
}

