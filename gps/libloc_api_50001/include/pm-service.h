#ifndef __PM_SERVICE_H__
#define __PM_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PM_RET_SUCCESS      0
#define PM_RET_FAILED       -1
#define PM_RET_UNSUPPORTED  1

enum pm_event {
    EVENT_PERIPH_GOING_OFFLINE,
    EVENT_PERIPH_IS_OFFLINE,
    EVENT_PERIPH_GOING_ONLINE,
    EVENT_PERIPH_IS_ONLINE,
};

typedef void (*pm_client_notifier) (void *, enum pm_event);

int pm_client_connect(void *clientId);

int pm_client_disconnect(void *clientId);

int pm_client_register(pm_client_notifier notifier, void *clientData,
                       const char *devName, const char *clientName,
                       enum pm_event *state, void **handle);

int pm_client_event_acknowledge(void *clientId, enum pm_event event);

#ifdef __cplusplus
}
#endif
#endif
