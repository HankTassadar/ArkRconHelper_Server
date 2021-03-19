#include "JsonOperate.h"

JsonOperate::JsonOperate() 
    :readerBuilder()
    ,jsonReader(readerBuilder.newCharReader())
{
}

JsonOperate::~JsonOperate()
{
	delete(this->jsonReader);
}

bool JsonOperate::openFile(const char* jsonFilePath) {
	
    ifstream file(jsonFilePath);
	if (!file.good()) {
		throw("Open JsonFile Failed!");
		return false;
	}		
    string dataStr;
    stringstream buffer;
    buffer << file.rdbuf();
    dataStr = buffer.str();
	file.close();
	return this->openStr(dataStr);
}

bool JsonOperate::openStr(std::string &dataStr)
{
	return this->jsonReader->parse(dataStr.c_str(), dataStr.c_str() + dataStr.length(), &this->root, &this->err);	
}
