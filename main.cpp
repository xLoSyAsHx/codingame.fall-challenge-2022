#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <queue>

#include <random>
#include <numeric>
#include <algorithm>

//using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int g_mapWidth = 0, g_mapHeight = 0;

struct Pos {
    int x = -1, y = -1;
    bool isValid = false;

    Pos operator+(const Pos& p) { return Pos{x + p.x, y + p.y}; }
    Pos operator-(const Pos& p) { return Pos{x - p.x, y - p.y}; }
    bool operator==(const Pos& p) { return x == p.x && y == p.y; }
    bool operator!=(const Pos& p) { return x != p.x || y != p.y; }
    bool operator<(const Pos& p) { return x < p.x || (x == p.x && y < p.y); } // X coordinate has higher priority
    friend std::ostream& operator<<(std::ostream& os, const Pos& p) { os << " { x = " << p.x << "; y = " << p.y << "; isValid == " << p.isValid << " } "; }

    // Some approximation
    int distanceTo(Pos p) { return abs(x - p.x) + abs(y - p.y); }

    void invalidate() { x = y = -1; isValid = false; };
};

#define DBG_MSG_V(msg, v) { std::cerr << msg << " " << v << std::endl; }
#define DBG_MSG_V2(msg, v1, v2) { std::cerr << msg << " " << v1 << " " << v2 << std::endl; }

#define DBG_V(v) { std::cerr << ""#v" = " << v << std::endl; }
#define DBG_V2(v1, v2) { std::cerr << "v1 = " << v1 << "; v2 = " << v2 << std::endl; }
#define DBG_V3(v1, v2, v3) { std::cerr << "v1 = " << v1 << "; v2 = " << v2 << "; " << v3 <<  std::endl; }

#define DBG_MSG_ARR_V(v) { std::cerr << ""#v" = { "; for (auto& el : v) std::cerr << el << "; "; std::cerr << std::endl; }
#define DBG_MSG_ARR_OF_PTR_V(v) { std::cerr << ""#v" = { "; for (auto el : v) std::cerr << *el << "; "; std::cerr << std::endl; }

#define DBG_MSG_STR(s) { std::cerr << s << std::endl; }

#define DBG_MSG_PTR(v) { std::cerr << ""#v" = " << *v << std::endl; }

#define DBG_MAP_CELLS(v) { std::cerr << ""#v" = ["; for (auto& el : v) std::cerr << el->p; std::cerr << "]" << std::endl; }
#define DBG_MAP_POSES(v) { std::cerr << ""#v" = ["; for (auto& el : v) std::cerr << el;    std::cerr << "]" << std::endl; }

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

    inline bool isMy()        const { return owner ==  1; }
    inline bool isEnemy()     const { return owner ==  0; }
    inline bool isEnemyUnit() const { return isEnemy() && units > 0; }
    inline bool isNobodys()   const { return owner == -1; }
    inline bool hasUnitsRec() const { return units || recycler; }


    inline bool isValid()   const { return p.isValid && scrap_amount; }

    Pos getTop()    { Pos nP {p.x, p.y - 1}; if (nP.y >= 0)          nP.isValid = true; return nP; }
    Pos getBottom() { Pos nP {p.x, p.y + 1}; if (nP.y < g_mapHeight) nP.isValid = true; return nP; }
    Pos getLeft()   { Pos nP {p.x - 1, p.y}; if (nP.x >= 0)          nP.isValid = true; return nP; }
    Pos getRight()  { Pos nP {p.x + 1, p.y}; if (nP.x < g_mapWidth)  nP.isValid = true; return nP; }

    std::vector<MapCell*> getCellNeighbours() {
        std::vector<MapCell*> res = {};

        if (Pos p = getTop();    p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getBottom(); p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getLeft();   p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(&map[p.y][p.x]);
        if (Pos p = getRight();  p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(&map[p.y][p.x]);

        return res;
    }

    std::vector<Pos> getCellNeighboursPos() {
        std::vector<Pos> res = {};

        if (Pos p = getTop();    p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(p);
        if (Pos p = getBottom(); p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(p);
        if (Pos p = getLeft();   p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(p);
        if (Pos p = getRight();  p.isValid && map[p.y][p.x].scrap_amount) res.emplace_back(p);

        return res;
    }

    int getRecyclerProfit()
    {
        if (recycleProfit == -1)
        {
            std::vector<MapCell*> n = getCellNeighbours();
            recycleProfit = std::accumulate(n.begin(), n.end(), scrap_amount, [](int acc, MapCell* rhd) { return acc + rhd->scrap_amount; });
        }
        return recycleProfit;
    }
};

struct ProfitCell {
    int profit;
    MapCell* cell;
};

struct GameInfo
{
    std::mt19937 random_device{std::random_device{}()};

    int currentTurn = -1;

    int my_matter  = 0;
    int opp_matter = 0;

    Pos prev_spawn {-1, -1};

    std::vector<std::vector<MapCell>> map = {};

    std::vector<MapCell*> myCells = {};
    std::vector<MapCell*> myCUnits = {};
    std::vector<MapCell*> enemyCUnits = {};
    int myCUnitsNum = 0;
    int enemyCUnitsNum = 0;

    int myRecycleNum = 0;
    int mySpawnNum = 0;
    int myMovedNum = 0;

    void ReadMatters() { std::cin >> my_matter >> opp_matter; std::cin.ignore(); }
    void ReadMapDimention() { std::cin >> g_mapWidth >> g_mapHeight; std::cin.ignore(); }
    void ReadCellsInput() {
        static bool isFirstCall = true;

        myCells.clear();
        myCUnits.clear();
        enemyCUnits.clear();
        myCUnitsNum = 0;
        enemyCUnitsNum = 0;
        myRecycleNum = 0;
        mySpawnNum = 0;
        myMovedNum = 0;
        

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
                if (pCell->scrap_amount)
                    pCell->p.isValid = true;

                if (pCell->isMy())
                {
                    myCells.emplace_back(pCell);

                    if (pCell->units)
                    {
                        myCUnits.emplace_back(pCell);
                        myCUnitsNum += pCell->units;
                    }
                    if (pCell->recycler)
                    {
                        myRecycleNum += 1;
                    }
                }
                else if (pCell->isEnemy() && pCell->units)
                {
                    enemyCUnits.emplace_back(pCell);
                    enemyCUnitsNum += pCell->units;
                }
            }
        }

        validate();

        isFirstCall = false;
        currentTurn++;
    }

    MapCell& getCell(Pos p)
    {
        if (p.isValid)
            return map[p.y][p.x];

        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR GetCell!" << std::endl;
        return map[0][0];
    }

    bool isValid(Pos p)
    {
        if (p.x >= 0 && p.x < g_mapWidth && p.y >= 0 && p.y < g_mapHeight)
        {
            return getCell(p).isValid();
        }

        return false;
    }

    bool isMyValid(Pos p)
    {
        if (p.x >= 0 && p.x < g_mapWidth && p.y >= 0 && p.y < g_mapHeight)
        {
            const auto& cell = getCell(p);
            return cell.isValid() && cell.isMy();
        }

        return false;
    }

    void validate()
    {
        // TODO: Use wave-algorithm with depth 7-15. If my unit inside separated area - need to fully expance it

        // Remove enemy if we can't achieve it
        std::vector<MapCell*> toRemove = {};
        for (auto cUnit : enemyCUnits)
        {
            bool bRemove = true;
            for (auto neighbour : cUnit->getCellNeighbours())
                if (!neighbour->recycler)
                {
                    bRemove = false;
                    break;
                }

            if (bRemove)
            {
                toRemove.emplace_back(cUnit);
            }
        }

        for (auto el : toRemove)
        {
            DBG_MSG_STR("TO DELETE:");
            DBG_V(el);
            enemyCUnits.erase(std::find(enemyCUnits.begin(), enemyCUnits.end(), el));
        }

        // Remove my units if they can't move.
        // TODO: Need to safe this cells to not spawn new units into it
        std::vector<MapCell*> myToRemove = {};
        for (auto cell : myCells)
        {
            if (cell->getCellNeighbours().empty())
            {
                myToRemove.emplace_back(cell);
            }
        }
        
        for (auto el : myToRemove)
        {
            DBG_MSG_STR("TO DELETE MY:");
            DBG_V(el);
            myCells.erase(std::find(myCells.begin(), myCells.end(), el));
        }

        DBG_MSG_STR("-------- Validation part finished--------");
    }

} GInf;

std::vector<std::vector<MapCell>>& MapCell::map = GInf.map;


class Command {
    std::stringstream ss;
    std::stringstream errSS;
    bool bEmpty = true;

public:
    bool empty() const { return bEmpty; }

    void Clear()  { ss.str(""); errSS.str(""); bEmpty = true; }
    void Submit() { std::cerr << errSS.str()<< std::endl; std::cout << ss.str() << std::endl; }

    void AddBuildRec(Pos p)                              { bEmpty = false; ss << "BUILD " << p.x << ' ' << p.y << ';'; GInf.my_matter -= 10; GInf.getCell(p).recycler = 1; }
    void AddMove(int amount, Pos from, Pos to, int line) { bEmpty = false; ss << "MOVE "  << amount << ' ' << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << ';'; GInf.myMovedNum += amount;
                                                                        errSS << "MOVE "  << amount << ' ' << from.x << ' ' << from.y << ' ' << to.x << ' ' << to.y << " l=" << line << "; ";}
    void AddSpawn(int amount, Pos p, int line)           { bEmpty = false; ss << "SPAWN " << amount << ' ' << p.x << ' ' << p.y << ';';                                     GInf.mySpawnNum += 1; GInf.my_matter -= amount * 10;
                                                                        errSS << "SPAWN " << amount << ' ' << p.x << ' ' << p.y << " l=" << line << "; "; }
    void AddWait()                                       { bEmpty = false; ss << "WAIT"; }
};


bool willCellTurnToGrass(MapCell* cell)
{
    int currScrapAmount = cell->scrap_amount;
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->recycler)
            --currScrapAmount;
    }

    return currScrapAmount <= 0;
}

bool hasEnemyNeighbours(MapCell* cell)
{
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->isEnemyUnit())
            return true;
    }

    return false;
}

int getEnemyNeighboursCount(MapCell* cell)
{
    // Wave depth == 2

    int numEnemies = 0;
    for (auto neighbour : cell->getCellNeighbours())
    {
        if (neighbour->isEnemyUnit())
            numEnemies += neighbour->units;

        for (auto neighbour2 : neighbour->getCellNeighbours())
        {
            if (neighbour2->p != neighbour->p && neighbour2->isEnemyUnit())
                numEnemies += neighbour2->units;
        }
    }

    return numEnemies;
}

// TODO: Do not use this dummy function
Pos getUnitsEstCenterPos(std::vector<MapCell*>& units)
{
    if (units.empty())
        return {};

    Pos res {0, 0};
    std::for_each(units.begin(), units.end(), [&res](MapCell* c){ res = res + c->p; });

    res.x /= (int)units.size();
    res.y /= (int)units.size();

    return res;
}

Pos getNearestUnitToPos(std::vector<MapCell*>& units, Pos startP)
{
    if (units.empty())
        return {};

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

ProfitCell findBestRecycler()
{
    // Find all unique poses where player can build a recycler
    std::vector<Pos> uniquePoses = {};
    int curMaxScrap = 0;
    for (auto cell : GInf.myCells)
    {
        if (!cell->hasUnitsRec())
            uniquePoses.emplace_back(cell->p);
    }

    for (auto cUnit : GInf.myCUnits)
    {
        int neighboursNum = cUnit->getCellNeighboursPos().size();

        if (neighboursNum == 1)
        {
            if (auto it = std::find(uniquePoses.begin(), uniquePoses.end(), cUnit->getCellNeighboursPos()[0]); it != uniquePoses.end())
                uniquePoses.erase(it);
        }
    }

    // Sort elements by profit amount
    std::sort(uniquePoses.begin(), uniquePoses.end(), [](Pos& lhd, Pos& rhd) {
        return GInf.getCell(lhd).getRecyclerProfit() > GInf.getCell(rhd).getRecyclerProfit();
    });

    if (uniquePoses.empty())
        return {};

    auto pCell = &GInf.getCell(uniquePoses.front());
    return { pCell->getRecyclerProfit(), pCell };
}

struct SpawnRecomendation
{
    Pos p;
    int units;
};
SpawnRecomendation findBestForSpawn()
{
    /*
     * Best for spawn search steps in priority order
     *
     * 1) If there are enemies near my units in 2-cells radius - spawn as much as possible to protect (if will not turn to grass)
     * 2) Find normal cell which will not turn into grass
     * 3) Find my unit nearest to enemy and spawn here
    */

   // ----- 1 -----
    for (auto pBestForSpawnSell : GInf.myCUnits)
    {
        if (int numEnemies = getEnemyNeighboursCount(pBestForSpawnSell); numEnemies && !willCellTurnToGrass(pBestForSpawnSell))
        {
            DBG_V(pBestForSpawnSell->p);

            int toSpawn = numEnemies - pBestForSpawnSell->units;
            /*
            if (toSpawn < 0)
            {
                toSpawn = 1;
            }
            else
            */
            {
                toSpawn = std::min(GInf.my_matter / 10, getEnemyNeighboursCount(pBestForSpawnSell));
            }

            return { pBestForSpawnSell->p, toSpawn };
        }
    }

    // ----- 2 -----
    if (GInf.myCUnitsNum < 1.3*double(GInf.enemyCUnitsNum))
    {
        for (auto pBestForSpawnSell : GInf.myCells)
        {
            int neighboursNum = pBestForSpawnSell->getCellNeighbours().size();
            if (neighboursNum > 3 && !pBestForSpawnSell->can_spawn && pBestForSpawnSell->p != GInf.prev_spawn && !willCellTurnToGrass(pBestForSpawnSell))
            {
                DBG_V(pBestForSpawnSell->p);
                GInf.prev_spawn = pBestForSpawnSell->p;
                return { pBestForSpawnSell->p, 1 };
            }
        }
    }

    // ----- 3 -----
    /*
    Pos p = getMyUnitNearestToEnemyPos();
    if (p.isValid)
    {
        return { p, 1 };
    }
    */

    return {};
}

Pos findBestForResearchMove(MapCell* myCell)
{
    // TODO: Need to try to move away ffrom enemies
    // Example: if my{ 5, 0 }, nearest_enemy{ 3, 0 }, need to move Right or Bottom

    std::array<Pos, 4> candidates  = {
        myCell->getLeft(),
        myCell->getTop(),
        myCell->getBottom(),
        myCell->getRight(),
    };

    for (auto candidatePos : candidates)
    {
        if (candidatePos.isValid)
        {
            auto& candidateCell = GInf.getCell(candidatePos);
            if (candidateCell.isValid() && !candidateCell.isMy() && !candidateCell.recycler)
                return candidateCell.p;
        }
    }

    return {};
}

int getMaxToMove(MapCell* cell)
{
    // TODO: maybe need to refine

    // If algorithm spawn units or moved them - we need to take it into account
    return std::min(cell->units, GInf.myCUnitsNum - GInf.mySpawnNum - GInf.myMovedNum);
}

Pos getNearestNooneCellPosToMove(MapCell* curCell)
{
    // TODO: use wave-algorithm with depth 5-7

    // Depth 1
    for (auto cell : curCell->getCellNeighbours())
    {
        if (cell->isNobodys() || (cell->isEnemy() && cell->recycler == 0))
            return cell->p;
    }

    // Depth 2
    for (auto cell : curCell->getCellNeighbours())
    {
        for (auto cell2 : cell->getCellNeighbours())
        {
            if (cell2->isNobodys() || (cell2->isEnemy() && cell2->recycler == 0))
                return cell2->p;
        }
    }

    return {};
}

/*
struct PosRoute {
    Pos p;
    std::vector<Pos>
};
*/
struct Poses_DestFirst {
    Pos pDest;
    Pos firstToMove;
};
Poses_DestFirst getNearest_Enemy_CellPosToMove(MapCell* curCell)
{
    Poses_DestFirst res{};
    // TODO: use wave-algorithm with depth 5-7

    // Depth 1
    for (auto cell : curCell->getCellNeighbours())
    {
        if (cell->isEnemy() && cell->recycler == 0)
        {
            res.firstToMove = res.pDest = cell->p;
            return res;
        }
    }

    // Depth 2
    for (auto cell : curCell->getCellNeighbours())
    {
        res.firstToMove = cell->p;
        for (auto cell2 : cell->getCellNeighbours())
        {
            if (cell2->isEnemy() && cell2->recycler == 0)
            {
                res.pDest = cell2->p;
                return res;
            }
        }
    }

    return {};
}
Poses_DestFirst getNearest_Noone_CellPosToMove(MapCell* curCell)
{
    Poses_DestFirst res{};
    // TODO: use wave-algorithm with depth 5-7

    // Depth 1
    for (auto cell : curCell->getCellNeighbours())
    {
        if (cell->isNobodys())
        {
            res.firstToMove = res.pDest = cell->p;
            return res;
        }
    }

    // Depth 2
    for (auto cell : curCell->getCellNeighbours())
    {
        res.firstToMove = cell->p;
        for (auto cell2 : cell->getCellNeighbours())
        {
            if (cell2->isNobodys())
            {
                res.pDest = cell2->p;
                return res;
            }
        }
    }

    return {};
}

Poses_DestFirst getNearest_EnemyOrNoone_CellPosToMove(MapCell* curCell)
{
    auto res = getNearest_Enemy_CellPosToMove(curCell);
    if (!res.pDest.isValid)
    {
        res = getNearest_Noone_CellPosToMove(curCell);
    }

    return res;
}

void getRandomCells(std::vector<MapCell*> &v, std::vector<MapCell*> &out, size_t nelems)
{
    std::sample(v.begin(), v.end(), std::back_inserter(out),
        nelems,
        GInf.random_device);
}

int main()
{    
    Command command;

    Pos researcherPos{};
    Pos invaderPos{};

    // game loop
    GInf.ReadMapDimention();
    while (1) {
        GInf.ReadMatters();
        GInf.ReadCellsInput();
        command.Clear();

        if (!GInf.isMyValid(researcherPos)) researcherPos.invalidate();
        if (!GInf.isMyValid(invaderPos))    invaderPos.invalidate();

        if (researcherPos.isValid && hasEnemyNeighbours(&GInf.getCell(researcherPos)))
        {
            DBG_MSG_V("WARN - Researcher in dangerous - behave like normal unit", researcherPos);
            researcherPos.invalidate();
        }

        // TODO: If invader is important - do not remove it
        if (invaderPos.isValid && hasEnemyNeighbours(&GInf.getCell(invaderPos)))
        {
            DBG_MSG_V("WARN - Invider in dangerous - behave like normal unit", invaderPos);
            invaderPos.invalidate();
        }

        bool is_spawn = false;

        DBG_V(GInf.currentTurn);
        DBG_MAP_CELLS(GInf.myCUnits);

        // If number of units too small - spend all matters to spawn units
        // TODO: Maybe need to spawn near the enemy
        if (GInf.myCUnitsNum < 2)
        {
            std::vector<MapCell*> randSpawn;
            getRandomCells(GInf.myCells, randSpawn, 10);

            for (auto cell : randSpawn)
            {
                if (cell->can_spawn)
                {
                    command.AddSpawn(GInf.my_matter / 10, cell->p, __LINE__);
                    command.Submit();
                    continue;
                }
            }
        }
        

        // TODO: Add logic to select smallest and far away from enemy unit as Researcher


        // 
        DBG_MSG_STR("-------- Recycler part start --------");
        if (GInf.myRecycleNum < 1 && GInf.my_matter >= 10)
        {
            ProfitCell cellForRecycler = findBestRecycler();
            if (cellForRecycler.cell == nullptr)
            {
                DBG_MSG_STR("cellForRecycler == nullptr");
            }
            else if (GInf.my_matter < 30)
            {
                auto cell_to_build = cellForRecycler.cell;
                DBG_V(cell_to_build->p);
                if (cell_to_build->can_build)
                {
                    command.AddBuildRec(cell_to_build->p);
                    GInf.my_matter -= 10;
                }
            }
        }

        // Spawn part
        DBG_MSG_STR("-------- Spawn part start --------");
        {
            if (GInf.my_matter >= 20)
            {
                auto spawnRec = findBestForSpawn();
                DBG_V(spawnRec.p);
                if (spawnRec.p.isValid)
                {
                    command.AddSpawn(spawnRec.units, spawnRec.p, __LINE__);
                    is_spawn = true;
                }
            }
        }

        // Global movement
        DBG_MSG_STR("-------- Global movement start --------");
        // 1) Check if recycler may destroy units. If so - move them to safe cell
        // 2) Move all units to nearest enemy

        // ============================================= Global movement 1 =============================================
        for (int i = 0; i < GInf.myCUnits.size(); i++)
        {
            auto myCell = GInf.myCUnits[i];
            if (willCellTurnToGrass(myCell))
            {
                // 1 - Find nearest enemy pos to current unit
                Pos nearestEnemyPos = getNearestUnitToPos(GInf.enemyCUnits, myCell->p);

                // 2 - Try to move to enemy direction
                Pos enemyDirection = nearestEnemyPos - myCell->p;
                Pos posToMove{};
                if (enemyDirection.x > 0)      { posToMove = myCell->getRight(); }
                else if (enemyDirection.x < 0) { posToMove = myCell->getLeft(); }
                else if (enemyDirection.y > 0) { posToMove = myCell->getTop(); }
                else if (enemyDirection.y < 0) { posToMove = myCell->getBottom(); }
                
                auto cellToMove = &GInf.getCell(posToMove);
                if (posToMove.isValid && cellToMove->scrap_amount && !willCellTurnToGrass(cellToMove))
                {
                    auto numUnitsToMove = getMaxToMove(myCell);
                    if (numUnitsToMove > 0)
                    {
                        command.AddMove(numUnitsToMove, myCell->p, cellToMove->p, __LINE__);

                        // Units already moved and can not do anything else. Lets just remove them
                        GInf.myCUnits.erase(GInf.myCUnits.begin() + i);
                        continue;
                    }
                }

                // 3 - Can't move to 'cellToMove' try to move somewhere
                for (auto neighbour : myCell->getCellNeighbours())
                {
                    if (!neighbour->recycler && !willCellTurnToGrass(neighbour))
                    {
                        auto numUnitsToMove = getMaxToMove(myCell);
                        if (numUnitsToMove > 0)
                        {
                            command.AddMove(numUnitsToMove, myCell->p, neighbour->p, __LINE__);

                            // Units already moved and can not do anything else. Lets just remove them
                            GInf.myCUnits.erase(GInf.myCUnits.begin() + i);
                        }
                    }
                }
            }
        }
        
        DBG_MSG_STR("After willCellTurnToGrass checks:");
        DBG_V(GInf.myCUnits.size());

        if (GInf.enemyCUnits.size() > 0)
        {
            // TODO: dummy logic - need to remove
            if (GInf.currentTurn % 2 == 0 && GInf.currentTurn < 10)
            {
                auto pMyCUnitsG1 = &GInf.getCell(getMyUnitNearestToEnemyPos());

                // unite into one unit: move all units to pMyCUnitsG1
                for (int i = 0; i < std::max((int)GInf.myCUnits.size() - 1, 1); i++)
                {
                    auto cell_to_move = GInf.myCUnits[i];

                    // Do not move researcher and invader
                    if (researcherPos.isValid && cell_to_move->p == researcherPos ||
                        invaderPos.isValid    && cell_to_move->p == invaderPos)
                        continue;

                    auto numUnitsToMove = getMaxToMove(cell_to_move);
                    if (cell_to_move->p != pMyCUnitsG1->p)
                    {
                        command.AddMove(numUnitsToMove, cell_to_move->p, pMyCUnitsG1->p, __LINE__);
                    }
                }
            }
            else
            {
                // ============================================= Global movement 2 =============================================
                for (int i = 0; i < std::max((int)GInf.myCUnits.size(), 1); i++)
                {
                    auto cell_to_move = GInf.myCUnits[i];

                    // Do not move researcher and invader
                    if (researcherPos.isValid && cell_to_move->p == researcherPos ||
                        invaderPos.isValid    && cell_to_move->p == invaderPos)
                        continue;

                    auto numUnitsToMove = getMaxToMove(cell_to_move);
                    if (numUnitsToMove > 0)
                    {
                        Pos nearestEnemyPos = getNearestUnitToPos(GInf.enemyCUnits, cell_to_move->p);
                        command.AddMove(numUnitsToMove, cell_to_move->p, nearestEnemyPos, __LINE__);
                    }
                }
            }
        }

        // Research part
        DBG_MSG_STR("-------- Research part start --------");
        if (!researcherPos.isValid && GInf.my_matter >= 10)
        {
            Pos enemyCenterPos = getUnitsEstCenterPos(GInf.enemyCUnits);
            int maxDistance = 0;
            for (auto myCell : GInf.myCells)
            {
                if (myCell->hasUnitsRec())
                    continue;

                int dist = myCell->p.distanceTo(enemyCenterPos);
                if (dist > maxDistance)
                {
                    auto[pDest, pFirstStep] = getNearest_EnemyOrNoone_CellPosToMove(myCell);
                    if (pDest.isValid)
                    {
                        maxDistance = dist;
                        researcherPos = myCell->p;
                        DBG_MSG_V("Nearest_NooneOrEnemy:", pDest);
                    }
                }
            }
            
            DBG_MSG_V("Spawn researcher at:", researcherPos);
            command.AddSpawn(1, researcherPos, __LINE__);
        }
        else if (researcherPos.isValid)
        {
            auto myResearcher = &GInf.getCell(researcherPos);
            auto[pNotMyDest, pFirstStep] = getNearest_EnemyOrNoone_CellPosToMove(myResearcher);
            if (pNotMyDest.isValid)
            {
                command.AddMove(myResearcher->units, myResearcher->p, pFirstStep, __LINE__);
                researcherPos = pFirstStep;
                DBG_MSG_V("Move researcher to:", pFirstStep);
            }
            else
            {
                researcherPos.invalidate();
            }
        }

        // Invader part
        DBG_MSG_STR("-------- Invader part start --------");
        if (!invaderPos.isValid && GInf.my_matter >= 10 && GInf.myCUnitsNum + GInf.my_matter/10 - 2 > GInf.enemyCUnitsNum + GInf.opp_matter/10)
        {
            int maxDistance = 0; // global max distance to nearest enemy
            for (auto myCell : GInf.myCells)
            {
                if (myCell->hasUnitsRec())
                    continue;

                int cellMinDistToEnemy = INT32_MAX;
                std::for_each(GInf.enemyCUnits.begin(), GInf.enemyCUnits.end(), [&cellMinDistToEnemy, myCell](MapCell* c) { cellMinDistToEnemy = std::min(cellMinDistToEnemy, myCell->p.distanceTo(c->p)); });
                if (cellMinDistToEnemy > maxDistance)
                {
                    auto[pDest, pFirstStep] = getNearest_Enemy_CellPosToMove(myCell);
                    if (pDest.isValid)
                    {
                        maxDistance = cellMinDistToEnemy;
                        invaderPos = myCell->p;
                        DBG_MSG_V("Nearest_Enemy:", pDest);
                    }
                }
            }
            
            DBG_MSG_V("Spawn invader at:", invaderPos);
            command.AddSpawn(1, invaderPos, __LINE__);
        }
        else if (invaderPos.isValid)
        {
            DBG_MSG_V("Invader pos:", invaderPos);
            auto myInvader = &GInf.getCell(invaderPos);
            auto[pDest, pFirstStep] = getNearest_Enemy_CellPosToMove(myInvader);
            if (pDest.isValid)
            {
                command.AddMove(myInvader->units, myInvader->p, pFirstStep, __LINE__);
                researcherPos = pFirstStep;
                DBG_MSG_V("Move invader to:", pFirstStep);
            }
            else
            {
                invaderPos.invalidate();
                DBG_MSG_V("Invalidate invader since no enemy cells around:", invaderPos);
            }
        }

        DBG_MSG_STR("--------------------------------------");
      
        if (command.empty())
            command.AddWait();

        command.Submit();
    }
}
