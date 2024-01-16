
##has dependencies on io
##pkg install -forge io
##pkg load io


[file, fpath, _] = uigetfile("*.csv", "Select voltage file");
if !strcmp(pwd,fpath)
  addpath(fpath)
endif

D = csv2cell(file);
starttime = D{2,1};
for i = 2:length(D)-1
  D{i,1} = D{i,1} - starttime;
  tvec(i-1) = D{i,1}/1000; %convert to seconds
  ivec(i-1) = D{i,2};
endfor

tstepinterp = 1.5;
tvecinterp = [0:tstepinterp:tvec(end)];

ivecinterp = interp1(tvec,ivec,tvecinterp);
[b,a] = butter(1, 0.01);
ivecfilt = filter(b,a,ivecinterp);

##xf = [0:0.05:10];
##yf = sin (2*pi*xf/5);
##xp = [0:10];
##yp = sin (2*pi*xp/5);
##lin = interp1 (xp, yp, xf);
##near = interp1 (xp, yp, xf, "nearest");
##pch = interp1 (xp, yp, xf, "pchip");
##spl = interp1 (xp, yp, xf, "spline");
##plot (xf,yf,"r", xf,near,"g", xf,lin,"b", xf,pch,"c", xf,spl,"m",
##      xp,yp,"r*");
##legend ("original", "nearest", "linear", "pchip", "spline");
