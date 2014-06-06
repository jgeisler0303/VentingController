float massOfWater(float T, float RH) {
  const float P= 1013.25; // pressure hPa
  const float M_H2O= 18.01534; // molar mass of water g/mol
  const float R= 8.31447215; // Universal gas constant J/mol/K

  float P_H2O= RH/100*vaporPressure(T); // partial pressure of water hPa
  float x_H2O= P_H2O/P; // mass concentration of water -

  float n_air= (P*100)/(R*(T+273.15)); // air density mol/m^3
  float c_H2O= x_H2O*n_air*M_H2O; // mass concentration of water g/m^2

  // c_H2O_= 216.7*(RH/100*6.112*exp(17.62*T/(243.12+T))/(273.15+T));
  // float c_H2O_= 100/R*M_H2O*(RH/100*6.112*exp(17.62*T/(243.12+T))/(273.15+T));
  return(c_H2O);
}

float vaporPressure(float T) {
  const float a0= 6.107799961;
  const float a1= 4.436518521e-1;
  const float a2= 1.428945805e-2;
  const float a3= 2.650648471e-4;
  const float a4= 3.031240396e-6;
  const float a5= 2.034080948e-8;
  const float a6= 6.136820929e-11;

  return(a0+T*(a1+T*(a2+T*(a3+T*(a4+T*(a5+T*a6))))));
}

