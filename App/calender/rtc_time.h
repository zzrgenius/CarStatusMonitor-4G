/*
 * rtc.h
 *
 *  Created on: Feb 2, 2016
 *      Author: petera
 */

#ifndef _RTC__TIME_H_
#define _RTC__TIME_H_

#include "main.h"
#include "rtc.h"
#define YEAR_BASE    (1900)
#define DAYOFWEEK    (1)


typedef struct {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
 } rtc_time;

typedef struct {
  uint16_t year;
  uint8_t month;
  uint8_t day;
   uint8_t week_day;
} rtc_date;

typedef struct {
  rtc_date date;
  rtc_time time;
} rtc_datetime;

 typedef struct 
{
   uint16_t  year;
   uint8_t   month;
   uint8_t   day;
	
   uint8_t   hour;
   uint8_t   min;
   uint8_t   sec;                  
   uint8_t   wday;             
}DateTime;

/**
 * Date and time data
 * @see nmea_time_now
 */
typedef struct _nmeaTIME
{
    int     year;       /**< Years since 1900 */
    int     mon;        /**< Months since January - [0,11] */
    int     day;        /**< Day of the month - [1,31] */
    int     hour;       /**< Hours since midnight - [0,23] */
    int     min;        /**< Minutes after the hour - [0,59] */
    int     sec;        /**< Seconds after the minute - [0,59] */
    int     hsec;       /**< Hundredth part of second - [0,99] */

} nmeaTIME;
void TM_GetLocaltime(nmeaTIME*	local_time);
unsigned int  xDate2Seconds(nmeaTIME *time);
void TM_SetTime(uint8_t hour,uint8_t min ,uint8_t sec);
void TM_SetDate(uint16_t year,uint8_t mon ,uint8_t day);
 void UTC_to_BJtime(nmeaTIME*	utc_time, int8_t	timezone);
#endif /*   */
