#include"Sqlite3Operate.h"

std::string Sqlite3Operate::GBKtoUTF8(std::string gbkStr) {
	iconv_t iconvH = iconv_open("UTF-8", "GB2312");
	const char* strChar = gbkStr.c_str();
	const char** pin = &strChar;
	size_t strLength = gbkStr.length();
	char* outbuf = (char*)malloc(strLength * 4);
	char* pBuff = outbuf;
	memset(outbuf, 0, strLength * 4);
	size_t outLength = strLength * 4;
	if (-1 == iconv(iconvH, (const char**)pin, &strLength, &outbuf, &outLength)) {
		free(pBuff);
		iconv_close(iconvH);
		return "´íÎó";
	}
	gbkStr = pBuff;
	iconv_close(iconvH);

	return gbkStr;
}

Sqlite3Operate::Sqlite3Operate(const char* dbPath) {
	this->dbPath = dbPath;
	sqlite3_open(dbPath, &(this->dataBasePtr));
}

Sqlite3Operate::~Sqlite3Operate() {
	sqlite3_close(this->dataBasePtr);
	//printf("dbÎö¹¹");
}

bool Sqlite3Operate::addTable(const char* name) {
	std::string name_U=GBKtoUTF8(name);
	std::string command = "";
	command += "create table if not exists ";
	command += name_U.c_str();
	command +="(id integer";
	command += ")";
	char* ErrMsg;
	int kRes = sqlite3_exec(this->dataBasePtr, command.c_str(), 0, 0, &ErrMsg);
	if (kRes != SQLITE_OK)return false;
	return true;
}

std::vector<std::string>* Sqlite3Operate::getAllTableName() {

	auto tableNames = new std::vector<std::string>();
	const char* sqlStr = "select name from sqlite_master where type='table' order by name";
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(this->dataBasePtr, sqlStr, -1, &stmt, NULL);
	if (SQLITE_OK == result) {
		while (SQLITE_ROW == sqlite3_step(stmt))
		{
			const unsigned char* name = sqlite3_column_text(stmt, 0);
			tableNames->push_back((const char*)name);
		}
	}
	else {
		printf("²éÑ¯ËùÓĞ±íÃû´íÎó");
	}
	sqlite3_finalize(stmt);
	return tableNames;
}

bool Sqlite3Operate::dropTable(const char* name) {
	std::string sqlStr = "drop table ";
	sqlStr += name;
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(this->dataBasePtr, sqlStr.c_str(), -1, &stmt, NULL);
	if (SQLITE_OK == result) {
		sqlite3_step(stmt);
	}
	else {
		printf("É¾±íÓï¾ä´íÎó");
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

bool Sqlite3Operate::addColumn(const char* table,const char* name)
{
	return false;
}

std::string* Sqlite3Operate::findOneData(const std::string& tablename, const std::string& targetname, const std::string& scrname, const std::string& scr)
{
	auto re = new std::string("");
	std::string sqlStr = "select " + targetname + " from " + tablename + " where(" + scrname + "='" + scr + "')";
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(this->dataBasePtr, sqlStr.c_str(), -1, &stmt, NULL);
	if (SQLITE_OK == result) {
		while (SQLITE_ROW == sqlite3_step(stmt))
		{
			const unsigned char* name = sqlite3_column_text(stmt, 0);
			re->append((const char*)name);
		}
	}
	else {
		printf("²éÑ¯´íÎó£ºfunction:findOneData\n");
	}
	sqlite3_finalize(stmt);
	return re;
}

std::string Sqlite3Operate::getOneData(const std::string& sqlStr)
{
	std::string re("");
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(this->dataBasePtr, sqlStr.c_str(), -1, &stmt, NULL);
	if (SQLITE_OK == result) {
		while (SQLITE_ROW == sqlite3_step(stmt))
		{
			const unsigned char* name = sqlite3_column_text(stmt, 0);
			re.append((const char*)name);
		}
	}
	else {
		printf("²éÑ¯´íÎó£ºfunction:getOneData\n");
	}
	sqlite3_finalize(stmt);
	return re;
}

bool Sqlite3Operate::doOneSqlStr(const std::string& sqlStr)
{
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(this->dataBasePtr, GBKtoUTF8(sqlStr).c_str(), -1, &stmt, NULL);
	if (SQLITE_OK == result) {
		sqlite3_step(stmt);
	}
	else {
		printf("Ö´ĞĞÓï¾ä´íÎó£ºfunction:doOneSqlStr");
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

bool Sqlite3Operate::insertNew(const std::string& tablename, const std::string& columns, const std::string& values)
{
	std::string sqlStr = "insert into ";
	sqlStr += tablename;
	sqlStr.append(" (");
	sqlStr += columns;
	sqlStr.append(") values (");
	sqlStr += values;
	sqlStr.append(")");
	sqlite3_stmt* stmt = nullptr;

	int result = sqlite3_prepare_v2(this->dataBasePtr, GBKtoUTF8(sqlStr).c_str(), -1, &stmt, NULL);
	if (SQLITE_OK == result) {
		sqlite3_step(stmt);
	}
	else {
		printf("²åÈëÓï¾ä´íÎó£ºfunction:insertNew");
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}
