#include "drv_ultrauart.h"
#include "../obk_config.h"

#if ENABLE_DRIVER_ULTRAUART

#include "../new_common.h"
#include "../new_pins.h"
#include "../logging/logging.h"
#include "drv_uart.h"

#define ULTRA_BAUD        9600
#define ULTRA_HEADER      0x18
#define ULTRA_TERMINATOR  0x0D
#define ULTRA_LEN_CMD     8
#define ULTRA_LEN_RESP    14
#define ULTRA_CHANNEL     1

static const byte ultra_cmd[ULTRA_LEN_CMD] = {
	0x18, 0x08, 0x6A, 0xFF, 0xFF, 0x27, 0xCE, 0x0D
};

static int ultra_lastRaw = -1;
static int ultra_frames = 0;

void UltraUART_Init(void) {
	UART_InitUART(ULTRA_BAUD, 0, false);
	UART_InitReceiveRingBuffer(256);
	ultra_lastRaw = -1;
	ultra_frames = 0;
	addLogAdv(LOG_INFO, LOG_FEATURE_DRV, "UltraUART: init");
}


void UltraUART_RunEverySecond(void) {
	for (int i = 0; i < ULTRA_LEN_CMD; i++) {
		UART_SendByte(ultra_cmd[i]);
	}
}

void UltraUART_RunQuickTick(void) {
	while (1) {
		int size = UART_GetDataSize();
		if (size < 2) {
			return;
		}
		if (UART_GetByte(0) != ULTRA_HEADER) {
			UART_ConsumeBytes(1);
			continue;
		}
		int len = UART_GetByte(1);
		if (len != ULTRA_LEN_CMD && len != ULTRA_LEN_RESP) {
			UART_ConsumeBytes(1);
			continue;
		}
		if (size < len) {
			return;
		}
		if (UART_GetByte(len - 1) != ULTRA_TERMINATOR) {
			UART_ConsumeBytes(1);
			continue;
		}
		if (len == ULTRA_LEN_RESP) {
			int raw = UART_GetByte(5) | (UART_GetByte(6) << 8);
			ultra_frames++;
			if (raw != 0xFFFF) {
				ultra_lastRaw = raw;
				CHANNEL_Set(ULTRA_CHANNEL, raw, 0);
			}
			addLogAdv(LOG_INFO, LOG_FEATURE_DRV, "UltraUART: raw=%i", raw);
		}
		UART_ConsumeBytes(len);
	}
}

void UltraUART_AppendInformationToHTTPIndexPage(http_request_t *request, int bPreState) {
	if (bPreState) {
		return;
	}
	hprintf255(request, "<h5>UltraUART raw=%i (frames=%i)</h5>", ultra_lastRaw, ultra_frames);
}

#endif