cd $HOME/cs350-os161/os161-1.99
./configure --ostree=$HOME/cs350-os161/root --toolprefix=cs350-
cd kern/conf
./config ASST1
cd ../compile/ASST1
bmake depend
bmake
bmake install
cd $HOME/cs350-os161/os161-1.99
bmake
bmake install
cd $HOME/cs350-os161/root
cp /u/cs350/sys161/sys161.conf sys161.conf
sys161 kernel-ASST1


cd $HOME/cs350-os161
/u/cs350/bin/cs350_submit 0 | tee submitlog.txt

ssh -Y s529chen@linux.student.cs.uwaterloo.ca
