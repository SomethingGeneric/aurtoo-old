all:
	g++ -std=c++17 aurtomatic.cpp -o aurtomatic

blank: all
	rm -rfv /home/matt/.config/aurtomatic
	./aurtomatic foo