#include "main.h"
#include "clarity_bootstrap.h"

// Bootstrap instance
ClarityBootstrap bootstrap;

void setup()
{
  bootstrap.initialize();
}

void loop()
{
  bootstrap.run();
}

