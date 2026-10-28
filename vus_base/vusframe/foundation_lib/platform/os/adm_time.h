#ifndef _ADM_TIME_H_
#define _ADM_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	ADM_CLOCK_REALTIME = 0,
	ADM_CLOCK_MONOTONIC
}ADM_TIME_CLK_TYPE;

typedef struct {
	D16U	year;
	D16U	month;
	D16U	day;
	D16U	hour;
	D16U	min;
	D16U	sec;
	D16U    msec;
} ADM_TIME_T;

extern D32S adm_time_get(ADM_TIME_T *time);


#ifdef __cplusplus
}
#endif


#endif /* _ADM_TIME_H_ */

