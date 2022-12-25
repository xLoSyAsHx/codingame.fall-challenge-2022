#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <queue>

#include <numeric>
#include <algorithm>

//using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int g_mapWidth = 0, g_mapHeight = 0;

struct Pos {
    int x = 0, y = 0;
    bool isValid = false;

    Pos operator+(const Pos& p) { return Pos{x + p.x, y + p.y}; }
    bool operator==(const Pos& p) { return x == p.x && y == p.y; }
    friend std::ostream& operator<<(std::ostream& os, const Pos& p) { os << " { x = " << p.x << "; y = " << p.y << " } "; }

    // Some approximation
    int distanceTo(Pos p) { return abs(x - p.x) + abs(y - p.y); }
};

#define DBG_MSG_V(v) { std::cerr << "MESSAGE: "#v" = " << v << std::endl; }
#define DBG_MSG2_V(v1, v2) { std::cerr << "MESSAGE: v1 = " << v1 << "; v2 = " << v2 << std::endl; }
#define DBG_MSG3_V(v1, v2, v3) { std::cerr << "MESSAGE: v1 = " << v1 << "; v2 = " << v2 << "; " << v3 <<  std::endl; }
#define DBG_MSG_STR(s) { std::cerr << "MESSAGE: " << s << std::endl; }


struct MapCell {
    Pos p;

    int scrap_amount = 0;
    int owner = 0; // 1 = me, 0 = enemy, -1 = neutral
    int units = 0;
    int recycler = 0;

    // debug
    int can_build = 0;
    int can_spawn = 0;
    int in_range_of_recycler = 0;

    // calc constants
    int recycleProfit = -1;

    // Global ptr to map
    static std::vector<std::vector<MapCell>>& map;

    void FillFromConsole() {
        std::cin >> scrap_amount >> owner >> units >> recycler >> can_build >> can_spawn >> in_range_of_recycler; std::cin.ignore();
        recycleProfit = -1;
    }

    inline bool isMy()    const { return owner == 1; }
    inline bool isEnemy() const { return owner == 0; }

    Pos getTop()    { Pos nP {p.x, p.y - 1}; if (nP.y >= 0)          nP.isValid = true; return nP; }
    Pos getBottom() { Pos nP {p.x, p.y + 1}; if (nP.y < g_mapHeight) nP.isValid = true; return nP; }
    Pos getLeft()   { Pos nP {p.x - 1, p.y}; if (nP.x >= 0)          nP.isValid = true; return nP; }
    Pos getRight()  { Pos nP {p.x + 1, p.y}; if (nP.x < g_mapWidth)  nP.isValid = true; return nP; }

    std::vector<MapCell*> getCellNeighbors() {
        std::vector<MapCell*> res = {};

        if (Pos p = getTop();    p.isValid) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getBottom(); p.isValid) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getLeft();   p.isValid) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getRight();  p.isValid) res.emplace_back(&map[p.y][p.x]);

        return res;
    }

    int getRecyclerProfit()
    {
        if (recycleProfit == -1)
        {
            recycleProfit = scrap_amount;

            std::vector<MapCell*> n = getCellNeighbors();
            std::for_each(n.begin(), n.end(), [this](MapCell* c) { recycleProfit += c->scrap_amount; });
        }
        return recycleProfit;
    }
};

struct GameInfo
{
    int currentTurn = -1;

    int my_matter  = 0;
    int opp_matter = 0;

    std::vector<std::vector<MapCell>> map = {};

    std::vector<MapCell*> myCUnits = {};
    std::vector<MapCell*> enemyCUnits = {};
    int myCUnitsNum = 0;
    int enemyCUnitsNum = 0;

    void ReadMatters() { std::cin >> my_matter >> opp_matter; std::cin.ignore(); }
    void ReadMapDimention() { std::cin >> g_mapWidth >> g_mapHeight; std::cin.ignore(); }
    void ReadCellsInput() {
        static bool isFirstCall = true;

        myCUnits.clear();
        enemyCUnits.clear();
        int myCUnitsNum = 0;
        int enemyCUnitsNum = 0;

        if (isFirstCall)
        {
            map.resize(g_mapHeight);
            for (auto& row : map)
                row.resize(g_mapWidth);
        }

        for (int i = 0; i < g_mapHeight; i++) {
            for (int j = 0; j < g_mapWidth; j++) {
                auto pCell = &map[i][j];
                if (isFirstCall) {
                    pCell->p = { j, i };
                }

                pCell->p.isValid = false;
                pCell->FillFromConsole();
                if (pCell->scrap_amount) pCell->p.isValid = true;

                if (pCell->isMy()) {
                    myCUnits.emplace_back(pCell);
                    myCUnitsNum += pCell->units;
                }
                else if (pCell->isEnemy() && pCell->recycler == 0) {
                    enemyCUnits.emplace_back(pCell);
                    enemyCUnitsNum += pCell->units;
                }

                DBG_MSG3_V(map[i][j].p, map[i][j].units, map[i][j].recycler);
            }
        }

        isFirstCall = false;
        currentTurn++;
    }

    MapCell& getCell(Pos p)
    {
        if (p.isValid)
            return map[p.y][p.x];
        
        printf("ERROR line %d", __LINE__);
        return map[0][0];
    }

} GInf;

std::vector<std::vector<MapCell>>& MapCell::map = GInf.map;


struct Command {
    std::stringstream ss;

    void Clear()  { ss.clear(); }
    void Submit() { std::cout << ss.str() << std::endl; }

    void AddBuildRec(Pos p)                    { ss << "BUILD " << p.x << ' ' << p.y << ';'; }
    void AddMove(int amount, Pos from, Pos to) { ss << "MOVE "  << amount << ' ' << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << ';'; }
    void AddSpawn(int amount, Pos p)           { ss << "SPAWN " << amount << ' ' << p.x << ' ' << p.y << ';'; }
    void AddWait()                             { ss << "WAIT"; }
};

struct ProfitCell {
    int profit;
    MapCell* cell;
};
std::array<ProfitCell, 3> findBestRecycler()
{
    std::array<ProfitCell, 3> res = {};

    auto cmp = [](ProfitCell left, ProfitCell right) { return left.profit < right.profit; }; // Lower priority - first
    std::priority_queue<ProfitCell, std::vector<ProfitCell>, decltype(cmp)> res_queue(cmp);

    auto checkProfitUpdateResQueue = [&res_queue](MapCell* cell) {
        int cellProfit = cell->getRecyclerProfit();

        res_queue.push(ProfitCell{ cellProfit, cell });
        if (res_queue.size() > 3)
            res_queue.pop(); // remove el with the smallest priority
    };

    for (auto cUnit : GInf.myCUnits)
    {
        checkProfitUpdateResQueue(cUnit);
        for (auto neighbor : cUnit->getCellNeighbors()) {
            checkProfitUpdateResQueue(neighbor);
        }
    }

    auto resIt = res.rend();
    for (int i = 0; i < res_queue.size(); ++i)
    {
        ++resIt;
        *resIt = res_queue.top();
        res_queue.pop();
    }

    return res;
}

Pos getUnitsEstCenterPos(std::vector<MapCell*>& units)
{
    Pos res = units[0]->p;
    std::for_each(units.begin(), units.end(), [&res](MapCell* c){ res = res + c->p; });
    res.x /= (int)units.size() + 1;
    res.y /= (int)units.size() + 1;

    return res;
}

Pos getNearestUnitToPos(std::vector<MapCell*>& units, Pos startP)
{
    Pos resP = units[0]->p;
    int smallestDist = startP.distanceTo(resP);

    for (auto unit : units)
        if (int dist = startP.distanceTo(unit->p); dist < smallestDist)
        {
            smallestDist = dist;
            resP = unit->p;
        }

    return resP;
}

Pos getMyUnitNearestToEnemyPos()
{
    // Find enemy center and select friend unit which is closest to it
    Pos enemyCenterEstPos = getUnitsEstCenterPos(GInf.enemyCUnits);
    return getNearestUnitToPos(GInf.myCUnits, enemyCenterEstPos);
}

int main()
{
    Command command;
    
    MapCell* pMyCUnitsG1 = nullptr;

    // game loop
    GInf.ReadMapDimention();
    while (1) {
        GInf.ReadMatters();
        GInf.ReadCellsInput();
        command.Clear();

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        if (GInf.currentTurn == 0)
        {
            pMyCUnitsG1 = &GInf.getCell(getMyUnitNearestToEnemyPos());
        }
        Pos nearestEnemyPos = getNearestUnitToPos(GInf.enemyCUnits, pMyCUnitsG1->p);

        std::array<ProfitCell, 3> cellsForRecyclers = findBestRecycler();
        if (auto cell = cellsForRecyclers[0].cell; cell->can_build)
        {
            command.AddBuildRec(cell->p);
        }
        else
        {
            if (pMyCUnitsG1->p == cell->p)
            {
                command.AddMove(pMyCUnitsG1->units, pMyCUnitsG1->p, nearestEnemyPos);
            }
            command.AddBuildRec(cell->p);
        }

        command.Submit();
    }
}