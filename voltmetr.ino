float get24V()
{
  return (5.1 * getVCC() * analogRead(A3)) / 1023000;
}

float getB16Voltage()
{
  return (5.118 * getVCC() * analogRead(A4)) / 1023000;
}

float getB12Voltage()
{
  return (5.125 * getVCC() * analogRead(A5)) / 1023000;
}

bool hasLine()
{
  return get24V() > 15;
}

bool isB12NeedsCharge()
{

  return getB12Voltage() < 12.8;
}

bool isB12Full()
{
  return getB12Voltage() > 13.3;
}

bool isB12Normal()
{
  return getB12Voltage() > 12.3;
}

bool isB12Available()
{
  return getB12Voltage() > 10.9;
}

bool isB16NeedsCharge()
{
  return getB16Voltage() < 16;
}

bool isB16Normal()
{
  return getB16Voltage() > 13.6;
}

bool isB16Full()
{
  return getB16Voltage() > 16.35;
}

bool isB16Available()
{
  return getB16Voltage() > 12.4;
}
