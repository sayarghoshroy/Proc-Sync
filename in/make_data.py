import random
f = open("input.txt", "w+")

f.write("10000\n")
n = 10000
i = 0
while i < n:
	num = random.randint(-10000, 10000);
	f.write(str(num))
	f.write("\n")
	i = i + 1

f.close()