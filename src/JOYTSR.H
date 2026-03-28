#ifndef _JOYTSR_H_
#define _JOYTSR_H_

#ifdef __cplusplus
extern "C"
{
#endif

void JOY_FindTSR(int verbosity, int factor);
uint8_t JOY_Port_Acc(uint16_t port, uint8_t val, uint16_t flags);

#ifdef __cplusplus
}
#endif

#endif//_JOYTSR_H_
