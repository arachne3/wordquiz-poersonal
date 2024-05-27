#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

typedef enum {
    C_ZERO,
    C_LIST,
    C_SHOW,
    C_TEST,
    C_EXIT,
} command_t;

typedef struct {
    char *word;
    char *meaning;
} WordEntry;

void shuffle(WordEntry *array, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            WordEntry temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
}

char *strndup(const char *s, size_t n) {
    char *result;
    size_t len = strlen(s);
    if (n < len)
        len = n;
    result = (char *)malloc(len + 1);
    if (!result)
        return 0;
    result[len] = '\0';
    return (char *)memcpy(result, s, len);
}

char *read_a_line(FILE *fp) {
    static char buf[BUFSIZ];
    static int buf_n = 0;
    static int curr = 0;

    if (feof(fp) && curr == buf_n - 1)
        return 0x0;

    char *s = 0x0;
    size_t s_len = 0;
    do {
        int end = curr;
        while (!(end >= buf_n || !iscntrl(buf[end]))) {
            end++;
        }
        if (curr < end && s != 0x0) {
            curr = end;
            break;
        }
        curr = end;
        while (!(end >= buf_n || iscntrl(buf[end]))) {
            end++;
        }
        if (curr < end) {
            if (s == 0x0) {
                s = strndup(buf + curr, end - curr);
                s_len = end - curr;
            } else {
                s = realloc(s, s_len + end - curr + 1);
                s = strncat(s, buf + curr, end - curr);
                s_len = s_len + end - curr;
            }
        }
        if (end < buf_n) {
            curr = end + 1;
            break;
        }

        buf_n = fread(buf, 1, sizeof(buf), fp);
        curr = 0;
    } while (buf_n > 0);
    return s;
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_menu() {
    printf("1. List all wordbooks\n");
    printf("2. Show the words in a wordbook\n");
    printf("3. Test with a wordbook\n");
    printf("4. Exit\n");
}

int get_command() {
    int cmd;

    printf(">");
    scanf("%d", &cmd);
    clear_input_buffer();
    return cmd;
}

void list_wordbooks() {
    DIR *d = opendir("wordbooks");

    printf("\n  ----\n");

    struct dirent *wb;
    while ((wb = readdir(d)) != NULL) {
        if (strcmp(wb->d_name, ".") != 0 && strcmp(wb->d_name, "..") != 0) {
            printf("  %s\n", wb->d_name);
        }
    }
    closedir(d);

    printf("  ----\n");
}

void show_words() {
    char wordbook[128];
    char filepath[256];

    list_wordbooks();

    printf("Type in the name of the wordbook?\n");
    printf(">");
    scanf("%s", wordbook);
    clear_input_buffer();

    sprintf(filepath, "wordbooks/%s", wordbook);

    FILE *fp = fopen(filepath, "r");

    if (fp == NULL) {
        printf("Error: Wordbook '%s' does not exist.\n", wordbook);
        return;
    }

    printf("\n  -----\n");
    char *line;
    while ((line = read_a_line(fp))) {
        char *word = strtok(line, "\"");
        strtok(NULL, "\"");
        char *meaning = strtok(NULL, "\"");
        printf("Word: %s - Meaning: %s\n", word, meaning);
        free(line);
    }
    fclose(fp);
}

void run_test() {
    char wordbook[128];
    char filepath[256];

    printf("Type in the name of the wordbook?\n");
    printf(">");
    scanf("%s", wordbook);
    clear_input_buffer();

    sprintf(filepath, "wordbooks/%s", wordbook);

    FILE *fp = fopen(filepath, "r");

    if (fp == NULL) {
        printf("Error: Wordbook '%s' does not exist.\n", wordbook);
        return;
    }

    WordEntry *entries = NULL;
    size_t entry_count = 0;

    printf("\n-----\n");

    char *line;
    while ((line = read_a_line(fp))) {
        entries = realloc(entries, (entry_count + 1) * sizeof(WordEntry));
        entries[entry_count].word = strtok(line, "\"");
        strtok(NULL, "\"");
        entries[entry_count].meaning = strtok(NULL, "\"");
        entry_count++;
    }
    fclose(fp);

    srand(time(NULL));
    shuffle(entries, entry_count);

    int n_questions = 0;
    int n_correct = 0;

    for (size_t i = 0; i < entry_count; i++) {
        printf("Q. %s\n", entries[i].meaning);
        printf("?  ");

        char answer[128];
        scanf("%s", answer);
        clear_input_buffer();

        if (strcmp(answer, entries[i].word) == 0) {
            printf("- correct\n");
            n_correct++;
        } else {
            printf("- wrong: %s\n", entries[i].word);
        }

        n_questions++;
    }

    printf("(%d/%d)\n", n_correct, n_questions);

    printf("-----\n\n");

    for (size_t i = 0; i < entry_count; i++) {
        free(entries[i].word);
        free(entries[i].meaning);
    }
    free(entries);
}

int main() {
    printf(" *** Word Quiz *** \n\n");

    int cmd;
    do {
        print_menu();

        cmd = get_command();
        switch (cmd) {
            case C_LIST: {
                list_wordbooks();
                break;
            }

            case C_SHOW: {
                show_words();
                break;
            }

            case C_TEST: {
                run_test();
                break;
            }

            case C_EXIT: {
                return EXIT_SUCCESS;
            }
        }
    } while (cmd != C_EXIT);

    return EXIT_SUCCESS;
}
