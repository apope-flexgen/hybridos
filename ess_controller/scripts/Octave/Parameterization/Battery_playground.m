%%Data input and conditioning
%clc;
%clear all;
close all;
pkg load io;
pkg load signal;

ncells = 400; ## number of cells in series
plots = false;
highcur_up_slew = 5;  %A/s to determine start of high current event
pulse_wait_tm = 60;    %seconds to wait since start of high current pulse to take second voltage measurement
pulse_min_cur = 300;   %A. Current threshold to define a high current pulse.
const_cur_slew = 0.5;  %A/s. Current slew rate below which we consider current to be 'steady state.'
const_vlt_slew = 0.25; %V/s. Voltage change over time below which we consider voltage to be 'steady state'
low_cur = 30;           %A. Current threshold to define 'no current'

V = []; %Voltage array
I = []; %Current array
T = []; %Temperature array

[vfile, vpath, _] = uigetfile("*.csv", "Select voltage file");
[ifile, ipath, _] = uigetfile("*.csv", "Select current file");
%fullvfile = [vpath, vfile];

if !strcmp(pwd,vpath)
  addpath(vpath)
endif

if !strcmp(pwd, ipath)
  addpath(ipath)
endif


V = csv2cell(vfile);
I = csv2cell(ifile);
if length(V) != length(I)
  error("Voltage and current files must have same lengths");
endif

tvec = ones(1,length(V)-2);
vvec = ones(1,length(V)-2);
ivec = ones(1,length(I)-2);


starttime = V{2,1};
for i = 2:length(V)-1


  V{i,1} = V{i,1} - starttime;
  tvec(i-1) = V{i,1}/1000; %convert to seconds
  vvec(i-1) = V{i,2};
  ivec(i-1) = I{i,2};
endfor

if plots
  figure
  #subplot(2,1,1)
  plot(tvec,vvec);
  hold on
  #subplot(2,1,2)
  plot(tvec,ivec);
endif


%% Data analysis
[b,a] = butter(1, 0.01);
ivecfilt = filter(b,a,ivec);
islew = ones(1,length(ivecfilt));
islew(1) = 0;
vslew = ones(1,length(ivecfilt));
vscale = ones(1,length(vvec));
tstep = ones(1,length(tvec));
vslew(1) = 0;


for i = 2:length(ivecfilt)
  tstep(i) = tvec(i) - tvec(i-1);
  %tstep = tstep/1000;
  islew(i) = (ivecfilt(i) - ivecfilt(i-1))/tstep(i);
endfor

for i = 2:length(vvec)
 % tstep = tvec(i) - tvec(i-1);
  vslew(i) = (vvec(i) - vvec(i-1))/tstep(i);
endfor

for i = 1:length(vvec)
  vscale(i) = vvec(i) - 1000;

 endfor

if plots
  figure
  subplot(2,1,1, "align")
  plot(tvec, ivecfilt);
  hold on;
  plot(tvec, islew)
  hold off;
  subplot(2,1,2, "align")
  %plot(tvec, vscale);
  %hold on;
  plot(tvec,vslew);
endif

%%calculate resistance
resvec = ones(1,length(vvec));
resvec(1) = 0;
highcur_start = false;
volstart_flag = false;
restaken = false;
new_measure = false;
volstart = 0;
voltend = 0;
pulse_start_time = 0;
pulse_cur = 0.001;
for i = 2:length(vvec)

  if (islew(i) > highcur_up_slew || highcur_start)
    if(highcur_start == false)                           %%start of new current pulse
       volstart = vvec(i-1)                    %%todo: better way to grab starting voltage
       pulse_start_time = tvec(i-1)
       highcur_start = true
    endif

    if((tvec(i) - pulse_start_time) > pulse_wait_tm) %wait 30s, take new measurement TODO: figure out how to exclude current that has still recently changed
      if((ivecfilt(i) > pulse_min_cur) && (abs(islew(i)) < const_cur_slew) && (abs(vslew(i)) < const_vlt_slew) && !restaken)
        voltend = vvec(i);
        pulse_cur = ivecfilt(i);
        restaken = true;
        new_measure = true;
      endif
      if abs(ivecfilt(i)) < low_cur
        highcur_start = false;
        restaken = false;
      endif
    endif
  endif
  if new_measure
    resvec(i) = abs((volstart-voltend))/pulse_cur;
##    pulse_start_time
##    voltstart
##    voltend
##    pulse_cur
    new_measure = false;
  else
    resvec(i) = resvec(i-1);
  endif

  #resvec(i) = abs((volstart-voltend))/pulse_cur;
  if resvec(i) > 0.5
    resvec(i) = 0;
  endif
endfor





