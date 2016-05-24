f= open("log.txt")
fw0=open("log_d0.txt",'w');
fw1=open("log_d1.txt",'w')
fw2=open("log_d2.txt",'w')
fw3=open("log_d3.txt",'w')
for line in f:
    if "ll header" in line:
        continue;
    elif "martian source"  in line:
        continue;
    elif "__ratelimit" in line:
        continue;
    elif "bayesfpga1" in line:
        fw0.write(line);
        fw1.write(line);
    elif "bayesfpga2"  in line:
        fw0.write(line)
        fw2.write(line);
    else:
        fw0.write(line);
        fw3.write(line);
        
f.close()
fw1.close();
        