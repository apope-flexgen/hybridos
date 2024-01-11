%{
Voltage balancing worksheet
Has dependencies on the control and ltfat packages

run the following commands in octave command line:
pkg install -forge control
pkg load control
pkg install -forge ltfat          %This takes a long time!. Remove the noise() call if you don't want to wait for this
pkg load ltfat
%}


clc;
##clear all;
%close all;

%battery_playground;

tstart = 120000;
tend = 150000;


pulsestart = 5000; %samples
pulseadjust = pulsestart;
pulselength = 5000; %samples
pulseramp = 10; %samples
pulsemagnitude = 600; %Amps
sign = 1;
readjust = true;
##tstep = 0.1;
##tvecss = [1:tstep:10000];
##ivecss = zeros(length(tvecss),1);
tvecss = tvecinterp(1:(0.5*end));
ivecss = ivecfilt(1:(0.5*end));  %need to run csvreadout.m first!! Otherwise uncomment 30-33, 54-69. Comment33-35;
tstep = tstepinterp;

##for i=1:length(ivecss)
##    if i< (pulseramp + pulsestart) && i >=pulsestart
##      ivecss(i) = ivecss(i-1) + ((sign) * pulsemagnitude/pulseramp);
##    elseif i>=(pulsestart + pulselength - pulseramp) && i <(pulsestart + pulselength)
##      ivecss(i) = ivecss(i-1) - ((sign) * pulsemagnitude/pulseramp);
##    elseif (i>= pulsestart + pulselength) && (readjust)
##      pulsestart = pulsestart + i;
##      %pulselength = pulselength + i;
##      sign = sign * -1;
##      readjust = false;
##    else
##      ivecss(i) = sign * pulsemagnitude;
##      readjust = true;
##    endif
##endfor
ripple = sin(4.3*[1:tstep:10000]);

##ns = 100*noise(length(ivecss),1,'white');
##for i=1:length(ivecss)
##  if i > pulsestart && i < pulsestart+pulseramp
##    ivecss(i) = ivecss(i-1) + (sign * (pulsemagnitude/pulseramp));
##    readjust = true;
##  elseif (i >= (pulsestart + pulseramp)) && (i<= (pulsestart + pulselength - pulseramp))
##    ivecss(i) = sign*pulsemagnitude;
##  elseif (i > (pulsestart + pulselength - pulseramp)) && (i< (pulsestart + pulselength)-1);
##    ivecss(i) = ivecss(i-1) - (sign * (pulsemagnitude/pulseramp));
##  elseif (i >= (pulsestart + pulselength)) && readjust
##    pulsestart = pulseadjust + i;
##    sign = sign * -1;
##    readjust = false;
##  endif
##  ivecss(i) = 0.98*ivecss(i) + 0.01*ivecss(i)*(1+ripple(i))+ 0.01*ivecss(i)*ns(i);
##endfor



%ECM
V0 = 1300; %volts

R1b = .0062; % Ohms
R2b = 0.0061; % Ohms
C1b= .15;    % Farads

R1a = 0.0035;
R2a = 0.0029;
C1a = 0.15;

%SS equation. Yes there is a a way to combine e.g. Aa and Ab into one matrix. No I am not going to put the time into that
Aa = [(-1/(C1a))*R2a, 0; 0, 0];
Ba = [(1/C1a)*R2a ; 0];
Ca = [-R2a, 1];
Da = [-R1a-R2a];

Ab = [(-1/(C1b))*R2b, 0; 0, 0];
Bb = [1/C1b * R2b ; 0];
Cb = [-R2b, 1];
Db = [-R1b-R2b];


X0 = [0, V0];
x0 = 0;
xdot = 0;
v0dot = V0;
x = 0;
%%set up initial condition vectors, state vectors.
XaDOT = [xdot;v0dot];
XbDOT = XaDOT; %
Xa = [x; V0];
Xb = Xa;



#[t, y] = ode45 (@(t, y) -y, [0, 1], 1, opt);

#[t, vout] = ode45 (@(t,vout), [0, tvecss(

sysa = ss(Aa, Ba, Ca, Da);
sysb = ss(Ab, Bb, Cb, Db);
sysa = c2d(sysa, tstep);
sysb = c2d(sysb, tstep);
##[vouta, touta] = lsim(sysa, ivecss, tvecss, X);
##[voutb, toutb] = lsim(sysb, ivecss, tvecss, X);


%%updatePowerDelta calibrations

dVThresh = 50;      %Volts. Max allowable difference between DC busses
scaleFactor = 1;  %Unitless. How strongly algorithm will change output power by
deadbandV = 1.5;     %Volts. If dV < deadbandV do not change output power
timeout = [0, 2, 4, 8, 16];       %Minimum time between deltaV updates. TODO update faster if dV > threshold where threshold is close to dVThresh?
enable = true;      %updatePowerDelta strategy
capAmax = 3000;      %Ah. Max capacity of battery A
capBmax = 3000;      %Ah. Max capacity of battery B
SoCinitA = 0.95;     %[0 1]. Initial SoC condition for battery A
SoCinitB = 0.95;     %[0 1]. Initial SoC condition for battery B
availableAhA = zeros(length(ivecss),1);
availableAhB = zeros(length(ivecss),1);
SoCA = zeros(length(ivecss),1);
SoCB = zeros(length(ivecss),1);

availableAhA(1) = SoCinitA * capAmax;
availableAhB(1) = SoCinitB * capBmax;
SoCA(1) = SoCinitA;
SoCB(1) = SoCinitB;

stepCurrent = 0;
deltaivec = zeros(length(ivecss),1);
iThresh = 1;    %Amps. Threshold below which out will not be adjusted

%ivecmodified = ivecss; % keep track of Ia
##pratio = zeros(length(ivecss),1);
j = 1;
##for j = 1:length(timeout);
  out(:,j) = zeros(length(ivecss),1);
  vouta(:,j) = ones(length(ivecss),1);
  voutb(:,j) = ones(length(ivecss),1);
  ratio(:,j) = zeros(length(ivecss),1);
  adjustedA(:,j) = ones(length(ivecss),1);
  adjustedB(:,j) = ones(length(ivecss),1);
  vouta(1,j) = V0;
  voutb(1,j) = V0;
  XaDOT = [xdot;v0dot];
  XbDOT = XaDOT; %
  Xa = [x; V0];
  Xb = Xa;
  tlast = -timeout/tstep;

  for i = 2:length(ivecss)

    stepCurrent = ivecss(i)/2;
##    deltaivec(i) = ivecss(i) - ivecss(i-1);
    adjustedA(i,j) = stepCurrent + abs(stepCurrent) * ratio(i-1,j); %abs() needed here to preserve sign for charging case. sign for sum controlled by sign of ratio.
    adjustedB(i,j) = stepCurrent - abs(stepCurrent) * ratio(i-1,j);
    availableAhA(i) = availableAhA(i-1) - (adjustedA(i,j) * ((tvecss(i) - tvecss(i-1))/3600));
    availableAhB(i) = availableAhB(i-1) - (adjustedB(i,j) * ((tvecss(i) - tvecss(i-1))/3600));
    SoCA(i) = availableAhA(i)/capAmax;
    SoCB(i) = availableAhB(i)/capBmax;


    XaDOT = sysa.A * Xa + sysa.B * adjustedA(i,j);
    vouta(i,j) = sysa.C * Xa + sysa.D * adjustedA(i,j);
    XbDOT = sysb.A * Xb + sysb.B * adjustedB(i,j);
    voutb(i,j) = sysb.C * Xb + sysb.D * adjustedB(i,j);

    if enable == true
      if (abs(vouta(i,j) - voutb(i,j))>= deadbandV)&& (i - tlast >= timeout(j)/tstep) && abs(ivecss(i)) >= iThresh
          out(i,j) = scaleFactor * (vouta(i,j) - voutb(i,j))/dVThresh;
          ratio(i,j) = ratio(i-1,j) + out(i,j);
          if ratio(i,j) > 1
            ratio(i,j) = 1;
          elseif ratio(i,j) < -1
            ratio(i,j) = -1;
          endif
          tlast = i;
        else
          ratio(i,j) = ratio(i-1,j);
          out(i,j) = out(i-1,j);
      endif
    endif

    Xa = XaDOT;
    Xb = XbDOT;
  endfor
  figure("name", strcat(num2str(timeout(j)), " Second Timeout"))
  subplot(3,1,1)
  title(strcat("Timeout: ", num2str(timeout(j))))
  plot(tvecss,vouta(:,j));
  axis([0 length(tvecss)/(1/tstep) 1450 1530])
  hold on
  plot(tvecss, voutb(:,j));
  title("Voltage output")
  subplot(3,1,2)
  plot(tvecss, (vouta(:,j)-voutb(:,j)));
  title("Delta Voltage")
  subplot(3,1,3)
  plot(tvecss,ivecss);
  title("Battery current")
##endfor



##figure; subplot(3,1,1);
##plot(tvecss,adjustedA);
##subplot(3,1,2);
##plot(tvecss,adjustedB);
##subplot(3,1,3);
##plot(tvecss, ivecss);


##pcsVolDiff = 50;
##scaleFactor = 0.7;
##powerCmd = 100;  %kW
##
##Va = [50: 0.5: 150];
##Vb = [150: -0.5: 50];
##deltaV = Va - Vb;
##
##powerDiff = ones(1,length(deltaV));
##
##for i = 1:length(deltaV)
##  powerDiff(i) = (scaleFactor*(powerCmd * deltaV(i)))/pcsVolDiff;
##  if powerDiff(i) > powerCmd * scaleFactor
##    powerDiff(i) = powerCmd * scaleFactor;
##  elseif powerDiff(i) < -powerCmd * scaleFactor
##    powerDiff(i) = -powerCmd * scaleFactor;
##  endif
##endfor






