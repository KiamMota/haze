#ifndef ROUTE_H
#define ROUTE_H

#include "api/proto/Request.h"
#include "api/proto/Response.h"

typedef struct {
  const char* name;
  Response* (*handler)(Request*);
} Route;

const Route* GetGeneratedRoutes(void);

#endif
