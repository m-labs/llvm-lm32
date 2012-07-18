
for i in `ls *.ll`
do
echo $i
mv $i $i.old
sed s/-march=monarch/-march=lm32/ < $i.old > $i
done
