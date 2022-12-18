#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    int width;
    int height;
    cin >> width >> height; cin.ignore();

    // game loop
    while (1) {
        int my_matter;
        int opp_matter;
        cin >> my_matter >> opp_matter; cin.ignore();
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int scrap_amount;
                int owner; // 1 = me, 0 = foe, -1 = neutral
                int units;
                int recycler;
                int can_build;
                int can_spawn;
                int in_range_of_recycler;
                cin >> scrap_amount >> owner >> units >> recycler >> can_build >> can_spawn >> in_range_of_recycler; cin.ignore();
            }
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << "WAIT" << endl;
    }
}