all:
	g++ -std=c++17 aurtomatic.cpp -o aurtomatic

resetconf:
	rm -rfv /home/matt/.config/aurtomatic

resethard:
	rm -rfv pkgs repo

reset: resetconf resethard

test: all resetconf resethard
	./aurtomatic add nofetch-git
	./aurtomatic remove nofetch-git