/*******************************************************************************
  * @file	wdtctl.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date	30.05.2019
  *****************************************************************************/

#ifndef WDTCTL_H_
#define WDTCTL_H_

#ifndef WDTCTL_DEV_NAME
# define WDTCTL_DEV_NAME		"/dev/watchdog0"
#endif
#ifndef WDTCTL_PIPE_NAME
# define WDTCTL_PIPE_NAME		"/tmp/wdtctl_in"
#endif
#ifndef WDTCTL_STATUS_PIN
# define WDTCTL_STATUS_PIN		41
#endif



/*! ---------------------------------------------------------------------------
 * @fn: wdtctl_ping
 *
 * @brief: Пнуть wdt
 *
 * input parameters
 *
 * output parameters
 *
 * return
 * 		== 0 - успешно
 * 		!= 0 - провал
*/
extern int wdtctl_ping(void);



/*! ---------------------------------------------------------------------------
 * @fn: wdtctl_settiomeout
 *
 * @brief: Установка таймаута до сброса без пинга
 *
 * input parameters
 * @param timeout - время до сброса в c
 *
 * output parameters
 *
 * return
 * 		== 0 - успешно
 * 		!= 0 - провал
*/
extern int wdtctl_settiomeout(int timeout);

#endif /* WDTCTL_H_ */
