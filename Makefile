all:
	g++ -std=c++17 aurtoo.cpp -o aurtoo

resetconf:
	rm -rfv /home/matt/.config/aurtoo

resethard:
	rm -rfv pkgs repo

reset: resetconf resethard

test: all resetconf resethard
	./aurtoo add nofetch-git
	./aurtoo remove nofetch-git