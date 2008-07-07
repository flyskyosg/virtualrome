import shelve

num=1000
mydict=shelve.open('d:\\tmp_mydict',writeback=True)
for i in range(0,num):
    mydict[str(i)]=dict()
    for j in range(0,num):
        mydict[str(i)][j]=0.001 * float(i) * float(j)
    print "closing",i
mydict.close()
#output=open('d:\\tmp.pkl','wb')
#pickle.dump(mydict,output)
#output.close()

print "open dict"
d=shelve.open('d:\\tmp_mydict')
i=720
j=570
print "get key"
if(d.has_key(str(i))):
    print d[str(i)][j]