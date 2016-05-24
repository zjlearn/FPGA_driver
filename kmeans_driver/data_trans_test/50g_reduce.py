logfile=open("log.txt");
fresult=open("50gresult.txt",'w');
for line in logfile:
    st=line.strip(" ");
    st=st[len(st)-2:]
    fresult.write(st)
logfile.close();
fresult.write("\n")
fresult.close();
    
    
