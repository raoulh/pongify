#ifndef SCORE_H
#define SCORE_H

class Player;

struct ScoreRR
{
    Player *player = nullptr;
    int score = 0;
    int setWin = 0;
    int setLoose = 0;
    int winCount = 0;
};

#endif // SCORE_H
