#include"TimeClass.h"

using namespace std;


std::string TimeClass::TimeNow()
{
	string re="";
	re += to_string(1900 + this->t_tm.tm_year);
	re += "/" + to_string(this->t_tm.tm_mon);
	re += "/" + to_string(this->t_tm.tm_mday);
	re += "-" + to_string(this->t_tm.tm_hour);
	re += ":" + to_string(this->t_tm.tm_min);
	re += ":" + to_string(this->t_tm.tm_sec);
	return re;
}

bool TimeClass::init() {
	try
	{
		time(&(this->t));
#if defined __linux__
		localtime_r(&(this->t), &(this->t_tm));
#elif defined _WIN32||_WIN64
		localtime_s(&(this->t_tm), &(this->t));
#endif // _WIN
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

std::string TimeClass::timeFromTm(DateFormat dateformat) {
	std::string str = "";
	switch (dateformat)
	{
	case TimeClass::DateFormat::SEC:
		str = std::to_string(t_tm.tm_sec) + "秒";
		break;
	case TimeClass::DateFormat::MIN:
		str = std::to_string(t_tm.tm_min) + "分";
		break;
	case TimeClass::DateFormat::HOUR:
		str = std::to_string(t_tm.tm_hour) + "时";
		break;
	case TimeClass::DateFormat::DAY:
		str = std::to_string(t_tm.tm_mday) + "日";
		break;
	case TimeClass::DateFormat::MON:
		str = std::to_string(t_tm.tm_mon + 1) + "月";
		break;
	case TimeClass::DateFormat::YEAR:
		str = std::to_string(t_tm.tm_year + 1900) + "年";
		break;
	case TimeClass::DateFormat::WEEKDAY:
		switch (t_tm.tm_wday)
		{
		case 0:
			str = "星期一";
		case 1:
			str = "星期二";
		case 2:
			str = "星期三";
		case 3:
			str = "星期四";
		case 4:
			str = "星期五";
		case 5:
			str = "星期六";
		case 6:
			str = "星期日";
		default:
			break;
		}
		break;
	case TimeClass::DateFormat::YEARDAY:
		str = "今年的第" + std::to_string(t_tm.tm_yday + 1) + "天";
		break;
	default:
		break;
	}
	return str;
}