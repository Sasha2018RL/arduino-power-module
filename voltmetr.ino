float getU(){
  return (5.11 * getVCC() * analogRead(A1)) / 1023000;
}
