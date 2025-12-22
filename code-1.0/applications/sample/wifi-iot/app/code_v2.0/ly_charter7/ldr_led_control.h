#ifndef __LDR_LED_CONTROL_H__
#define __LDR_LED_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

void LdrLed_MainLoop(void);
int LdrLed_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* __LDR_LED_CONTROL_H__ */
