#pragma once
#include<string>
#include<cstring>
#include<cstdarg>
#include<ctime>
#include<cstdlib>
#include<memory>

class TimeClass {
public:
	enum class DateFormat
	{
		SEC,
		MIN,
		HOUR,
		DAY,
		MON,
		YEAR,
		WEEKDAY,
		YEARDAY,
	};
public:
	TimeClass() { if (!(this->init())) { this->t = 0; memset(&(this->t_tm), 0, sizeof(this->t_tm)); }; }
	/**
	*按输入参数的格式输出指定的时间字符串
	*/
	template<typename ...Arg>
	std::string Time_Now_str(DateFormat dataFormat, Arg...arg) {
		return this->getTime(dataFormat, arg...);
	}

	/**
	*返回时间字符串，格式为xxxx/xx/xx-xx:xx:xx
	*/
	std::string TimeNow();
	/**
	*返回当前时间的tm结构体
	*/
	tm Time_Now_tm() { return this->t_tm; }

	/**
	返回当前时间的time_t值（从1900年后到现在的秒数）
	*/
	time_t Time_Now_time_t() { return this->t; }

private:
	bool init();

	template<typename ...Arg>
	std::string getTime(TimeClass::DateFormat dateformat, Arg...arg) {
		return this->timeFromTm(dateformat) + this->getTime(arg...);
	}

	std::string getTime() {
		return "";
	}

	std::string timeFromTm(DateFormat dateformat);
private:
	time_t t;
	tm t_tm;
};