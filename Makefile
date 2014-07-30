default: blackjack

blackjack.o: blackjack.c
	gcc -c blackjack.c -o blackjack.o

blackjack: blackjack.o
	gcc blackjack.o -o blackjack

clean:
	-rm -f blackjack.o
	-rm -f blackjack
