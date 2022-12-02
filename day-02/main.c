#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common.c"

void panic(const char *str) {
    printf("%s\n", str);
    exit(1);
}

enum {
    ReadOpponent,
    ReadPlayer,
};

enum {
    Rock,
    Paper,
    Scissor,
};

enum {
    Win, 
    Loose,
    Draw,
};

int parse_play(char play) {
    if (play == 'A' || play == 'X') {
        return Rock;
    } else if (play == 'B' || play == 'Y') {
        return Paper;
    } else if (play == 'C' || play == 'Z') {
        return Scissor;
    }

    panic("Invalid play!");

    return -1;
}

int parse_strategy(char strat) {
    if (strat == 'X') {
        return Loose;
    } else if (strat == 'Y') {
        return Draw;
    } else if (strat == 'Z') {
        return Win;
    }

    panic("Invalid strategy!");

    return -1;
}

int decode_play(int opponent_play, int strategy) {
    switch (strategy) {
        case Win:
            switch (opponent_play) {
                case Rock:
                    return Paper;
                    break;

                case Paper:
                    return Scissor;
                    break;

                case Scissor:
                    return Rock;
                    break;
            }
            break;

        case Draw:
            return opponent_play;
            break;

        case Loose:
            switch (opponent_play) {
                case Rock:
                    return Scissor;
                    break;

                case Paper:
                    return Rock;
                    break;

                case Scissor:
                    return Paper;
                    break;
            }
            break;

        default:
            break;

    }

    panic("unreachable");

    return -1;
}

int score(int opponent_play, int your_play) {
    int op = opponent_play;
    int y = your_play;
    int outcome = 0;
    int shape_score = 0;

    switch (y) {
        case Rock:
            shape_score= 1;
            if (op == Rock) {
                outcome = 3;
            } else if (op == Scissor) {
                outcome = 6;
            }
            break;

        case Paper:
            shape_score = 2;
            if (op == Paper) {
                outcome = 3;
            } else if (op == Rock) {
                outcome = 6;
            }
            break;

        case Scissor:
            shape_score = 3;
            if (op == Scissor) {
                outcome = 3;
            } else if (op == Paper) {
                outcome = 6;
            }
            break;

        default:
            panic("unreachable");
            break;
    }

    return shape_score + outcome;
}


int score_for_line(char *line) {
    return score(parse_play(line[0]), parse_play(line[2]));
}

int score_part_two(char opponent, char strategy) {
    int op_play = parse_play(opponent);
    int strategy_code = parse_strategy(strategy);
    int my_play = decode_play(op_play, strategy_code);

    return score(op_play, my_play);
}

int score_for_line_part_two(char *line) {
    return score_part_two(line[0], line[2]);
}

int main() {
    char **lines = c_read_file_lines("input");

    char *line;
    int i = 0;
    int total_score = 0;
    int total_score_part_two = 0;
    while ((line = lines[i++])) {
        total_score += score_for_line(line);
        total_score_part_two += score_for_line_part_two(line);
    } 

    printf("total score: %d\n", total_score);
    printf("total score part two: %d\n", total_score_part_two);

    free(lines);
}
