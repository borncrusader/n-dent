N=[1,2,4,8,16,32,64,128,256,512,1024]
t1=[700.812,240.156,282.1688,547.822,968.194,182.62,187.88,234.5,194.2,322.84,304.72]

r=[1,2,3,4,5]
t2=[27.292,115.634,252.186,521.478,951.36]

MSS=[100,200,300,400,500,600,700,800,900,1000]
t3=[835.45,306.044,239.94,212.142,252.186,201.78,142.26,99.12,91.14,80.9]

p=[0.01,0.02,0.03,0.04,0.05,0.06,0.07,0.08,0.09,0.10]
t4=[204.984,163.8,253.724,172.626,252.186,133.874,146.96,172.58,227.12,231.98]

plot(N,t1)
title("Task 1 : average delay versus window size")
xlabel("N")
ylabel("t")
legend("t (average delay) v N (window size)")

figure
plot(r,t2)
title("Task 2 : average delay versus number of receivers")
xlabel("r")
ylabel("t")
legend("t (average delay) v r (number of receivers)")

figure
plot(MSS,t3)
title("Task 3 : average delay versus MSS")
xlabel("MSS")
ylabel("t")
legend("t (average delay) v MSS (Maximum Segment Size)")

figure
plot(p,t4)
title("Task 4 : average delay versus probability of packet loss")
xlabel("p")
ylabel("t")
legend("t (average delay) v p (probability of packet loss)")
