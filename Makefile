all:
	g++ -std=c++17 aurtomatic.cpp -o aurtomatic

reset: all
	rm -rfv /home/matt/.config/aurtomatic
	./aurtomatic add nofetch-git