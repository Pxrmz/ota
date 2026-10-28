#include "adm_com_hash.h"
#include "gtest/gtest.h"
#include "json/json.h"
#include "adm_test_tool.h"

namespace{

const DCHAR* m_configPath = "./adm_common_hash_config.json";

TEST(common_hash, common_hmacmd5){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    Json::Value arr = root["hmacmd5"];
    for(D32S i=0; i<arr.size(); ++i){
	const std::string in = arr[i]["in"].asString();
	const std::string out = arr[i]["out"].asString();
	const std::string key = arr[i]["key"].asString();
	DCHAR resOut[256] = {0};
	adm_com_hmacmd5((D8U*)in.c_str(), in.length(), (D8U*)key.c_str(), key.length(), resOut, sizeof(resOut)/sizeof(resOut[0]));
	EXPECT_STREQ(out.c_str(), resOut);
    }
}

TEST(common_hash, common_hmacsha256){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    Json::Value arr = root["hmacsha256"];
    for(D32S i=0; i<arr.size(); ++i){
	const std::string in = arr[i]["in"].asString();
	const std::string out = arr[i]["out"].asString();
	const std::string key = arr[i]["key"].asString();
	DCHAR resOut[256] = {0};
	adm_com_hmacsha256((D8U*)in.c_str(), in.length(), (D8U*)key.c_str(), key.length(), resOut, sizeof(resOut)/sizeof(resOut[0]));
	EXPECT_STREQ(out.c_str(), resOut);
    }
}

TEST(common_hash, common_md5_string){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    Json::Value arr = root["md5string"];
    for(D32S i=0; i<arr.size(); ++i){
	const std::string in = arr[i]["in"].asString();
	const std::string out = arr[i]["out"].asString();
	DCHAR resOut[256] = {0};
	adm_com_md5((D8U*)in.c_str(), in.length(), resOut, sizeof(resOut)/sizeof(resOut[0]));
	EXPECT_STREQ(out.c_str(), resOut);
    }
}

TEST(common_hash, common_md5_file){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    Json::Value arr = root["md5file"];
    for(D32S i=0; i<arr.size(); ++i){
	const std::string infile = arr[i]["infile"].asString();
	const std::string out = arr[i]["out"].asString();
	DCHAR resOut[256] = {0};
	adm_com_md5File(infile.c_str(), resOut, sizeof(resOut)/sizeof(resOut[0]));
	EXPECT_STREQ(out.c_str(), resOut);
    }
}

TEST(common_hash, common_sha256_string){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    Json::Value arr = root["sha256string"];
    for(D32S i=0; i<arr.size(); ++i){
	const std::string in = arr[i]["in"].asString();
	const std::string out = arr[i]["out"].asString();
	DCHAR resOut[256] = {0};
	adm_com_sha256((D8U*)in.c_str(), in.length(), resOut, sizeof(resOut)/sizeof(resOut[0]));
	EXPECT_STREQ(out.c_str(), resOut);
    }
}

TEST(common_hash, common_sha256_file){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    Json::Value arr = root["sha256file"];
    for(D32S i=0; i<arr.size(); ++i){
	const std::string infile = arr[i]["infile"].asString();
	const std::string out = arr[i]["out"].asString();
	DCHAR resOut[256] = {0};
	adm_com_sha256File(infile.c_str(), resOut, sizeof(resOut)/sizeof(resOut[0]));
	EXPECT_STREQ(out.c_str(), resOut);
    }
}




}
