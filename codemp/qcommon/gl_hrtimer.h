#ifndef __GL_HRTIMER_H__
#define __GL_HRTIMER_H__
#ifndef MM_RELEASE

void HRT_Stop(int TimerID);
void HRT_Start(int TimerID);
double HRT_GetTimingMMS(int TimerID);
double HRT_GetTimingMS(int TimerID);
double HRT_GetTimingS(int TimerID);

#endif
#endif