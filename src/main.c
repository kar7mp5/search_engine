#include <stdio.h>
#include <string.h>


int webcrawling() {
	char Site[100] = {"https://movie.naver.com/movie/running/current.nhn"};
	char Command[100] = {"wget -O index.html "};
	char YesOrNo;
	
	while (1) {
		system("clear");
		printf("\n%s\nDo you want to start web crawling?(y / n) : ", Site);
		scanf("%c", &YesOrNo);
		__fpurge(stdin);
		if (YesOrNo == 'n' || YesOrNo == 'N') {
			printf("End web crawling....\n");
			return 0;
		}
		else if (YesOrNo == 'Y' || YesOrNo == 'y') {
			strcat(Command, Site);
			system(Command);
			printf("\nWeb crawling success!\n");
			break;
		}
		else {
			printf("You entered an incorrect character!\n");
			sleep(1);
		}
	}
	return 1;
}

int webSelecting() {
	FILE *fp = fopen("index.html", "r");
	if (fp == NULL) {
		printf("index.html is not EXIST!\n");
		return 0;
	}
	else {
		char *buff = malloc(sizeof(char) * 1024);
		int count = 1;
		
		while (fgets(buff, 100, fp)) {
			if (strstr(buff, "<li  data-title=") != 0) {
				for (int i = 0; i < 100 - 25; i++) {
					buff[i] = buff[i+25];
					if (buff[i] == '\"') {
						buff[i] = '\0';
						break;
					}
				}
				printf("%d.\n%s\n",count++, buff);
			}
		}
		free(buff);
		return 1;
	}
}


void main() {
	if (webcrawling()) {
        webSelecting();
    }
	system("rm index.html");
}