all:
	gcc -o ioulive64 main.c utils.c ioulive.c iou-raw.c parse.c
clean:
	rm -rf ioulive64
