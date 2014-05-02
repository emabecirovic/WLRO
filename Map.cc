
class Map_Draw
{
public:

    Map_Draw();
    ~Map_Draw();

private:
    int x_pos;
    int y_pos;
    float distance;
    int last_turn; // svängde senast 90 grader åt höger eller vänster 1 = höger , 2 = vänster , 0 är inte svängt än.

};
