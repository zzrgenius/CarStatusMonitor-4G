#include "rtc_time.h" 
 #include <stdio.h>
 #include <stdbool.h>

#define SECSPERMIN      (60)
#define MINSPERHOUR     (60)
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define HOURSPERDAY     (24)
#define SECSPERDAY      (SECSPERMIN * MINSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK     (7)
#define MONETHSPERYEAR  (12)
#define DAYSPERYEAR     (365)
#define DAYSPERLEAPYEAR (366)

/* Days per month, LY is special */
static uint8_t daysPerMonth[2][MONETHSPERYEAR] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	/* Normal year */
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	/* Leap year */
};
const uint8_t table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表   

int8_t ReturnWeekDay(  uint16_t iYear, uint8_t iMonth, uint8_t iDay )  
{  
    int8_t iWeek = 0;  
    unsigned int y = 0, c = 0, m = 0, d = 0;  
  
    if ( iMonth == 1 || iMonth == 2 )  
    {  
        c = ( iYear - 1 ) / 100;  
        y = ( iYear - 1 ) % 100;  
        m = iMonth + 12;  
        d = iDay;  
    }  
    else  
    {  
        c = iYear / 100;  
        y = iYear % 100;  
        m = iMonth;  
        d = iDay;  
    }  
      
    iWeek = y + y / 4 + c / 4 - 2 * c + 26 * ( m + 1 ) / 10 + d - 1;    //蔡勒公式  
    iWeek = iWeek >= 0 ? ( iWeek % 7 ) : ( iWeek % 7 + 7 );    //iWeek为负时取模  
    if ( iWeek == 0 )    //星期日不作为一周的第一天  
    {  
        iWeek = 7;  
    }  
  
    return iWeek;  
} 
/* Converts the number of days offset from the start year to real year
   data accounting for leap years */
static void GetDMLY(int dayOff,   DateTime *pTime)
{
	bool YearFound = false;
	int daysYear = dayOff;
	int leapYear, curLeapYear, year = YEAR_BASE, monthYear = 0;
	bool MonthFound = false;

	/* Leap year check for less than 1 year time */
	if ((year % 4) == 0) {
		curLeapYear = 1;
	}
	else {
		curLeapYear = 0;
	}

	/* Determine offset of years from days offset */
	while (YearFound == false) {
		if ((year % 4) == 0) {
			/* Leap year, 366 days */
			daysYear = DAYSPERLEAPYEAR;
			leapYear = 1;
		}
		else {
			/* Leap year, 365 days */
			daysYear = DAYSPERYEAR;
			leapYear = 0;
		}

		if (dayOff > daysYear) {
			dayOff -= daysYear;
			year++;
		}
		else {
			YearFound = true;
			curLeapYear = leapYear;	/* In a leap year */
		}
	}

	/* Save relative year and day into year */
	pTime->year = year - YEAR_BASE;	/* Base year relative */
 
	/* Determine offset of months from days offset */
	while (MonthFound == false) {
		if ((dayOff + 1) > daysPerMonth[curLeapYear][monthYear]) {
			/* Next month */
			dayOff -= daysPerMonth[curLeapYear][monthYear];
			monthYear++;
		}
		else {
			/* Month found */
			MonthFound = true;
		}
	}

	pTime->day = dayOff + 1;/* 1 relative */
	pTime->month = monthYear;	/* 0 relative */
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

 
/* Converts an RTC tick time to Universal time */
void ConvertRtcTime(uint32_t rtcTick,   DateTime *pTime)
{
	int daySeconds, dayNum;

	/* Get day offset and seconds since start */
	dayNum = (int) (rtcTick / SECSPERDAY);
	daySeconds = (int) (rtcTick % SECSPERDAY);

	/* Fill in secs, min, hours */
	pTime->sec = daySeconds % 60;
	pTime->hour = daySeconds / SECSPERHOUR;
	pTime->min = (daySeconds - (pTime->hour * SECSPERHOUR)) / SECSPERMIN;

	/* Weekday, 0 = Sunday, 6 = Saturday */
	pTime->wday = (dayNum + DAYOFWEEK) % DAYSPERWEEK;

	/* Get year, month, day of month, and day of year */
	GetDMLY(dayNum, pTime);

 
}

/* Converts a Universal time to RTC tick time */
void ConvertTimeRtc(DateTime *pTime, uint32_t *rtcTick)
{
	int leapYear, year = pTime->year + YEAR_BASE;
	uint32_t dayOff, monthOff, monthCur, rtcTicks = 0;

	/* Leap year check for less than 1 year time */
	if ((year % 4) == 0) {
		leapYear = 1;
	}
	else {
		leapYear = 0;
	}

	/* Add days for each year and leap year */
	while (year > YEAR_BASE) {
		if ((year % 4) == 0) {
			/* Leap year, 366 days */
			rtcTicks += DAYSPERLEAPYEAR * SECSPERDAY;
			leapYear = 1;
		}
		else {
			/* Leap year, 365 days */
			rtcTicks += DAYSPERYEAR * SECSPERDAY;
			leapYear = 0;
		}

		year--;
	}

	/* Day and month are 0 relative offsets since day and month
	   start at 1 */
	dayOff = (uint32_t) pTime->day - 1;
	monthOff = (uint32_t) pTime->month;

	/* Add in seconds for passed months */
	for (monthCur = 0; monthCur < monthOff; monthCur++) {
		rtcTicks += (uint32_t) (daysPerMonth[leapYear][monthCur] * SECSPERDAY);
	}

	/* Add in seconds for day offset into the current month */
	rtcTicks += (dayOff * SECSPERDAY);

	/* Add in seconds for hours, minutes, and seconds */
	rtcTicks += (uint32_t) (pTime->hour * SECSPERHOUR);
	rtcTicks += (uint32_t) (pTime->min * SECSPERMIN);
	rtcTicks += (uint32_t) pTime->sec;

	*rtcTick = rtcTicks;
}

//输出:该年份是不是闰年.1,是.0,不是

uint8_t Is_Leap_Year(uint16_t year)
{                     
   if(year%4==0) //必须能被4整除
   { 
		  if(year%100==0) 
		  { 
				 if(year%400==0)return 1;//如果以00结尾,还要能被400整除          
				 else return 0;   
		  }else return 1;   
   }else return 0; 
} 


void TM_SetTime(uint8_t hour,uint8_t min ,uint8_t sec)
{
	RTC_TimeTypeDef sTime;
	if( (hour < 24) && (min < 60) )
		if(sec < 60)
		{
			sTime.Hours = hour;
			sTime.Minutes = min;
			sTime.Seconds = sec;
			sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
			sTime.StoreOperation = RTC_STOREOPERATION_RESET;
			if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
			{
				_Error_Handler(__FILE__, __LINE__);
			}
		}
			
		
		
}
void TM_SetDate(uint16_t year,uint8_t mon ,uint8_t day)
{
	RTC_DateTypeDef sDate;
	if( (mon < 13) &&(day < 32))
	{
		sDate.WeekDay = ReturnWeekDay(year,  mon ,  day);
		sDate.Month = mon;
		sDate.Date = day;
		sDate.Year = year-2000;
		if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
		{
			__nop();

		}
	}
		
}
void TM_GetLocaltime(nmeaTIME*	local_time)
{
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;
	
	HAL_RTC_GetDate (&hrtc,&sDate,RTC_FORMAT_BIN);
	HAL_RTC_GetTime (&hrtc,&sTime,RTC_FORMAT_BIN);
	local_time->year = sDate.Year + 2000;
	local_time->mon = 	sDate.Month;
	local_time->day = 		sDate.Date ;
	local_time->hour =	sTime.Hours;
	local_time->min = 	sTime.Minutes;
	local_time->sec = sTime.Seconds;
	
 
}

 
 
void UTC_to_BJtime(nmeaTIME*	utc_time, int8_t	timezone)
{
	int year,month,day,hour;
    int lastday = 0;					//last day of this month
    int lastlastday = 0;			//last day of last month

    year	 = utc_time->year;			 //utc time
    month  = utc_time->mon;
    day 	 = utc_time->day;
    hour 	 = utc_time->hour + timezone; 
	
    if(month==1 || month==3 || month==5 || month==7 || month==8 || month==10 || month==12){
        lastday = 31;
        if(month == 3){
            if((year%400 == 0)||(year%4 == 0 && year%100 != 0))				//if this is lunar year
                lastlastday = 29;
            else
                lastlastday = 28;
        }
        if(month == 8)
            lastlastday = 31;
    }
    else if(month == 4 || month == 6 || month == 9 || month == 11){
        lastday = 30;
        lastlastday = 31;
    }
    else{
        lastlastday = 31;
        if((year%400 == 0)||(year%4 == 0 && year%100 != 0))
            lastday = 29;
        else
            lastday = 28;
    }

    if(hour >= 24){					// if >24, day+1
            hour -= 24;
            day += 1; 

            if(day > lastday){ 		// next month,  day-lastday of this month
                day -= lastday;
                month += 1;

                if(month > 12){		//	next year , month-12
                    month -= 12;
                    year += 1;
                }
            }
        }
    if(hour < 0){										// if <0, day-1
            hour += 24;
            day -= 1; 
            if(day < 1){					  // month-1, day=last day of last month
                day = lastlastday;
                month -= 1;
                if(month < 1){ 			// last year , month=12
                    month = 12;
                    year -= 1;
                }
            }
        }
//   // transfer value to NMEA_result.local_time
//	NMEA_result.local_time.year  = year;
//	NMEA_result.local_time.month = month;
//	NMEA_result.local_time.date  = day;
//	NMEA_result.local_time.hour  = hour;
//	NMEA_result.local_time.min	 = utc_time->min;
//	NMEA_result.local_time.sec	 = utc_time->sec;
}
 
 
 
#define xMINUTE    (60             ) //1分的秒数
#define xHOUR      (60*xMINUTE) //1小时的秒数
#define xDAY        (24*xHOUR   ) //1天的秒数
#define xYEAR       (365*xDAY   ) //1年的秒数

//将localtime（UTC+8北京时间）转为UNIX TIME，以1970年1月1日为起点
unsigned int  xDate2Seconds(nmeaTIME *time)
{
	  const unsigned int  month[12]={
    /*01月*/xDAY*(0),
    /*02月*/xDAY*(31),
    /*03月*/xDAY*(31+28),
    /*04月*/xDAY*(31+28+31),
    /*05月*/xDAY*(31+28+31+30),
    /*06月*/xDAY*(31+28+31+30+31),
    /*07月*/xDAY*(31+28+31+30+31+30),
    /*08月*/xDAY*(31+28+31+30+31+30+31),
    /*09月*/xDAY*(31+28+31+30+31+30+31+31),
    /*10月*/xDAY*(31+28+31+30+31+30+31+31+30),
    /*11月*/xDAY*(31+28+31+30+31+30+31+31+30+31),
    /*12月*/xDAY*(31+28+31+30+31+30+31+31+30+31+30)
  };
 
  unsigned int  seconds = 0;
  unsigned int  year = 0;
  year = time->year-1970;       //不考虑2100年千年虫问题
  seconds = xYEAR*year + xDAY*((year+1)/4);  //前几年过去的秒数
  seconds += month[time->mon-1];      //加上今年本月过去的秒数
  if( (time->mon > 2) && (((year+2)%4)==0) )//2008年为闰年
    seconds += xDAY;            //闰年加1天秒数
  seconds += xDAY*(time->day-1);         //加上本天过去的秒数
  seconds += xHOUR*time->hour;           //加上本小时过去的秒数
  seconds += xMINUTE*time->min;       //加上本分钟过去的秒数
  seconds += time->sec;               //加上当前秒数<br>　seconds -= 8 * xHOUR;
  return seconds;
}
 
//将UNIX时间转为UTC+8 即北京时间
//UNIX转为UTC 已进行时区转换 北京时间UTC+8
void xSeconds2Date(unsigned long seconds,nmeaTIME *time )
{
  
    unsigned int days; 
    unsigned short leap_y_count; 
    time->sec      = seconds % 60;//获得秒 
    seconds          /= 60; 
    time->min      =  seconds % 60;//获得分 
    seconds          += 8 * 60 ;        //时区矫正 转为UTC+8 bylzs
    seconds          /= 60; 
    time->hour        = seconds % 24;//获得时 
    days              = seconds / 24;//获得总天数 
    leap_y_count = (days + 365) / 1461;//过去了多少个闰年(4年一闰) 
    if( ((days + 366) % 1461) == 0) 
    {//闰年的最后1天 
        time->year = 1970 + (days / 366);//获得年 
        time->mon = 12;              //调整月 
        time->day = 31; 
        return; 
    } 
    days -= leap_y_count; 
    time->year = 1970 + (days / 365);     //获得年 
    days %= 365;                       //今年的第几天 
    days = 01 + days;                  //1日开始 
    if( (time->year % 4) == 0 ) 
    { 
        if(days > 60)--days;            //闰年调整 
        else 
        { 
            if(days == 60) 
            { 
                time->mon = 2; 
                time->day = 29; 
                return; 
            } 
        } 
    } 
    for(time->mon = 0;daysPerMonth[0][time->mon] < days;time->mon++) 
    { 
        days -= daysPerMonth[0][time->mon]; 
    } 
    ++time->mon;               //调整月 
    time->day = days;           //获得日 
}
#if 0
 //UTC（字符串）转UNIX时间
/*******************************************************************************
* Function Name  : ConvertTimeToSecond
* Description    : Convert GPS Date to Log buffer.
* Input          : @date: format 'DDMMYY,HHMMSS.SSS'
* Output         : None
* Return         : Sencod
*******************************************************************************/
static uint32_t ConvertDateToSecond(const uint8_t *date)
{
	uint32_t sencods = 0;
	uint16_t temp = 1970;
	uint16_t days = 0;
	if(NULL == date) {
		return 0;
	}
	//year
	temp = (date[4] - 0x30) * 10 + (date[5] - 0x30) + 2000;
	if(0 == (temp % 4)) {
		days += 1;
	}
	temp -= 1;
	//UTC time start 1970
	for(; temp >= 1970; temp--) {
		if(temp % 4) {
			days += 365;
		} else {
			//leap year
			days += 366;
		}
	}
	//month
	temp = (date[2] - 0x30) * 10 + (date[3] - 0x30);
	temp -= 1;
	for(; temp >= 1; temp--) {
		days += daysPerMonth[0][temp];
	}
	//day
	temp = (date[0] - 0x30) * 10 + (date[1] - 0x30);
	days += temp - 1;
	//hour
	temp = (date[7] - 0x30) * 10 + (date[8] - 0x30);
	sencods += temp * SECSPERHOUR;
	//min
	temp = (date[9] - 0x30) * 10 + (date[10] - 0x30);
	sencods += temp * 60;
	//sencod
	temp = (date[11] - 0x30) * 10 + (date[12] - 0x30);
	sencods += temp;

	sencods += days * SECSPERDAY;

	return sencods;
}
#endif