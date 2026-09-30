#pragma once

#include "../httpserver/new_http.h"

void UltraUART_Init(void);
void UltraUART_RunEverySecond(void);
void UltraUART_RunQuickTick(void);
void UltraUART_AppendInformationToHTTPIndexPage(http_request_t *request, int bPreState);